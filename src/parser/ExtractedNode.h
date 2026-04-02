#ifndef PROJECT_SVG_PARSER_EXTRACTED_NODE_H_
#define PROJECT_SVG_PARSER_EXTRACTED_NODE_H_

#include <string>
#include <vector>

namespace svg{
namespace parser{

struct ExtractedAttribute {
    std::string name;
    std::string value;
};

struct ExtractedNode {
    std::string element_name;
    std::vector<ExtractedAttribute> attributes;
    std::vector<ExtractedNode> children;
};

}
}

#endif  // PROJECT_SVG_PARSER_EXTRACTED_NODE_H_
