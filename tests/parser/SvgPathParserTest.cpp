#include <gtest/gtest.h>

#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "parser/PathCommand.h"
#include "parser/SvgPathParser.h"

namespace {

using svg::parser::PathCommand;
using svg::parser::PathCommandType;
using svg::parser::SvgPathParser;

std::optional<std::vector<PathCommand>> parse_path(std::string_view d) {
    SvgPathParser parser;
    return parser.parse(std::string(d));
}

void expect_command(const PathCommand& command,
                    PathCommandType expected_type,
                    bool expected_is_absolute,
                    std::initializer_list<std::string_view> expected_parameters) {
    EXPECT_EQ(command.type, expected_type);
    EXPECT_EQ(command.is_absolute, expected_is_absolute);
    ASSERT_EQ(command.parameters.size(), expected_parameters.size());

    std::size_t index = 0;
    for (const std::string_view expected_parameter : expected_parameters) {
        EXPECT_EQ(command.parameters[index], expected_parameter);
        ++index;
    }
}

TEST(SvgPathParserTest, test_normal_001) {
    const auto result = parse_path("M 10 20");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"10", "20"});
}

TEST(SvgPathParserTest, test_normal_002) {
    const auto result = parse_path("l 30 40");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::LineTo, false, {"30", "40"});
}

TEST(SvgPathParserTest, test_normal_003) {
    const auto result = parse_path("H 50");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::HorizontalTo, true, {"50"});
}

TEST(SvgPathParserTest, test_normal_004) {
    const auto result = parse_path("v 60");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::VerticalTo, false, {"60"});
}

TEST(SvgPathParserTest, test_normal_005) {
    const auto result = parse_path("C 1 2 3 4 5 6");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::CubicBezierTo, true, {"1", "2", "3", "4", "5", "6"});
}

TEST(SvgPathParserTest, test_normal_006) {
    const auto result = parse_path("Z");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::ClosePath, true, {});
}

TEST(SvgPathParserTest, test_normal_007) {
    const auto result = parse_path("s 1 2 3 4");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::SmoothCubicBezierTo, false, {"1", "2", "3", "4"});
}

TEST(SvgPathParserTest, test_normal_008) {
    const auto result = parse_path("Q 1 2 3 4");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::QuadraticBezierTo, true, {"1", "2", "3", "4"});
}

TEST(SvgPathParserTest, test_normal_009) {
    const auto result = parse_path("t 7 8");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::SmoothQuadraticBezierTo, false, {"7", "8"});
}

TEST(SvgPathParserTest, test_normal_010) {
    const auto result = parse_path("A 1 2 0 0 1 30 40");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::ArcTo, true, {"1", "2", "0", "0", "1", "30", "40"});
}

TEST(SvgPathParserTest, test_normal_011) {
    const auto result = parse_path("M10 20 L30 40");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"10", "20"});
    expect_command(result->at(1), PathCommandType::LineTo, true, {"30", "40"});
}

TEST(SvgPathParserTest, test_normal_012) {
    const auto result = parse_path("M 10 20 L 30 40 Z");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"10", "20"});
    expect_command(result->at(1), PathCommandType::LineTo, true, {"30", "40"});
    expect_command(result->at(2), PathCommandType::ClosePath, true, {});
}

TEST(SvgPathParserTest, test_normal_013) {
    const auto result = parse_path("M10,20L30,40");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"10", "20"});
    expect_command(result->at(1), PathCommandType::LineTo, true, {"30", "40"});
}

TEST(SvgPathParserTest, test_normal_014) {
    const auto result = parse_path("M10 20H30V40");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"10", "20"});
    expect_command(result->at(1), PathCommandType::HorizontalTo, true, {"30"});
    expect_command(result->at(2), PathCommandType::VerticalTo, true, {"40"});
}

TEST(SvgPathParserTest, test_normal_015) {
    const auto result = parse_path("M10-20L30-40");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"10", "-20"});
    expect_command(result->at(1), PathCommandType::LineTo, true, {"30", "-40"});
}

TEST(SvgPathParserTest, test_normal_016) {
    const auto result = parse_path("M10 20L30 40H50");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"10", "20"});
    expect_command(result->at(1), PathCommandType::LineTo, true, {"30", "40"});
    expect_command(result->at(2), PathCommandType::HorizontalTo, true, {"50"});
}

TEST(SvgPathParserTest, test_abnormal_001) {
    const auto result = parse_path("M 10");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_002) {
    const auto result = parse_path("L 10");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_003) {
    const auto result = parse_path("H");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_004) {
    const auto result = parse_path("V");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_005) {
    const auto result = parse_path("C 1 2 3 4 5");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_006) {
    const auto result = parse_path("Z 1");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_007) {
    const auto result = parse_path("S 1 2 3");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_008) {
    const auto result = parse_path("Q 1 2 3");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_009) {
    const auto result = parse_path("T 1");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_010) {
    const auto result = parse_path("A 1 2 0 0 1 30");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_011) {
    const auto result = parse_path("@ 10 20");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_012) {
    const auto result = parse_path("M @ 20");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_013) {
    const auto result = parse_path("M 10 ? 20");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_014) {
    const auto result = parse_path("X 10 20");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_015) {
    const auto result = parse_path("10 20");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_016) {
    const auto result = parse_path("");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_017) {
    const auto result = parse_path("M , ,");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_018) {
    const auto result = parse_path("M 10 #");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_019) {
    const auto result = parse_path("M10 20 L30");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_020) {
    const auto result = parse_path("M10 20 Z 1");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_021) {
    const auto result = parse_path("M10 20 X30 40");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_022) {
    const auto result = parse_path("M10 20 ? 30 40");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_abnormal_023) {
    const auto result = parse_path("M10 20 L30 x40");

    EXPECT_FALSE(result.has_value());
}

TEST(SvgPathParserTest, test_boundary_001) {
    const auto result = parse_path("M1&0 20");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"1&0", "20"});
}

TEST(SvgPathParserTest, test_boundary_002) {
    const auto result = parse_path("M10x20 30");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"10x20", "30"});
}

TEST(SvgPathParserTest, test_boundary_003) {
    const auto result = parse_path("M1_0 20");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"1_0", "20"});
}

TEST(SvgPathParserTest, test_boundary_004) {
    const auto result = parse_path("C1x2 3 4 5 6 7");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::CubicBezierTo, true, {"1x2", "3", "4", "5", "6", "7"});
}

TEST(SvgPathParserTest, test_boundary_005) {
    const auto result = parse_path("M1e3 20");

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    expect_command(result->at(0), PathCommandType::MoveTo, true, {"1e3", "20"});
}

}  // namespace
