#include "XmlNodeExtractor.h"

#include <cctype>
#include <utility>

namespace svg{
namespace parser{

XmlNodeExtractor::XmlNodeExtractor(std::string_view xml_content)
    : content_(xml_content), position_(0) {}

std::optional<ExtractedNode> XmlNodeExtractor::extract_from_xml(std::string_view xml_content) {
    XmlNodeExtractor extractor(xml_content);
    return extractor.extract();
}

std::optional<ExtractedNode> XmlNodeExtractor::extract() {
    if (!skip_xml_misc()) {
        return std::nullopt;
    }

    ExtractedNode root_node;
    if (!parse_node_recursive(root_node)) {
        return std::nullopt;
    }

    if (root_node.element_name != "svg") {
        return std::nullopt;
    }

    skip_xml_misc();
    skip_whitespace();
    if (!is_eof()) {
        return std::nullopt;
    }

    return root_node;
}

bool XmlNodeExtractor::skip_xml_misc() {
    bool skipped = false;

    while (!is_eof()) {
        skip_whitespace();

        if (starts_with("<?")) {
            consume_token("<?");
            if (!skip_until("?>")) {
                return false;
            }
            skipped = true;
            continue;
        }

        if (starts_with("<!--")) {
            consume_token("<!--");
            if (!skip_until("-->")) {
                return false;
            }
            skipped = true;
            continue;
        }

        if (starts_with("<!DOCTYPE")) {
            consume_token("<!DOCTYPE");
            int bracket_depth = 0;
            while (!is_eof()) {
                const char current = peek();
                if (current == '[') {
                    ++bracket_depth;
                } else if (current == ']') {
                    if (bracket_depth > 0) {
                        --bracket_depth;
                    }
                } else if (current == '>' && bracket_depth == 0) {
                    advance();
                    break;
                }
                advance();
            }
            if (is_eof() && peek() != '>') {
                return false;
            }
            skipped = true;
            continue;
        }

        break;
    }

    return skipped || !is_eof();
}

bool XmlNodeExtractor::parse_node_recursive(ExtractedNode& extracted_node) {
    if (!consume('<')) {
        return false;
    }

    if (peek() == '/' || peek() == '!' || peek() == '?') {
        return false;
    }

    std::string element_name;
    if (!parse_xml_name(element_name)) {
        return false;
    }

    extracted_node = ExtractedNode{};
    extracted_node.element_name = std::move(element_name);

    while (!is_eof()) {
        skip_whitespace();

        if (consume('/')) {
            return consume('>');
        }

        if (consume('>')) {
            break;
        }

        ExtractedAttribute extracted_attribute;
        if (!parse_attribute(extracted_attribute)) {
            return false;
        }
        extracted_node.attributes.push_back(std::move(extracted_attribute));
    }

    while (!is_eof()) {
        skip_whitespace();

        if (starts_with("<!--")) {
            consume_token("<!--");
            if (!skip_until("-->")) {
                return false;
            }
            continue;
        }

        if (starts_with("</")) {
            consume_token("</");
            std::string closing_name;
            if (!parse_xml_name(closing_name)) {
                return false;
            }
            skip_whitespace();
            if (!consume('>')) {
                return false;
            }
            return closing_name == extracted_node.element_name;
        }

        if (peek() == '<') {
            ExtractedNode child_node;
            if (!parse_node_recursive(child_node)) {
                return false;
            }
            extracted_node.children.push_back(std::move(child_node));
            continue;
        }

        while (!is_eof() && peek() != '<') {
            advance();
        }
    }

    return false;
}

bool XmlNodeExtractor::parse_attribute(ExtractedAttribute& extracted_attribute) {
    std::string attribute_name;
    if (!parse_xml_name(attribute_name)) {
        return false;
    }

    skip_whitespace();
    if (!consume('=')) {
        return false;
    }
    skip_whitespace();

    std::string attribute_value;
    if (!parse_quoted_value(attribute_value)) {
        return false;
    }

    extracted_attribute.name = std::move(attribute_name);
    extracted_attribute.value = std::move(attribute_value);
    return true;
}

bool XmlNodeExtractor::parse_xml_name(std::string& output_name) {
    if (!is_name_start_char(peek())) {
        return false;
    }

    output_name.clear();
    output_name.push_back(peek());
    advance();

    while (is_name_char(peek())) {
        output_name.push_back(peek());
        advance();
    }

    return true;
}

bool XmlNodeExtractor::parse_quoted_value(std::string& output_value) {
    const char quote = peek();
    if (quote != '\'' && quote != '"') {
        return false;
    }
    advance();

    output_value.clear();
    while (!is_eof() && peek() != quote) {
        output_value.push_back(peek());
        advance();
    }

    if (!consume(quote)) {
        return false;
    }

    return true;
}

bool XmlNodeExtractor::skip_until(std::string_view terminal) {
    while (!is_eof()) {
        if (starts_with(terminal)) {
            consume_token(terminal);
            return true;
        }
        advance();
    }

    return false;
}

bool XmlNodeExtractor::is_eof() const {
    return position_ >= content_.size();
}

char XmlNodeExtractor::peek() const {
    if (is_eof()) {
        return '\0';
    }
    return content_[position_];
}

bool XmlNodeExtractor::consume(char expected) {
    if (peek() != expected) {
        return false;
    }
    advance();
    return true;
}

bool XmlNodeExtractor::starts_with(std::string_view token) const {
    if (token.size() > content_.size() - position_) {
        return false;
    }
    return content_.substr(position_, token.size()) == token;
}

bool XmlNodeExtractor::consume_token(std::string_view token) {
    if (!starts_with(token)) {
        return false;
    }
    position_ += token.size();
    return true;
}

void XmlNodeExtractor::advance() {
    if (!is_eof()) {
        ++position_;
    }
}

void XmlNodeExtractor::skip_whitespace() {
    while (!is_eof() && std::isspace(static_cast<unsigned char>(peek())) != 0) {
        advance();
    }
}

bool XmlNodeExtractor::is_name_start_char(char value) {
    return std::isalpha(static_cast<unsigned char>(value)) != 0 || value == '_' || value == ':';
}

bool XmlNodeExtractor::is_name_char(char value) {
    return std::isalnum(static_cast<unsigned char>(value)) != 0 || value == '_' || value == ':' || value == '-' || value == '.';
}

}
}
