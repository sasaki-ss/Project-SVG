#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "XmlNodeExtractor.h"

namespace {

bool expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        return false;
    }
    return true;
}

bool expect_attribute(const svg::parser::ExtractedNode& node,
                      std::size_t index,
                      std::string_view expected_name,
                      std::string_view expected_value,
                      std::string_view case_name) {
    if (!expect(index < node.attributes.size(), std::string(case_name) + ": attribute index is out of range.")) {
        return false;
    }

    const auto& attribute = node.attributes[index];
    if (!expect(attribute.name == expected_name,
                std::string(case_name) + ": unexpected attribute name at index " + std::to_string(index))) {
        return false;
    }

    return expect(attribute.value == expected_value,
                  std::string(case_name) + ": unexpected attribute value at index " + std::to_string(index));
}

bool test_extract_minimal_svg() {
    constexpr std::string_view xml_content = R"(<svg></svg>)";

    svg::parser::XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    if (!expect(extracted.has_value(), "minimal: expected parsed node.")) {
        return false;
    }

    if (!expect(extracted->element_name == "svg", "minimal: root element must be svg.")) {
        return false;
    }

    if (!expect(extracted->attributes.empty(), "minimal: root attributes must be empty.")) {
        return false;
    }

    return expect(extracted->children.empty(), "minimal: root children must be empty.");
}

bool test_extract_svg_attributes() {
    constexpr std::string_view xml_content = R"(<svg width="24" height='16' viewBox="0 0 24 16"></svg>)";

    svg::parser::XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    if (!expect(extracted.has_value(), "attributes: expected parsed node.")) {
        return false;
    }

    if (!expect(extracted->attributes.size() == 3, "attributes: expected 3 attributes.")) {
        return false;
    }

    if (!expect_attribute(*extracted, 0, "width", "24", "attributes")) {
        return false;
    }

    if (!expect_attribute(*extracted, 1, "height", "16", "attributes")) {
        return false;
    }

    return expect_attribute(*extracted, 2, "viewBox", "0 0 24 16", "attributes");
}

bool test_extract_children_recursively() {
    constexpr std::string_view xml_content =
        R"(<svg><g id="layer"><path d="M0 0" /></g><rect width="10" height="20"></rect></svg>)";

    svg::parser::XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    if (!expect(extracted.has_value(), "children: expected parsed node.")) {
        return false;
    }

    if (!expect(extracted->children.size() == 2, "children: expected 2 direct children under svg.")) {
        return false;
    }

    const auto& g_node = extracted->children[0];
    if (!expect(g_node.element_name == "g", "children: first child must be g.")) {
        return false;
    }

    if (!expect(g_node.attributes.size() == 1, "children: g must have one attribute.")) {
        return false;
    }

    if (!expect_attribute(g_node, 0, "id", "layer", "children")) {
        return false;
    }

    if (!expect(g_node.children.size() == 1, "children: g must have one nested child.")) {
        return false;
    }

    const auto& path_node = g_node.children[0];
    if (!expect(path_node.element_name == "path", "children: nested child must be path.")) {
        return false;
    }

    if (!expect(path_node.children.empty(), "children: path must have no children.")) {
        return false;
    }

    if (!expect(path_node.attributes.size() == 1, "children: path must have one attribute.")) {
        return false;
    }

    if (!expect_attribute(path_node, 0, "d", "M0 0", "children")) {
        return false;
    }

    const auto& rect_node = extracted->children[1];
    if (!expect(rect_node.element_name == "rect", "children: second child must be rect.")) {
        return false;
    }

    if (!expect(rect_node.attributes.size() == 2, "children: rect must have two attributes.")) {
        return false;
    }

    if (!expect_attribute(rect_node, 0, "width", "10", "children")) {
        return false;
    }

    return expect_attribute(rect_node, 1, "height", "20", "children");
}

bool test_extract_self_closing_nodes() {
    constexpr std::string_view xml_content = R"(<svg><g/><path d="M1 1"/></svg>)";

    svg::parser::XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    if (!expect(extracted.has_value(), "self-closing: expected parsed node.")) {
        return false;
    }

    if (!expect(extracted->children.size() == 2, "self-closing: expected 2 children.")) {
        return false;
    }

    const auto& g_node = extracted->children[0];
    if (!expect(g_node.element_name == "g", "self-closing: first child must be g.")) {
        return false;
    }

    if (!expect(g_node.attributes.empty(), "self-closing: g attributes must be empty.")) {
        return false;
    }

    if (!expect(g_node.children.empty(), "self-closing: g children must be empty.")) {
        return false;
    }

    const auto& path_node = extracted->children[1];
    if (!expect(path_node.element_name == "path", "self-closing: second child must be path.")) {
        return false;
    }

    if (!expect(path_node.children.empty(), "self-closing: path children must be empty.")) {
        return false;
    }

    if (!expect(path_node.attributes.size() == 1, "self-closing: path must have one attribute.")) {
        return false;
    }

    return expect_attribute(path_node, 0, "d", "M1 1", "self-closing");
}

bool test_fail_when_root_is_not_svg() {
    constexpr std::string_view xml_content = R"(<g></g>)";

    svg::parser::XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    return expect(!extracted.has_value(), "non-svg root: expected nullopt.");
}

}  // namespace

int main() {
    const std::vector<std::pair<const char*, bool (*)()>> test_cases = {
        {"extract minimal svg", test_extract_minimal_svg},
        {"extract svg attributes", test_extract_svg_attributes},
        {"extract children recursively", test_extract_children_recursively},
        {"extract self-closing nodes", test_extract_self_closing_nodes},
        {"fail when root is not svg", test_fail_when_root_is_not_svg},
    };

    for (const auto& test_case : test_cases) {
        if (!test_case.second()) {
            std::cerr << "Failed test: " << test_case.first << std::endl;
            return 1;
        }
    }

    return 0;
}
