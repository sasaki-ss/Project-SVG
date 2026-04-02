#ifndef PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
#define PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_

#include <optional>
#include <string_view>

#include "ExtractedNode.h"

namespace svg{
namespace parser{

class XmlNodeExtractor {
public:
    static std::optional<ExtractedNode> extract_from_xml(std::string_view xml_content);
};

}
}

#endif  // PROJECT_SVG_PARSER_XML_NODE_EXTRACTOR_H_
