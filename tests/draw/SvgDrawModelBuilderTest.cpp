#include <gtest/gtest.h>

#include <cmath>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "draw/DrawShape.h"
#include "draw/SvgDrawModelBuilder.h"
#include "interpreter/InterpretedSvg.h"
#include "interpreter/PathInstruction.h"
#include "interpreter/SvgElementInterpreter.h"
#include "parser/ExtractedNode.h"

namespace {

using svg::draw::DrawShape;
using svg::draw::SvgDrawModelBuilder;
using svg::interpreter::InterpretedSvg;
using svg::interpreter::PathInstruction;
using svg::interpreter::PathInstructionType;
using svg::interpreter::Point;
using svg::interpreter::StrokeLineCap;
using svg::interpreter::StrokeLineJoin;
using svg::interpreter::SvgElementInterpreter;
using svg::parser::ExtractedAttribute;
using svg::parser::ExtractedNode;

ExtractedNode create_node(
    std::string element_name,
    std::vector<ExtractedAttribute> attributes,
    std::vector<ExtractedNode> children = {}) {
    ExtractedNode node;
    node.element_name = std::move(element_name);
    node.attributes = std::move(attributes);
    node.children = std::move(children);
    return node;
}

ExtractedNode create_root(
    std::vector<ExtractedNode> children,
    std::string fill = "none",
    std::string stroke = "currentColor") {
    return create_node(
        "svg",
        {
            {"viewBox", "0 0 24 24"},
            {"width", "24"},
            {"height", "24"},
            {"stroke", std::move(stroke)},
            {"fill", std::move(fill)},
            {"stroke-width", "2"},
            {"stroke-linecap", "round"},
            {"stroke-linejoin", "round"},
        },
        std::move(children));
}

auto build_draw_shapes(const ExtractedNode& root)
    -> std::optional<std::vector<DrawShape>> {
    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);
    if (!interpreted.has_value()) {
        return std::nullopt;
    }
    return SvgDrawModelBuilder::build(*interpreted);
}

void expect_point(const Point& point, double expected_x, double expected_y) {
    EXPECT_DOUBLE_EQ(point.x, expected_x);
    EXPECT_DOUBLE_EQ(point.y, expected_y);
}

void expect_instruction(
    const PathInstruction& instruction,
    PathInstructionType expected_type,
    std::size_t expected_point_count) {
    EXPECT_EQ(instruction.type, expected_type);
    EXPECT_EQ(instruction.points.size(), expected_point_count);
}

