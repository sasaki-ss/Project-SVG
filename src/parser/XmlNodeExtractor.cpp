#include "XmlNodeExtractor.h"

#include <cctype>
#include <string>
#include <vector>

namespace svg{
namespace parser{
namespace {

class XmlCursor {
public:
    explicit XmlCursor(std::string_view content) : content_(content), position_(0) {}

    bool is_eof() const {
        return position_ >= content_.size();
    }

    char peek() const {
        if (is_eof()) {
            return '\0';
        }
        return content_[position_];
    }

    char peek_next() const {
        if (position_ + 1 >= content_.size()) {
            return '\0';
        }
        return content_[position_ + 1];
    }

    void advance() {
        if (!is_eof()) {
            ++position_;
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
        if (token.size() > content_.size() - position_) {
            return false;
        }
        return content_.substr(position_, token.size()) == token;
    }

    bool consume_token(std::string_view token) {
        if (!starts_with(token)) {
            return false;
        }
        position_ += token.size();
        return true;
    }

    void skip_whitespace() {
        while (!is_eof() && std::isspace(static_cast<unsigned char>(peek())) != 0) {
            advance();
        }
    }

    size_t position() const {
        return position_;
    }

private:
    std::string_view content_;
    size_t position_;
};

bool is_name_start_char(char value) {
    return std::isalpha(static_cast<unsigned char>(value)) != 0 || value == '_' || value == ':';
}

bool is_name_char(char value) {
    return std::isalnum(static_cast<unsigned char>(value)) != 0 || value == '_' || value == ':' || value == '-' || value == '.';
}

bool parse_xml_name(XmlCursor& cursor, std::string& output_name) {
    if (!is_name_start_char(cursor.peek())) {
        return false;
    }

    output_name.clear();
    output_name.push_back(cursor.peek());
    cursor.advance();

    while (is_name_char(cursor.peek())) {
        output_name.push_back(cursor.peek());
        cursor.advance();
    }

    return true;
}

bool skip_until(XmlCursor& cursor, std::string_view terminal) {
    while (!cursor.is_eof()) {
        if (cursor.starts_with(terminal)) {
            cursor.consume_token(terminal);
            return true;
        }
        cursor.advance();
    }
    return false;
}

bool skip_xml_misc(XmlCursor& cursor) {
    bool skipped = false;

    while (!cursor.is_eof()) {
        cursor.skip_whitespace();

        if (cursor.starts_with("<?")) {
            cursor.consume_token("<?");
            if (!skip_until(cursor, "?>")) {
                return false;
            }
            skipped = true;
            continue;
        }

        if (cursor.starts_with("<!--")) {
            cursor.consume_token("<!--");
            if (!skip_until(cursor, "-->")) {
                return false;
            }
            skipped = true;
            continue;
        }

        if (cursor.starts_with("<!DOCTYPE")) {
            cursor.consume_token("<!DOCTYPE");
            int bracket_depth = 0;
            while (!cursor.is_eof()) {
                const char current = cursor.peek();
                if (current == '[') {
                    ++bracket_depth;
                } else if (current == ']') {
                    if (bracket_depth > 0) {
                        --bracket_depth;
                    }
                } else if (current == '>' && bracket_depth == 0) {
                    cursor.advance();
                    break;
                }
                cursor.advance();
            }
            if (cursor.is_eof() && cursor.peek() != '>') {
                return false;
            }
            skipped = true;
            continue;
        }

        break;
    }

    return skipped || !cursor.is_eof();
}

bool parse_quoted_value(XmlCursor& cursor, std::string& output_value) {
    const char quote = cursor.peek();
    if (quote != '\'' && quote != '"') {
        return false;
    }
    cursor.advance();

    output_value.clear();
    while (!cursor.is_eof() && cursor.peek() != quote) {
        output_value.push_back(cursor.peek());
        cursor.advance();
    }

    if (!cursor.consume(quote)) {
        return false;
    }

    return true;
}

bool parse_attribute(XmlCursor& cursor, ExtractedAttribute& extracted_attribute) {
    std::string attribute_name;
    if (!parse_xml_name(cursor, attribute_name)) {
        return false;
    }

    cursor.skip_whitespace();
    if (!cursor.consume('=')) {
        return false;
    }
    cursor.skip_whitespace();

    std::string attribute_value;
    if (!parse_quoted_value(cursor, attribute_value)) {
        return false;
    }

    extracted_attribute.name = std::move(attribute_name);
    extracted_attribute.value = std::move(attribute_value);
    return true;
}

bool parse_node_recursive(XmlCursor& cursor, ExtractedNode& extracted_node) {
    if (!cursor.consume('<')) {
        return false;
    }

    if (cursor.peek() == '/' || cursor.peek() == '!' || cursor.peek() == '?') {
        return false;
    }

    std::string element_name;
    if (!parse_xml_name(cursor, element_name)) {
        return false;
    }

    extracted_node = ExtractedNode{};
    extracted_node.element_name = std::move(element_name);

    while (!cursor.is_eof()) {
        cursor.skip_whitespace();

        if (cursor.consume('/')) {
            return cursor.consume('>');
        }

        if (cursor.consume('>')) {
            break;
        }

        ExtractedAttribute extracted_attribute;
        if (!parse_attribute(cursor, extracted_attribute)) {
            return false;
        }
        extracted_node.attributes.push_back(std::move(extracted_attribute));
    }

    while (!cursor.is_eof()) {
        cursor.skip_whitespace();

        if (cursor.starts_with("<!--")) {
            cursor.consume_token("<!--");
            if (!skip_until(cursor, "-->")) {
                return false;
            }
            continue;
        }

        if (cursor.starts_with("</")) {
            cursor.consume_token("</");
            std::string closing_name;
            if (!parse_xml_name(cursor, closing_name)) {
                return false;
            }
            cursor.skip_whitespace();
            if (!cursor.consume('>')) {
                return false;
            }
            return closing_name == extracted_node.element_name;
        }

        if (cursor.peek() == '<') {
            ExtractedNode child_node;
            if (!parse_node_recursive(cursor, child_node)) {
                return false;
            }
            extracted_node.children.push_back(std::move(child_node));
            continue;
        }

        while (!cursor.is_eof() && cursor.peek() != '<') {
            cursor.advance();
        }
    }

    return false;
}

}  // namespace

std::optional<ExtractedNode> XmlNodeExtractor::extract_from_xml(std::string_view xml_content) {
    XmlCursor cursor(xml_content);
    if (!skip_xml_misc(cursor)) {
        return std::nullopt;
    }

    ExtractedNode root_node;
    if (!parse_node_recursive(cursor, root_node)) {
        return std::nullopt;
    }

    if (root_node.element_name != "svg") {
        return std::nullopt;
    }

    skip_xml_misc(cursor);
    cursor.skip_whitespace();
    if (!cursor.is_eof()) {
        return std::nullopt;
    }

    return root_node;
}

}
}
