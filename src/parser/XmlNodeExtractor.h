#ifndef PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
#define PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_

#include <optional>
#include <string>
#include <string_view>

#include "ExtractedNode.h"

namespace svg{
namespace parser{

class XmlNodeExtractor {
public:
    static std::optional<ExtractedNode> extract_from_xml(std::string_view xml_content);

private:
    explicit XmlNodeExtractor(std::string_view xml_content);

    std::optional<ExtractedNode> extract();
    bool skip_xml_misc();
    bool parse_node_recursive(ExtractedNode& extracted_node);
    bool parse_attribute(ExtractedAttribute& extracted_attribute);
    bool parse_xml_name(std::string& output_name);
    bool parse_quoted_value(std::string& output_value);
    bool skip_until(std::string_view terminal);

    bool is_eof() const;
    char peek() const;
    bool consume(char expected);
    bool starts_with(std::string_view token) const;
    bool consume_token(std::string_view token);
    void advance();
    void skip_whitespace();

    static bool is_name_start_char(char value);
    static bool is_name_char(char value);

    std::string_view content_;
    size_t position_;
};

}
}

#endif  // PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
