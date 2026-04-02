#ifndef PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
#define PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "ExtractedNode.h"

namespace svg{
namespace parser{

class XmlNodeExtractor {
public:
    static std::optional<ExtractedNode> extract_from_xml(std::string_view xml_content);

private:
    class XmlReader;

    static bool normalize_document_start(XmlReader& xml_reader);
    static std::optional<ExtractedNode> extract_node(XmlReader& xml_reader);
    static bool extract_xml_name(XmlReader& xml_reader, std::string& extracted_name);
    static bool extract_attributes(XmlReader& xml_reader, std::vector<ExtractedAttribute>& extracted_attributes);
    static bool extract_quoted_value(XmlReader& xml_reader, std::string& extracted_value);
    static bool extract_child_nodes(XmlReader& xml_reader, std::string_view parent_name, std::vector<ExtractedNode>& extracted_children);
    static bool validate_document_end(XmlReader& xml_reader);
};

}
}

#endif  // PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
