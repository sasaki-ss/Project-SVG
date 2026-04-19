#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "interpreter/InterpretedSvg.h"
#include "interpreter/SvgElementInterpreter.h"
#include "interpreter/SvgShapeInterpreter.h"
#include "parser/ExtractedNode.h"

namespace {

using svg::interpreter::CircleElement;
using svg::interpreter::EllipseElement;
using svg::interpreter::InterpretedSvg;
using svg::interpreter::LineElement;
using svg::interpreter::PathElement;
using svg::interpreter::Point;
using svg::interpreter::PolygonElement;
using svg::interpreter::PolylineElement;
using svg::interpreter::RectElement;
using svg::interpreter::StrokeLineCap;
using svg::interpreter::StrokeLineJoin;
using svg::interpreter::SvgElementInterpreter;
using svg::interpreter::SvgElementType;
using svg::interpreter::SvgShape;
using svg::interpreter::SvgShapeInterpreter;
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

void expect_point(const Point& point, double expected_x, double expected_y) {
    EXPECT_DOUBLE_EQ(point.x, expected_x);
    EXPECT_DOUBLE_EQ(point.y, expected_y);
}

TEST(SvgInterpreterTest, test_normal_1) {
    const auto root = create_node(
        "svg",
        {
            {"viewBox", "0 0 24 24"},
            {"width", "24"},
            {"height", "24"},
            {"stroke", "currentColor"},
            {"fill", "none"},
            {"stroke-width", "2"},
            {"stroke-linecap", "round"},
            {"stroke-linejoin", "round"},
        });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    EXPECT_TRUE(interpreted.has_value());
}

TEST(SvgInterpreterTest, test_normal_2) {
    const auto root = create_node(
        "svg",
        {
            {"viewBox", "1 2 30 40"},
            {"width", "30"},
            {"height", "40"},
            {"stroke", "#333"},
            {"fill", "none"},
            {"stroke-width", "1.5"},
            {"stroke-linecap", "round"},
            {"stroke-linejoin", "round"},
        });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    ASSERT_TRUE(interpreted.has_value());
    EXPECT_DOUBLE_EQ(interpreted->view_box.min_x, 1.0);
    EXPECT_DOUBLE_EQ(interpreted->view_box.min_y, 2.0);
    EXPECT_DOUBLE_EQ(interpreted->view_box.width, 30.0);
    EXPECT_DOUBLE_EQ(interpreted->view_box.height, 40.0);
    EXPECT_DOUBLE_EQ(interpreted->width, 30.0);
    EXPECT_DOUBLE_EQ(interpreted->height, 40.0);
    EXPECT_EQ(interpreted->root_style.stroke, "#333");
    EXPECT_EQ(interpreted->root_style.fill, "none");
    EXPECT_DOUBLE_EQ(interpreted->root_style.stroke_width, 1.5);
    EXPECT_EQ(interpreted->root_style.stroke_linecap, StrokeLineCap::Round);
    EXPECT_EQ(interpreted->root_style.stroke_linejoin, StrokeLineJoin::Round);
}

TEST(SvgInterpreterTest, test_normal_3) {
    const auto root = create_node(
        "svg",
        {
            {"viewBox", "0 0 24 24"},
            {"width", "24"},
            {"height", "24"},
            {"stroke", "currentColor"},
            {"fill", "none"},
            {"stroke-width", "2"},
            {"stroke-linecap", "round"},
            {"stroke-linejoin", "round"},
        },
        {
            create_node("g", {}, {create_node("path", {{"d", "M1 1L2 2"}})}),
            create_node("circle", {{"cx", "3"}, {"cy", "4"}, {"r", "5"}}),
        });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    ASSERT_TRUE(interpreted.has_value());
    ASSERT_EQ(interpreted->shapes.size(), 2U);
    EXPECT_EQ(interpreted->shapes[0].type, SvgElementType::Path);
    ASSERT_TRUE(std::holds_alternative<PathElement>(interpreted->shapes[0].data));
    EXPECT_EQ(std::get<PathElement>(interpreted->shapes[0].data).d, "M1 1L2 2");

    EXPECT_EQ(interpreted->shapes[1].type, SvgElementType::Circle);
    ASSERT_TRUE(std::holds_alternative<CircleElement>(interpreted->shapes[1].data));

    const auto& circle = std::get<CircleElement>(interpreted->shapes[1].data);
    EXPECT_DOUBLE_EQ(circle.cx, 3.0);
    EXPECT_DOUBLE_EQ(circle.cy, 4.0);
    EXPECT_DOUBLE_EQ(circle.r, 5.0);
}

TEST(SvgInterpreterTest, test_normal_4) {
    const auto node = create_node("path", {{"d", "M0 0L10 10"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    ASSERT_TRUE(shape.has_value());
    EXPECT_EQ(shape->type, SvgElementType::Path);
    ASSERT_TRUE(std::holds_alternative<PathElement>(shape->data));
    EXPECT_EQ(std::get<PathElement>(shape->data).d, "M0 0L10 10");
}

TEST(SvgInterpreterTest, test_normal_5) {
    const auto node = create_node("circle", {{"cx", "5"}, {"cy", "6"}, {"r", "7"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    ASSERT_TRUE(shape.has_value());
    EXPECT_EQ(shape->type, SvgElementType::Circle);
    ASSERT_TRUE(std::holds_alternative<CircleElement>(shape->data));

    const auto& circle = std::get<CircleElement>(shape->data);
    EXPECT_DOUBLE_EQ(circle.cx, 5.0);
    EXPECT_DOUBLE_EQ(circle.cy, 6.0);
    EXPECT_DOUBLE_EQ(circle.r, 7.0);
}

TEST(SvgInterpreterTest, test_normal_6) {
    const auto node = create_node("rect", {{"x", "1"}, {"y", "2"}, {"width", "10"}, {"height", "20"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    ASSERT_TRUE(shape.has_value());
    EXPECT_EQ(shape->type, SvgElementType::Rect);
    ASSERT_TRUE(std::holds_alternative<RectElement>(shape->data));

    const auto& rect = std::get<RectElement>(shape->data);
    EXPECT_DOUBLE_EQ(rect.x, 1.0);
    EXPECT_DOUBLE_EQ(rect.y, 2.0);
    EXPECT_DOUBLE_EQ(rect.width, 10.0);
    EXPECT_DOUBLE_EQ(rect.height, 20.0);
    EXPECT_FALSE(rect.rx.has_value());
    EXPECT_FALSE(rect.ry.has_value());
}

TEST(SvgInterpreterTest, test_normal_7) {
    const auto node = create_node(
        "rect",
        {{"x", "1"}, {"y", "2"}, {"width", "10"}, {"height", "20"}, {"rx", "3"}, {"ry", "4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    ASSERT_TRUE(shape.has_value());
    EXPECT_EQ(shape->type, SvgElementType::Rect);
    ASSERT_TRUE(std::holds_alternative<RectElement>(shape->data));

    const auto& rect = std::get<RectElement>(shape->data);
    ASSERT_TRUE(rect.rx.has_value());
    ASSERT_TRUE(rect.ry.has_value());
    EXPECT_DOUBLE_EQ(*rect.rx, 3.0);
    EXPECT_DOUBLE_EQ(*rect.ry, 4.0);
}

TEST(SvgInterpreterTest, test_normal_8) {
    const auto node = create_node("line", {{"x1", "1"}, {"y1", "2"}, {"x2", "3"}, {"y2", "4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    ASSERT_TRUE(shape.has_value());
    EXPECT_EQ(shape->type, SvgElementType::Line);
    ASSERT_TRUE(std::holds_alternative<LineElement>(shape->data));

    const auto& line = std::get<LineElement>(shape->data);
    EXPECT_DOUBLE_EQ(line.x1, 1.0);
    EXPECT_DOUBLE_EQ(line.y1, 2.0);
    EXPECT_DOUBLE_EQ(line.x2, 3.0);
    EXPECT_DOUBLE_EQ(line.y2, 4.0);
}

TEST(SvgInterpreterTest, test_normal_9) {
    const auto node = create_node("ellipse", {{"cx", "3"}, {"cy", "4"}, {"rx", "5"}, {"ry", "6"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    ASSERT_TRUE(shape.has_value());
    EXPECT_EQ(shape->type, SvgElementType::Ellipse);
    ASSERT_TRUE(std::holds_alternative<EllipseElement>(shape->data));

    const auto& ellipse = std::get<EllipseElement>(shape->data);
    EXPECT_DOUBLE_EQ(ellipse.cx, 3.0);
    EXPECT_DOUBLE_EQ(ellipse.cy, 4.0);
    EXPECT_DOUBLE_EQ(ellipse.rx, 5.0);
    EXPECT_DOUBLE_EQ(ellipse.ry, 6.0);
}

TEST(SvgInterpreterTest, test_normal_10) {
    const auto node = create_node("polyline", {{"points", "1 2 3 4 5 6"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    ASSERT_TRUE(shape.has_value());
    EXPECT_EQ(shape->type, SvgElementType::Polyline);
    ASSERT_TRUE(std::holds_alternative<PolylineElement>(shape->data));

    const auto& points = std::get<PolylineElement>(shape->data).points;
    ASSERT_EQ(points.size(), 3U);
    expect_point(points[0], 1.0, 2.0);
    expect_point(points[1], 3.0, 4.0);
    expect_point(points[2], 5.0, 6.0);
}

TEST(SvgInterpreterTest, test_normal_11) {
    const auto node = create_node("polygon", {{"points", "7,8 9,10 11,12"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    ASSERT_TRUE(shape.has_value());
    EXPECT_EQ(shape->type, SvgElementType::Polygon);
    ASSERT_TRUE(std::holds_alternative<PolygonElement>(shape->data));

    const auto& points = std::get<PolygonElement>(shape->data).points;
    ASSERT_EQ(points.size(), 3U);
    expect_point(points[0], 7.0, 8.0);
    expect_point(points[1], 9.0, 10.0);
    expect_point(points[2], 11.0, 12.0);
}

TEST(SvgInterpreterTest, test_abnormal_1) {
    const auto root = create_node(
        "g",
        {
            {"viewBox", "0 0 24 24"},
            {"width", "24"},
            {"height", "24"},
            {"stroke", "currentColor"},
            {"fill", "none"},
            {"stroke-width", "2"},
            {"stroke-linecap", "round"},
            {"stroke-linejoin", "round"},
        });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    EXPECT_FALSE(interpreted.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_2) {
    const auto root = create_node(
        "svg",
        {
            {"viewBox", "0 0 24"},
            {"width", "24"},
            {"height", "24"},
            {"stroke", "currentColor"},
            {"fill", "none"},
            {"stroke-width", "2"},
            {"stroke-linecap", "round"},
            {"stroke-linejoin", "round"},
        });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    EXPECT_FALSE(interpreted.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_3) {
    const auto root = create_node(
        "svg",
        {
            {"viewBox", "0 0 24 24"},
            {"height", "24"},
            {"stroke", "currentColor"},
            {"fill", "none"},
            {"stroke-width", "2"},
            {"stroke-linecap", "round"},
            {"stroke-linejoin", "round"},
        });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    EXPECT_FALSE(interpreted.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_4) {
    const auto root = create_node(
        "svg",
        {
            {"viewBox", "0 0 24 24"},
            {"width", "24"},
            {"height", "24"},
            {"stroke", "currentColor"},
            {"fill", "none"},
            {"stroke-width", "2"},
            {"stroke-linecap", "square"},
            {"stroke-linejoin", "round"},
        });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    EXPECT_FALSE(interpreted.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_5) {
    const auto root = create_node(
        "svg",
        {
            {"viewBox", "0 0 24 24"},
            {"width", "24"},
            {"height", "24"},
            {"stroke", "currentColor"},
            {"fill", "none"},
            {"stroke-width", "2"},
            {"stroke-linecap", "round"},
            {"stroke-linejoin", "bevel"},
        });

    const std::optional<InterpretedSvg> interpreted = SvgElementInterpreter::interpret(root);

    EXPECT_FALSE(interpreted.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_6) {
    const auto node = create_node("g", {});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_7) {
    const auto node = create_node("path", {});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_8) {
    const auto node = create_node("circle", {{"cx", "5"}, {"r", "7"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_9) {
    const auto node = create_node("circle", {{"cx", "invalid"}, {"cy", "6"}, {"r", "7"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_10) {
    const auto node = create_node("rect", {{"x", "1"}, {"width", "10"}, {"height", "20"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_11) {
    const auto node = create_node(
        "rect",
        {{"x", "1"}, {"y", "2"}, {"width", "10"}, {"height", "20"}, {"rx", "invalid"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_12) {
    const auto node = create_node(
        "rect",
        {{"x", "1"}, {"y", "2"}, {"width", "10"}, {"height", "20"}, {"ry", "invalid"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_13) {
    const auto node = create_node("line", {{"x1", "1"}, {"x2", "3"}, {"y2", "4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_14) {
    const auto node = create_node("line", {{"x1", "1"}, {"y1", "invalid"}, {"x2", "3"}, {"y2", "4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_15) {
    const auto node = create_node("ellipse", {{"cx", "3"}, {"cy", "4"}, {"rx", "5"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_16) {
    const auto node = create_node("ellipse", {{"cx", "3"}, {"cy", "4"}, {"rx", "invalid"}, {"ry", "6"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_17) {
    const auto node = create_node("polyline", {});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_18) {
    const auto node = create_node("polyline", {{"points", "1 2 3"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_19) {
    const auto node = create_node("polyline", {{"points", "1 2 x 4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_20) {
    const auto node = create_node("polygon", {});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_21) {
    const auto node = create_node("polygon", {{"points", "7 8 9"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

TEST(SvgInterpreterTest, test_abnormal_22) {
    const auto node = create_node("polygon", {{"points", "7 8 invalid 10"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    EXPECT_FALSE(shape.has_value());
}

}  // namespace
