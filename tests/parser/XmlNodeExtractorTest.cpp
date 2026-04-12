#include <gtest/gtest.h>

#include <string_view>

#include "parser/XmlNodeExtractor.h"

namespace {

using svg::parser::ExtractedNode;
using svg::parser::XmlNodeExtractor;

void expect_attribute(const ExtractedNode& node,
                      std::size_t index,
                      std::string_view expected_name,
                      std::string_view expected_value) {
    ASSERT_LT(index, node.attributes.size());
    EXPECT_EQ(node.attributes[index].name, expected_name);
    EXPECT_EQ(node.attributes[index].value, expected_value);
}

TEST(XmlNodeExtractorTest, test_normal_1) {
    constexpr std::string_view xml_content = R"(<svg></svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    ASSERT_TRUE(extracted.has_value());
    EXPECT_EQ(extracted->element_name, "svg");
    EXPECT_TRUE(extracted->attributes.empty());
    EXPECT_TRUE(extracted->children.empty());
}

TEST(XmlNodeExtractorTest, test_normal_2) {
    constexpr std::string_view xml_content = R"(<svg width="24" height='16' viewBox="0 0 24 16"></svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    ASSERT_TRUE(extracted.has_value());
    ASSERT_EQ(extracted->attributes.size(), 3U);
    expect_attribute(*extracted, 0, "width", "24");
    expect_attribute(*extracted, 1, "height", "16");
    expect_attribute(*extracted, 2, "viewBox", "0 0 24 16");
}

TEST(XmlNodeExtractorTest, test_normal_3) {
    constexpr std::string_view xml_content =
        R"(<svg><g id="layer"><path d="M0 0" /></g><rect width="10" height="20"></rect></svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    ASSERT_TRUE(extracted.has_value());
    ASSERT_EQ(extracted->children.size(), 2U);

    const auto& g_node = extracted->children[0];
    EXPECT_EQ(g_node.element_name, "g");
    ASSERT_EQ(g_node.attributes.size(), 1U);
    expect_attribute(g_node, 0, "id", "layer");
    ASSERT_EQ(g_node.children.size(), 1U);

    const auto& path_node = g_node.children[0];
    EXPECT_EQ(path_node.element_name, "path");
    EXPECT_TRUE(path_node.children.empty());
    ASSERT_EQ(path_node.attributes.size(), 1U);
    expect_attribute(path_node, 0, "d", "M0 0");

    const auto& rect_node = extracted->children[1];
    EXPECT_EQ(rect_node.element_name, "rect");
    ASSERT_EQ(rect_node.attributes.size(), 2U);
    expect_attribute(rect_node, 0, "width", "10");
    expect_attribute(rect_node, 1, "height", "20");
}

TEST(XmlNodeExtractorTest, test_normal_4) {
    constexpr std::string_view xml_content = R"(<svg><g/><path d="M1 1"/></svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    ASSERT_TRUE(extracted.has_value());
    ASSERT_EQ(extracted->children.size(), 2U);

    const auto& g_node = extracted->children[0];
    EXPECT_EQ(g_node.element_name, "g");
    EXPECT_TRUE(g_node.attributes.empty());
    EXPECT_TRUE(g_node.children.empty());

    const auto& path_node = extracted->children[1];
    EXPECT_EQ(path_node.element_name, "path");
    EXPECT_TRUE(path_node.children.empty());
    ASSERT_EQ(path_node.attributes.size(), 1U);
    expect_attribute(path_node, 0, "d", "M1 1");
}

TEST(XmlNodeExtractorTest, test_normal_5) {
    constexpr std::string_view xml_content =
        R"(<?xml version="1.0" encoding="UTF-8"?>
<svg><g/></svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    ASSERT_TRUE(extracted.has_value());
    EXPECT_EQ(extracted->element_name, "svg");
    ASSERT_EQ(extracted->children.size(), 1U);
    EXPECT_EQ(extracted->children[0].element_name, "g");
}

TEST(XmlNodeExtractorTest, test_normal_6) {
    constexpr std::string_view xml_content =
        R"(<!-- document comment -->
<svg><!-- child comment --><g id="layer"/><!-- trailing child comment --></svg>
<!-- after root comment -->)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    ASSERT_TRUE(extracted.has_value());
    ASSERT_EQ(extracted->children.size(), 1U);

    const auto& child = extracted->children[0];
    EXPECT_EQ(child.element_name, "g");
    ASSERT_EQ(child.attributes.size(), 1U);
    expect_attribute(child, 0, "id", "layer");
}

TEST(XmlNodeExtractorTest, test_normal_7) {
    constexpr std::string_view xml_content =
        R"(<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg><rect/></svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    ASSERT_TRUE(extracted.has_value());
    ASSERT_EQ(extracted->children.size(), 1U);
    EXPECT_EQ(extracted->children[0].element_name, "rect");
}

TEST(XmlNodeExtractorTest, test_normal_8) {
    constexpr std::string_view xml_content =
        R"(<svg>
  before
  <g>inside-text<path d="M0 0"/>after-text</g>
  tail
</svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    ASSERT_TRUE(extracted.has_value());
    ASSERT_EQ(extracted->children.size(), 1U);

    const auto& g_node = extracted->children[0];
    EXPECT_EQ(g_node.element_name, "g");
    ASSERT_EQ(g_node.children.size(), 1U);

    const auto& path_node = g_node.children[0];
    EXPECT_EQ(path_node.element_name, "path");
    ASSERT_EQ(path_node.attributes.size(), 1U);
    expect_attribute(path_node, 0, "d", "M0 0");
}

TEST(XmlNodeExtractorTest, test_abnormal_1) {
    constexpr std::string_view xml_content = R"(<svg><g></svg></g>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    EXPECT_FALSE(extracted.has_value());
}

TEST(XmlNodeExtractorTest, test_abnormal_2) {
    constexpr std::string_view xml_content = R"(<g></g>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    EXPECT_FALSE(extracted.has_value());
}

TEST(XmlNodeExtractorTest, test_abnormal_3) {
    constexpr std::string_view xml_content = R"(<svg width=24></svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    EXPECT_FALSE(extracted.has_value());
}

TEST(XmlNodeExtractorTest, test_abnormal_4) {
    constexpr std::string_view xml_content = R"(<svg width "24"></svg>)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    EXPECT_FALSE(extracted.has_value());
}

TEST(XmlNodeExtractorTest, test_abnormal_5) {
    constexpr std::string_view xml_content = R"(<svg)";

    XmlNodeExtractor extractor(xml_content);
    const auto extracted = extractor.extract_from_xml();

    EXPECT_FALSE(extracted.has_value());
}

}  // namespace
