#include "XmlNodeExtractor.h"

#include <cctype>
#include <utility>

namespace svg{
namespace parser{

class XmlNodeExtractor::XmlReader {
public:
    explicit XmlReader(std::string_view xml_content)
        : xml_content(xml_content), current_position(0) {}

    bool is_eof() const {
        return current_position >= xml_content.size();
    }

    char peek() const {
        if (is_eof()) {
            return '\0';
        }
        return xml_content[current_position];
    }

    void advance() {
        if (!is_eof()) {
            ++current_position;
        }
    }

    bool consume(char expected) {
        if (peek() != expected) {
            return false;
        }
        advance();
        return true;
    }

    bool starts_with(std::string_view token) const {
        if (token.size() > xml_content.size() - current_position) {
            return false;
        }
        return xml_content.substr(current_position, token.size()) == token;
    }

    bool consume_token(std::string_view token) {
        if (!starts_with(token)) {
            return false;
        }
        current_position += token.size();
        return true;
    }

    void skip_whitespace() {
        while (!is_eof() && std::isspace(static_cast<unsigned char>(peek())) != 0) {
            advance();
        }
    }

    bool skip_until(std::string_view terminal) {
        while (!is_eof()) {
            if (starts_with(terminal)) {
                consume_token(terminal);
                return true;
            }
            advance();
        }
        return false;
    }

private:
    std::string_view xml_content;
    std::size_t current_position;
};

namespace {

bool is_xml_name_start_char(char value) {
    return std::isalpha(static_cast<unsigned char>(value)) != 0 || value == '_' || value == ':';
}

bool is_xml_name_char(char value) {
    return std::isalnum(static_cast<unsigned char>(value)) != 0 || value == '_' || value == ':' || value == '-' || value == '.';
}

}  // namespace

std::optional<ExtractedNode> XmlNodeExtractor::extract_from_xml(std::string_view xml_content) {
    XmlReader xml_reader(xml_content);

    if (!normalize_document_start(xml_reader)) {
        return std::nullopt;
    }

    auto root_node = extract_node(xml_reader);
    if (!root_node) {
        return std::nullopt;
    }

    if (root_node->element_name != "svg") {
        return std::nullopt;
    }

    if (!validate_document_end(xml_reader)) {
        return std::nullopt;
    }

    return root_node;
}

bool XmlNodeExtractor::normalize_document_start(XmlReader& xml_reader) {
    while (!xml_reader.is_eof()) {
        xml_reader.skip_whitespace();

        if (xml_reader.starts_with("<?")) {
            xml_reader.consume_token("<?");
            if (!xml_reader.skip_until("?>")) {
                return false;
            }
            continue;
        }

        if (xml_reader.starts_with("<!--")) {
            xml_reader.consume_token("<!--");
            if (!xml_reader.skip_until("-->")) {
                return false;
            }
            continue;
        }

        if (xml_reader.starts_with("<!DOCTYPE")) {
            xml_reader.consume_token("<!DOCTYPE");
            int bracket_depth = 0;
            while (!xml_reader.is_eof()) {
                const char current = xml_reader.peek();
                if (current == '[') {
                    ++bracket_depth;
                } else if (current == ']') {
                    if (bracket_depth > 0) {
                        --bracket_depth;
                    }
                } else if (current == '>' && bracket_depth == 0) {
                    xml_reader.advance();
                    break;
                }
                xml_reader.advance();
            }

            if (xml_reader.is_eof() && xml_reader.peek() != '>') {
                return false;
            }
            continue;
        }

        break;
    }

    return !xml_reader.is_eof();
}

std::optional<ExtractedNode> XmlNodeExtractor::extract_node(XmlReader& xml_reader) {
    if (!xml_reader.consume('<')) {
        return std::nullopt;
    }

    if (xml_reader.peek() == '/' || xml_reader.peek() == '!' || xml_reader.peek() == '?') {
        return std::nullopt;
    }

    std::string element_name;
    if (!extract_xml_name(xml_reader, element_name)) {
        return std::nullopt;
    }

    ExtractedNode extracted_node;
    extracted_node.element_name = std::move(element_name);

    if (!extract_attributes(xml_reader, extracted_node.attributes)) {
        return std::nullopt;
    }

    if (xml_reader.consume_token("/>")) {
        return extracted_node;
    }

    if (!xml_reader.consume('>')) {
        return std::nullopt;
    }

    if (!extract_child_nodes(xml_reader, extracted_node.element_name, extracted_node.children)) {
        return std::nullopt;
    }

    return extracted_node;
}

bool XmlNodeExtractor::extract_xml_name(XmlReader& xml_reader, std::string& extracted_name) {
    if (!is_xml_name_start_char(xml_reader.peek())) {
        return false;
    }

    extracted_name.clear();
    extracted_name.push_back(xml_reader.peek());
    xml_reader.advance();

    while (is_xml_name_char(xml_reader.peek())) {
        extracted_name.push_back(xml_reader.peek());
        xml_reader.advance();
    }

    return true;
}

bool XmlNodeExtractor::extract_attributes(XmlReader& xml_reader, std::vector<ExtractedAttribute>& extracted_attributes) {
    while (!xml_reader.is_eof()) {
        xml_reader.skip_whitespace();

        if (xml_reader.peek() == '>' || xml_reader.starts_with("/>")) {
            return true;
        }

        std::string attribute_name;
        if (!extract_xml_name(xml_reader, attribute_name)) {
            return false;
        }

        xml_reader.skip_whitespace();
        if (!xml_reader.consume('=')) {
            return false;
        }
        xml_reader.skip_whitespace();

        std::string attribute_value;
        if (!extract_quoted_value(xml_reader, attribute_value)) {
            return false;
        }

        ExtractedAttribute extracted_attribute;
        extracted_attribute.name = std::move(attribute_name);
        extracted_attribute.value = std::move(attribute_value);
        extracted_attributes.push_back(std::move(extracted_attribute));
    }

    return false;
}

bool XmlNodeExtractor::extract_quoted_value(XmlReader& xml_reader, std::string& extracted_value) {
    const char quote = xml_reader.peek();
    if (quote != '\'' && quote != '"') {
        return false;
    }

    xml_reader.advance();
    extracted_value.clear();

    while (!xml_reader.is_eof() && xml_reader.peek() != quote) {
        extracted_value.push_back(xml_reader.peek());
        xml_reader.advance();
    }

    return xml_reader.consume(quote);
}

bool XmlNodeExtractor::extract_child_nodes(XmlReader& xml_reader, std::string_view parent_name, std::vector<ExtractedNode>& extracted_children) {
    while (!xml_reader.is_eof()) {
        if (xml_reader.starts_with("<!--")) {
            xml_reader.consume_token("<!--");
            if (!xml_reader.skip_until("-->")) {
                return false;
            }
            continue;
        }

        if (xml_reader.starts_with("</")) {
            xml_reader.consume_token("</");

            std::string closing_name;
            if (!extract_xml_name(xml_reader, closing_name)) {
                return false;
            }

            xml_reader.skip_whitespace();
            if (!xml_reader.consume('>')) {
                return false;
            }

            return closing_name == parent_name;
        }

        if (xml_reader.peek() == '<') {
            auto child_node = extract_node(xml_reader);
            if (!child_node) {
                return false;
            }
            extracted_children.push_back(std::move(*child_node));
            continue;
        }

        while (!xml_reader.is_eof() && xml_reader.peek() != '<') {
            xml_reader.advance();
        }
    }

    return false;
}

bool XmlNodeExtractor::validate_document_end(XmlReader& xml_reader) {
    while (!xml_reader.is_eof()) {
        xml_reader.skip_whitespace();

        if (xml_reader.starts_with("<!--")) {
            xml_reader.consume_token("<!--");
            if (!xml_reader.skip_until("-->")) {
                return false;
            }
            continue;
        }

        if (xml_reader.starts_with("<?")) {
            xml_reader.consume_token("<?");
            if (!xml_reader.skip_until("?>")) {
                return false;
            }
            continue;
        }

        if (xml_reader.starts_with("<!DOCTYPE")) {
            return false;
        }

        break;
    }

    return xml_reader.is_eof();
}

}
}
