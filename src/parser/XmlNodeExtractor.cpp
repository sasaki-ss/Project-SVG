#include "XmlNodeExtractor.h"

#include <cstring>

namespace svg{
namespace parser{

std::optional<ExtractedNode> XmlNodeExtractor::extract_from_document(const tinyxml2::XMLDocument& xml_document) {
    const tinyxml2::XMLElement* root_element = xml_document.RootElement();
    if (root_element == nullptr) {
        return std::nullopt;
    }

    if (std::strcmp(root_element->Name(), "svg") != 0) {
        return std::nullopt;
    }

    return extract_node_recursive(*root_element);
}

ExtractedNode XmlNodeExtractor::extract_node_recursive(const tinyxml2::XMLElement& xml_element) {
    ExtractedNode extracted_node;
    extracted_node.element_name = xml_element.Name();

    const tinyxml2::XMLAttribute* attribute = xml_element.FirstAttribute();
    while (attribute != nullptr) {
        ExtractedAttribute extracted_attribute;
        extracted_attribute.name = attribute->Name();
        extracted_attribute.value = attribute->Value();
        extracted_node.attributes.push_back(extracted_attribute);
        attribute = attribute->Next();
    }

    const tinyxml2::XMLElement* child_element = xml_element.FirstChildElement();
    while (child_element != nullptr) {
        extracted_node.children.push_back(extract_node_recursive(*child_element));
        child_element = child_element->NextSiblingElement();
    }

    return extracted_node;
}

}
}
