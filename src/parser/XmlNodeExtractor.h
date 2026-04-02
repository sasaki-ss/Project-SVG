#ifndef PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
#define PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ExtractedNode.h"

namespace svg{
namespace parser{

class XmlNodeExtractor {
public:
    explicit XmlNodeExtractor(std::string_view xml_content);

    std::optional<ExtractedNode> extract_from_xml();

private:
    class XmlReader {
    public:
        explicit XmlReader(std::string_view xml_content);

        static bool is_xml_name_start_char(char value);
        static bool is_xml_name_char(char value);

        bool is_eof() const;
        char peek() const;
        void advance();
        bool consume(char expected);
        bool starts_with(std::string_view token) const;
        bool consume_token(std::string_view token);
        bool consume_enclosed(std::string_view begin_token, std::string_view end_token);
        bool consume_doctype();
        void skip_whitespace();
        bool skip_until(std::string_view terminal);

    private:
        std::string_view xml_content;
        std::size_t current_position = 0;
    };

    bool normalize_document_start();
    std::optional<ExtractedNode> extract_node();
    bool extract_xml_name(std::string& extracted_name);
    bool extract_attributes(std::vector<ExtractedAttribute>& extracted_attributes);
    bool extract_quoted_value(std::string& extracted_value);
    bool extract_child_nodes(std::string_view parent_name, std::vector<ExtractedNode>& extracted_children);
    bool validate_document_end();

    std::string xml_content;
    XmlReader xml_reader;
};

}
}

#endif  // PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