TEST(SvgDrawModelBuilderTest, PathUsesSvgPathInstructions) {
    const auto result = build_draw_shapes(create_root({
        create_node("path", {{"d", "M1 2L3 4Z"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const DrawShape& shape = result->at(0);
    ASSERT_EQ(shape.path_instructions.size(), 3U);
    expect_instruction(shape.path_instructions[0], PathInstructionType::MoveTo, 1U);
    expect_point(shape.path_instructions[0].points[0], 1.0, 2.0);
    expect_instruction(shape.path_instructions[1], PathInstructionType::LineTo, 1U);
    expect_point(shape.path_instructions[1].points[0], 3.0, 4.0);
    expect_instruction(shape.path_instructions[2], PathInstructionType::ClosePath, 0U);
}

TEST(SvgDrawModelBuilderTest, CircleUsesFourQuarterCubicBezierSegments) {
    constexpr double CUBIC_ARC_FACTOR = 0.5522847498307933;

    const auto result = build_draw_shapes(create_root({
        create_node("circle", {{"cx", "10"}, {"cy", "20"}, {"r", "2"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const std::vector<PathInstruction>& instructions = result->at(0).path_instructions;
    ASSERT_EQ(instructions.size(), 6U);
    expect_instruction(instructions[0], PathInstructionType::MoveTo, 1U);
    expect_point(instructions[0].points[0], 12.0, 20.0);
    for (std::size_t index = 1; index <= 4; ++index) {
        expect_instruction(instructions[index], PathInstructionType::CubicBezierTo, 3U);
    }
    expect_point(instructions[1].points[0], 12.0, 20.0 + 2.0 * CUBIC_ARC_FACTOR);
    expect_point(instructions[1].points[1], 10.0 + 2.0 * CUBIC_ARC_FACTOR, 22.0);
    expect_point(instructions[1].points[2], 10.0, 22.0);
    expect_point(instructions[2].points[2], 8.0, 20.0);
    expect_point(instructions[3].points[2], 10.0, 18.0);
    expect_point(instructions[4].points[2], 12.0, 20.0);
    expect_instruction(instructions[5], PathInstructionType::ClosePath, 0U);
}

TEST(SvgDrawModelBuilderTest, EllipseUsesFourQuarterCubicBezierSegments) {
    constexpr double CUBIC_ARC_FACTOR = 0.5522847498307933;

    const auto result = build_draw_shapes(create_root({
        create_node("ellipse", {{"cx", "10"}, {"cy", "20"}, {"rx", "4"}, {"ry", "2"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const std::vector<PathInstruction>& instructions = result->at(0).path_instructions;
    ASSERT_EQ(instructions.size(), 6U);
    expect_point(instructions[0].points[0], 14.0, 20.0);
    expect_instruction(instructions[1], PathInstructionType::CubicBezierTo, 3U);
    expect_point(instructions[1].points[0], 14.0, 20.0 + 2.0 * CUBIC_ARC_FACTOR);
    expect_point(instructions[1].points[1], 10.0 + 4.0 * CUBIC_ARC_FACTOR, 22.0);
    expect_point(instructions[1].points[2], 10.0, 22.0);
    expect_point(instructions[4].points[2], 14.0, 20.0);
    expect_instruction(instructions[5], PathInstructionType::ClosePath, 0U);
}

TEST(SvgDrawModelBuilderTest, RectWithoutRadiiUsesClosedLinePath) {
    const auto result = build_draw_shapes(create_root({
        create_node("rect", {{"x", "1"}, {"y", "2"}, {"width", "6"}, {"height", "4"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const std::vector<PathInstruction>& instructions = result->at(0).path_instructions;
    ASSERT_EQ(instructions.size(), 5U);
    expect_instruction(instructions[0], PathInstructionType::MoveTo, 1U);
    expect_point(instructions[0].points[0], 1.0, 2.0);
    expect_instruction(instructions[1], PathInstructionType::LineTo, 1U);
    expect_point(instructions[1].points[0], 7.0, 2.0);
    expect_point(instructions[2].points[0], 7.0, 6.0);
    expect_point(instructions[3].points[0], 1.0, 6.0);
    expect_instruction(instructions[4], PathInstructionType::ClosePath, 0U);
}

TEST(SvgDrawModelBuilderTest, RoundedRectUsesCubicBezierCorners) {
    const auto result = build_draw_shapes(create_root({
        create_node(
            "rect",
            {{"x", "0"}, {"y", "0"}, {"width", "10"}, {"height", "8"}, {"rx", "2"}, {"ry", "3"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const std::vector<PathInstruction>& instructions = result->at(0).path_instructions;
    ASSERT_EQ(instructions.size(), 10U);
    expect_point(instructions[0].points[0], 2.0, 0.0);
    expect_instruction(instructions[1], PathInstructionType::LineTo, 1U);
    expect_point(instructions[1].points[0], 8.0, 0.0);
    expect_instruction(instructions[2], PathInstructionType::CubicBezierTo, 3U);
    expect_point(instructions[2].points[2], 10.0, 3.0);
    expect_instruction(instructions[4], PathInstructionType::CubicBezierTo, 3U);
    expect_instruction(instructions[6], PathInstructionType::CubicBezierTo, 3U);
    expect_instruction(instructions[8], PathInstructionType::CubicBezierTo, 3U);
    expect_instruction(instructions[9], PathInstructionType::ClosePath, 0U);
}

TEST(SvgDrawModelBuilderTest, RectWithOnlyRxCopiesRadiusToRy) {
    const auto result = build_draw_shapes(create_root({
        create_node(
            "rect",
            {{"x", "0"}, {"y", "0"}, {"width", "10"}, {"height", "8"}, {"rx", "2"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const std::vector<PathInstruction>& instructions = result->at(0).path_instructions;
    expect_point(instructions[0].points[0], 2.0, 0.0);
    expect_point(instructions[2].points[2], 10.0, 2.0);
}

TEST(SvgDrawModelBuilderTest, RectRadiiAreClampedToHalfDimensions) {
    const auto result = build_draw_shapes(create_root({
        create_node(
            "rect",
            {{"x", "0"}, {"y", "0"}, {"width", "10"}, {"height", "6"}, {"rx", "20"}, {"ry", "10"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const std::vector<PathInstruction>& instructions = result->at(0).path_instructions;
    expect_point(instructions[0].points[0], 5.0, 0.0);
    expect_point(instructions[2].points[2], 10.0, 3.0);
}

TEST(SvgDrawModelBuilderTest, LineUsesMoveAndLineAndNeverHasFill) {
    const auto result = build_draw_shapes(create_root(
        {
            create_node("line", {{"x1", "1"}, {"y1", "2"}, {"x2", "3"}, {"y2", "4"}}),
        },
        "currentColor"));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const DrawShape& shape = result->at(0);
    EXPECT_FALSE(shape.has_fill);
    EXPECT_TRUE(shape.has_stroke);
    ASSERT_EQ(shape.path_instructions.size(), 2U);
    expect_instruction(shape.path_instructions[0], PathInstructionType::MoveTo, 1U);
    expect_instruction(shape.path_instructions[1], PathInstructionType::LineTo, 1U);
    expect_point(shape.path_instructions[1].points[0], 3.0, 4.0);
}

TEST(SvgDrawModelBuilderTest, PolylineUsesOpenLinePath) {
    const auto result = build_draw_shapes(create_root({
        create_node("polyline", {{"points", "1,2 3,4 5,6"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const std::vector<PathInstruction>& instructions = result->at(0).path_instructions;
    ASSERT_EQ(instructions.size(), 3U);
    expect_instruction(instructions[0], PathInstructionType::MoveTo, 1U);
    expect_instruction(instructions[1], PathInstructionType::LineTo, 1U);
    expect_instruction(instructions[2], PathInstructionType::LineTo, 1U);
    expect_point(instructions[2].points[0], 5.0, 6.0);
}

TEST(SvgDrawModelBuilderTest, PolygonClosesLinePath) {
    const auto result = build_draw_shapes(create_root({
        create_node("polygon", {{"points", "1,2 3,4 5,6"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const std::vector<PathInstruction>& instructions = result->at(0).path_instructions;
    ASSERT_EQ(instructions.size(), 4U);
    expect_instruction(instructions[0], PathInstructionType::MoveTo, 1U);
    expect_instruction(instructions[1], PathInstructionType::LineTo, 1U);
    expect_instruction(instructions[2], PathInstructionType::LineTo, 1U);
    expect_instruction(instructions[3], PathInstructionType::ClosePath, 0U);
}

TEST(SvgDrawModelBuilderTest, ShapeFillOverridesRootAndStrokeIsInherited) {
    const auto result = build_draw_shapes(create_root({
        create_node(
            "circle",
            {{"cx", "2"}, {"cy", "3"}, {"r", "1"}, {"fill", "currentColor"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    const DrawShape& shape = result->at(0);
    EXPECT_TRUE(shape.has_fill);
    EXPECT_TRUE(shape.has_stroke);
    EXPECT_DOUBLE_EQ(shape.stroke_width, 2.0);
    EXPECT_EQ(shape.stroke_linecap, StrokeLineCap::Round);
    EXPECT_EQ(shape.stroke_linejoin, StrokeLineJoin::Round);
}

TEST(SvgDrawModelBuilderTest, ShapeStrokeOverridesRoot) {
    const auto result = build_draw_shapes(create_root({
        create_node(
            "circle",
            {{"cx", "2"}, {"cy", "3"}, {"r", "1"}, {"stroke", "none"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    EXPECT_FALSE(result->at(0).has_stroke);
}

TEST(SvgDrawModelBuilderTest, UnsupportedChildFillValueFails) {
    const auto result = build_draw_shapes(create_root({
        create_node(
            "circle",
            {{"cx", "2"}, {"cy", "3"}, {"r", "1"}, {"fill", "#000000"}}),
    }));

    EXPECT_FALSE(result.has_value());
}

TEST(SvgDrawModelBuilderTest, UnsupportedRootStrokeValueFails) {
    const auto result = build_draw_shapes(create_root(
        {
            create_node("circle", {{"cx", "2"}, {"cy", "3"}, {"r", "1"}}),
        },
        "none",
        "#000000"));

    EXPECT_FALSE(result.has_value());
}

TEST(SvgDrawModelBuilderTest, UnsupportedRootFillFailsWhenChildOverridesIt) {
    const auto result = build_draw_shapes(create_root(
        {
            create_node(
                "circle",
                {{"cx", "2"}, {"cy", "3"}, {"r", "1"}, {"fill", "currentColor"}}),
        },
        "#000000"));

    EXPECT_FALSE(result.has_value());
}

TEST(SvgDrawModelBuilderTest, CircleWithZeroRadiusIsOmitted) {
    const auto result = build_draw_shapes(create_root({
        create_node("circle", {{"cx", "2"}, {"cy", "3"}, {"r", "0"}}),
    }));

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST(SvgDrawModelBuilderTest, EllipseWithZeroRadiusIsOmitted) {
    const auto result = build_draw_shapes(create_root({
        create_node("ellipse", {{"cx", "2"}, {"cy", "3"}, {"rx", "0"}, {"ry", "1"}}),
    }));

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST(SvgDrawModelBuilderTest, EllipseWithZeroVerticalRadiusIsOmitted) {
    const auto result = build_draw_shapes(create_root({
        create_node("ellipse", {{"cx", "2"}, {"cy", "3"}, {"rx", "1"}, {"ry", "0"}}),
    }));

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST(SvgDrawModelBuilderTest, RectWithZeroDimensionIsOmitted) {
    const auto result = build_draw_shapes(create_root({
        create_node("rect", {{"x", "1"}, {"y", "2"}, {"width", "0"}, {"height", "4"}}),
    }));

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST(SvgDrawModelBuilderTest, RectWithZeroHeightIsOmitted) {
    const auto result = build_draw_shapes(create_root({
        create_node("rect", {{"x", "1"}, {"y", "2"}, {"width", "4"}, {"height", "0"}}),
    }));

    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST(SvgDrawModelBuilderTest, NegativeCircleRadiusFails) {
    const auto result = build_draw_shapes(create_root({
        create_node("circle", {{"cx", "2"}, {"cy", "3"}, {"r", "-1"}}),
    }));

    EXPECT_FALSE(result.has_value());
}

TEST(SvgDrawModelBuilderTest, NegativeRectDimensionFails) {
    const auto result = build_draw_shapes(create_root({
        create_node("rect", {{"x", "1"}, {"y", "2"}, {"width", "-1"}, {"height", "4"}}),
    }));

    EXPECT_FALSE(result.has_value());
}

TEST(SvgDrawModelBuilderTest, MalformedShapeIsNotSilentlySkipped) {
    const auto root = create_root({
        create_node("circle", {{"cx", "2"}, {"cy", "3"}, {"r", "1"}}),
        create_node("rect", {{"x", "1"}, {"y", "2"}, {"width", "invalid"}, {"height", "4"}}),
    });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    EXPECT_FALSE(interpreted.has_value());
}

TEST(SvgDrawModelBuilderTest, FailedPathConversionFailsWholeModel) {
    const auto result = build_draw_shapes(create_root({
        create_node("circle", {{"cx", "2"}, {"cy", "3"}, {"r", "1"}}),
        create_node("path", {{"d", "M0 0 L"}}),
    }));

    EXPECT_FALSE(result.has_value());
}

TEST(SvgDrawModelBuilderTest, DrawShapesKeepSvgDocumentOrder) {
    const auto result = build_draw_shapes(create_root({
        create_node("path", {{"d", "M1 1L2 2"}}),
        create_node("circle", {{"cx", "10"}, {"cy", "20"}, {"r", "2"}}),
    }));

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_point(result->at(0).path_instructions[0].points[0], 1.0, 1.0);
    expect_point(result->at(1).path_instructions[0].points[0], 12.0, 20.0);
}

}  // namespace
