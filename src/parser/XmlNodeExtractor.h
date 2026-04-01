#ifndef PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
#define PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_

#include <optional>

#include <tinyxml2.h>

#include "ExtractedNode.h"

namespace svg{
namespace parser{

class XmlNodeExtractor {
public:
    static std::optional<ExtractedNode> extract_from_document(const tinyxml2::XMLDocument& xml_document);

private:
    static ExtractedNode extract_node_recursive(const tinyxml2::XMLElement& xml_element);
};

}
}

#endif  // PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
