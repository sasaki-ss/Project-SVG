#include <iostream>
#include <string_view>

#include "XmlNodeExtractor.h"

int main() {
    constexpr std::string_view sample_xml = R"(<svg width="24"><g /></svg>)";

    svg::parser::XmlNodeExtractor extractor(sample_xml);
    const auto extracted = extractor.extract_from_xml();

    if (!extracted.has_value()) {
        std::cerr << "XmlNodeExtractor failed to parse a minimal svg document." << std::endl;
        return 1;
    }

    if (extracted->element_name != "svg") {
        std::cerr << "Unexpected root node name: " << extracted->element_name << std::endl;
        return 1;
    }

    return 0;
}
