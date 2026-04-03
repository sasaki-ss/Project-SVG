#include <iostream>
#include <optional>
#include <string>
#include <string_view>
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

bool expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        return false;
    }

    return true;
}

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

bool expect_point(const Point& point, double expected_x, double expected_y, std::string_view case_name) {
    if (!expect(point.x == expected_x, std::string(case_name) + ": unexpected x value.")) {
        return false;
    }

    return expect(point.y == expected_y, std::string(case_name) + ": unexpected y value.");
}



bool test_interpret_svg_root_not_svg_returns_nullopt() {
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

    return expect(!interpreted.has_value(), "element invalid root: expected nullopt.");
}

bool test_interpret_svg_root_invalid_view_box_returns_nullopt() {
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

    return expect(!interpreted.has_value(), "element invalid viewBox: expected nullopt.");
}

bool test_interpret_svg_root_missing_required_attribute_returns_nullopt() {
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

    return expect(!interpreted.has_value(), "element missing required attribute: expected nullopt.");
}

bool test_interpret_svg_root_invalid_stroke_linecap_returns_nullopt() {
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

    return expect(!interpreted.has_value(), "element invalid stroke-linecap: expected nullopt.");
}

bool test_interpret_svg_root_invalid_stroke_linejoin_returns_nullopt() {
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

    return expect(!interpreted.has_value(), "element invalid stroke-linejoin: expected nullopt.");
}

bool test_interpret_unsupported_shape_returns_nullopt() {
    const auto node = create_node("g", {});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape unsupported element: expected nullopt.");
}

bool test_interpret_path_missing_d_returns_nullopt() {
    const auto node = create_node("path", {});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape path missing d: expected nullopt.");
}

bool test_interpret_circle_missing_required_attribute_returns_nullopt() {
    const auto node = create_node("circle", {{"cx", "5"}, {"r", "7"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape circle missing required attribute: expected nullopt.");
}

bool test_interpret_circle_invalid_number_returns_nullopt() {
    const auto node = create_node("circle", {{"cx", "invalid"}, {"cy", "6"}, {"r", "7"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape circle invalid number: expected nullopt.");
}

bool test_interpret_rect_missing_required_attribute_returns_nullopt() {
    const auto node = create_node("rect", {{"x", "1"}, {"width", "10"}, {"height", "20"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape rect missing required attribute: expected nullopt.");
}

bool test_interpret_rect_invalid_rx_returns_nullopt() {
    const auto node = create_node(
        "rect",
        {{"x", "1"}, {"y", "2"}, {"width", "10"}, {"height", "20"}, {"rx", "invalid"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape rect invalid rx: expected nullopt.");
}

bool test_interpret_rect_invalid_ry_returns_nullopt() {
    const auto node = create_node(
        "rect",
        {{"x", "1"}, {"y", "2"}, {"width", "10"}, {"height", "20"}, {"ry", "invalid"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape rect invalid ry: expected nullopt.");
}

bool test_interpret_line_missing_required_attribute_returns_nullopt() {
    const auto node = create_node("line", {{"x1", "1"}, {"x2", "3"}, {"y2", "4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape line missing required attribute: expected nullopt.");
}

bool test_interpret_line_invalid_number_returns_nullopt() {
    const auto node = create_node("line", {{"x1", "1"}, {"y1", "invalid"}, {"x2", "3"}, {"y2", "4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape line invalid number: expected nullopt.");
}

bool test_interpret_ellipse_missing_required_attribute_returns_nullopt() {
    const auto node = create_node("ellipse", {{"cx", "3"}, {"cy", "4"}, {"rx", "5"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape ellipse missing required attribute: expected nullopt.");
}

bool test_interpret_ellipse_invalid_number_returns_nullopt() {
    const auto node = create_node("ellipse", {{"cx", "3"}, {"cy", "4"}, {"rx", "invalid"}, {"ry", "6"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape ellipse invalid number: expected nullopt.");
}

bool test_interpret_polyline_missing_points_returns_nullopt() {
    const auto node = create_node("polyline", {});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape polyline missing points: expected nullopt.");
}

bool test_interpret_polyline_odd_points_count_returns_nullopt() {
    const auto node = create_node("polyline", {{"points", "1 2 3"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape polyline odd points count: expected nullopt.");
}

bool test_interpret_polyline_invalid_points_token_returns_nullopt() {
    const auto node = create_node("polyline", {{"points", "1 2 x 4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape polyline invalid points token: expected nullopt.");
}

bool test_interpret_polygon_missing_points_returns_nullopt() {
    const auto node = create_node("polygon", {});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape polygon missing points: expected nullopt.");
}

bool test_interpret_polygon_odd_points_count_returns_nullopt() {
    const auto node = create_node("polygon", {{"points", "7 8 9"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape polygon odd points count: expected nullopt.");
}

bool test_interpret_polygon_invalid_points_token_returns_nullopt() {
    const auto node = create_node("polygon", {{"points", "7 8 invalid 10"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    return expect(!shape.has_value(), "shape polygon invalid points token: expected nullopt.");
}

bool test_interpret_svg_root_minimal_success() {
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

    return expect(interpreted.has_value(), "element minimal: expected interpreted svg.");
}

bool test_interpret_svg_root_attributes_reflected() {
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

    if (!expect(interpreted.has_value(), "element attributes: expected interpreted svg.")) {
        return false;
    }

    if (!expect(interpreted->view_box.min_x == 1.0, "element attributes: unexpected viewBox min_x.")) {
        return false;
    }
    if (!expect(interpreted->view_box.min_y == 2.0, "element attributes: unexpected viewBox min_y.")) {
        return false;
    }
    if (!expect(interpreted->view_box.width == 30.0, "element attributes: unexpected viewBox width.")) {
        return false;
    }
    if (!expect(interpreted->view_box.height == 40.0, "element attributes: unexpected viewBox height.")) {
        return false;
    }
    if (!expect(interpreted->width == 30.0, "element attributes: unexpected root width.")) {
        return false;
    }
    if (!expect(interpreted->height == 40.0, "element attributes: unexpected root height.")) {
        return false;
    }
    if (!expect(interpreted->root_style.stroke == "#333", "element attributes: unexpected stroke.")) {
        return false;
    }
    if (!expect(interpreted->root_style.fill == "none", "element attributes: unexpected fill.")) {
        return false;
    }
    if (!expect(interpreted->root_style.stroke_width == 1.5, "element attributes: unexpected stroke-width.")) {
        return false;
    }
    if (!expect(interpreted->root_style.stroke_linecap == StrokeLineCap::Round,
                "element attributes: unexpected stroke-linecap.")) {
        return false;
    }

    return expect(interpreted->root_style.stroke_linejoin == StrokeLineJoin::Round,
                  "element attributes: unexpected stroke-linejoin.");
}

bool test_interpret_svg_root_collects_child_shapes() {
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

    if (!expect(interpreted.has_value(), "element children: expected interpreted svg.")) {
        return false;
    }

    if (!expect(interpreted->shapes.size() == 2, "element children: expected two collected shapes.")) {
        return false;
    }

    if (!expect(interpreted->shapes[0].type == SvgElementType::Path, "element children: first shape must be path.")) {
        return false;
    }

    if (!expect(std::holds_alternative<PathElement>(interpreted->shapes[0].data),
                "element children: first shape data type must be PathElement.")) {
        return false;
    }

    if (!expect(std::get<PathElement>(interpreted->shapes[0].data).d == "M1 1L2 2",
                "element children: path d must be preserved.")) {
        return false;
    }

    if (!expect(interpreted->shapes[1].type == SvgElementType::Circle,
                "element children: second shape must be circle.")) {
        return false;
    }

    if (!expect(std::holds_alternative<CircleElement>(interpreted->shapes[1].data),
                "element children: second shape data type must be CircleElement.")) {
        return false;
    }

    const auto& circle = std::get<CircleElement>(interpreted->shapes[1].data);
    if (!expect(circle.cx == 3.0, "element children: unexpected circle cx.")) {
        return false;
    }
    if (!expect(circle.cy == 4.0, "element children: unexpected circle cy.")) {
        return false;
    }

    return expect(circle.r == 5.0, "element children: unexpected circle r.");
}

bool test_interpret_path_success() {
    const auto node = create_node("path", {{"d", "M0 0L10 10"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    if (!expect(shape.has_value(), "shape path: expected parsed shape.")) {
        return false;
    }

    if (!expect(shape->type == SvgElementType::Path, "shape path: unexpected shape type.")) {
        return false;
    }

    if (!expect(std::holds_alternative<PathElement>(shape->data), "shape path: unexpected shape data type.")) {
        return false;
    }

    return expect(std::get<PathElement>(shape->data).d == "M0 0L10 10", "shape path: d must be preserved.");
}

bool test_interpret_circle_success() {
    const auto node = create_node("circle", {{"cx", "5"}, {"cy", "6"}, {"r", "7"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    if (!expect(shape.has_value(), "shape circle: expected parsed shape.")) {
        return false;
    }

    if (!expect(shape->type == SvgElementType::Circle, "shape circle: unexpected shape type.")) {
        return false;
    }

    if (!expect(std::holds_alternative<CircleElement>(shape->data),
                "shape circle: unexpected shape data type.")) {
        return false;
    }

    const auto& circle = std::get<CircleElement>(shape->data);
    if (!expect(circle.cx == 5.0, "shape circle: unexpected cx.")) {
        return false;
    }
    if (!expect(circle.cy == 6.0, "shape circle: unexpected cy.")) {
        return false;
    }

    return expect(circle.r == 7.0, "shape circle: unexpected r.");
}

bool test_interpret_rect_without_rx_ry_success() {
    const auto node = create_node("rect", {{"x", "1"}, {"y", "2"}, {"width", "10"}, {"height", "20"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    if (!expect(shape.has_value(), "shape rect no-rx-ry: expected parsed shape.")) {
        return false;
    }

    if (!expect(shape->type == SvgElementType::Rect, "shape rect no-rx-ry: unexpected shape type.")) {
        return false;
    }

    if (!expect(std::holds_alternative<RectElement>(shape->data),
                "shape rect no-rx-ry: unexpected shape data type.")) {
        return false;
    }

    const auto& rect = std::get<RectElement>(shape->data);
    if (!expect(rect.x == 1.0, "shape rect no-rx-ry: unexpected x.")) {
        return false;
    }
    if (!expect(rect.y == 2.0, "shape rect no-rx-ry: unexpected y.")) {
        return false;
    }
    if (!expect(rect.width == 10.0, "shape rect no-rx-ry: unexpected width.")) {
        return false;
    }
    if (!expect(rect.height == 20.0, "shape rect no-rx-ry: unexpected height.")) {
        return false;
    }
    if (!expect(!rect.rx.has_value(), "shape rect no-rx-ry: rx must be empty.")) {
        return false;
    }

    return expect(!rect.ry.has_value(), "shape rect no-rx-ry: ry must be empty.");
}

bool test_interpret_rect_with_rx_ry_success() {
    const auto node = create_node(
        "rect",
        {{"x", "1"}, {"y", "2"}, {"width", "10"}, {"height", "20"}, {"rx", "3"}, {"ry", "4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    if (!expect(shape.has_value(), "shape rect with-rx-ry: expected parsed shape.")) {
        return false;
    }

    if (!expect(shape->type == SvgElementType::Rect, "shape rect with-rx-ry: unexpected shape type.")) {
        return false;
    }

    if (!expect(std::holds_alternative<RectElement>(shape->data),
                "shape rect with-rx-ry: unexpected shape data type.")) {
        return false;
    }

    const auto& rect = std::get<RectElement>(shape->data);
    if (!expect(rect.rx.has_value(), "shape rect with-rx-ry: rx must exist.")) {
        return false;
    }
    if (!expect(rect.ry.has_value(), "shape rect with-rx-ry: ry must exist.")) {
        return false;
    }
    if (!expect(*rect.rx == 3.0, "shape rect with-rx-ry: unexpected rx.")) {
        return false;
    }

    return expect(*rect.ry == 4.0, "shape rect with-rx-ry: unexpected ry.");
}

bool test_interpret_line_success() {
    const auto node = create_node("line", {{"x1", "1"}, {"y1", "2"}, {"x2", "3"}, {"y2", "4"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    if (!expect(shape.has_value(), "shape line: expected parsed shape.")) {
        return false;
    }

    if (!expect(shape->type == SvgElementType::Line, "shape line: unexpected shape type.")) {
        return false;
    }

    if (!expect(std::holds_alternative<LineElement>(shape->data), "shape line: unexpected shape data type.")) {
        return false;
    }

    const auto& line = std::get<LineElement>(shape->data);
    if (!expect(line.x1 == 1.0, "shape line: unexpected x1.")) {
        return false;
    }
    if (!expect(line.y1 == 2.0, "shape line: unexpected y1.")) {
        return false;
    }
    if (!expect(line.x2 == 3.0, "shape line: unexpected x2.")) {
        return false;
    }

    return expect(line.y2 == 4.0, "shape line: unexpected y2.");
}

bool test_interpret_ellipse_success() {
    const auto node = create_node("ellipse", {{"cx", "3"}, {"cy", "4"}, {"rx", "5"}, {"ry", "6"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    if (!expect(shape.has_value(), "shape ellipse: expected parsed shape.")) {
        return false;
    }

    if (!expect(shape->type == SvgElementType::Ellipse, "shape ellipse: unexpected shape type.")) {
        return false;
    }

    if (!expect(std::holds_alternative<EllipseElement>(shape->data),
                "shape ellipse: unexpected shape data type.")) {
        return false;
    }

    const auto& ellipse = std::get<EllipseElement>(shape->data);
    if (!expect(ellipse.cx == 3.0, "shape ellipse: unexpected cx.")) {
        return false;
    }
    if (!expect(ellipse.cy == 4.0, "shape ellipse: unexpected cy.")) {
        return false;
    }
    if (!expect(ellipse.rx == 5.0, "shape ellipse: unexpected rx.")) {
        return false;
    }

    return expect(ellipse.ry == 6.0, "shape ellipse: unexpected ry.");
}

bool test_interpret_polyline_points_split_by_space_success() {
    const auto node = create_node("polyline", {{"points", "1 2 3 4 5 6"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    if (!expect(shape.has_value(), "shape polyline: expected parsed shape.")) {
        return false;
    }

    if (!expect(shape->type == SvgElementType::Polyline, "shape polyline: unexpected shape type.")) {
        return false;
    }

    if (!expect(std::holds_alternative<PolylineElement>(shape->data),
                "shape polyline: unexpected shape data type.")) {
        return false;
    }

    const auto& points = std::get<PolylineElement>(shape->data).points;
    if (!expect(points.size() == 3, "shape polyline: expected three points.")) {
        return false;
    }
    if (!expect_point(points[0], 1.0, 2.0, "shape polyline first point")) {
        return false;
    }
    if (!expect_point(points[1], 3.0, 4.0, "shape polyline second point")) {
        return false;
    }

    return expect_point(points[2], 5.0, 6.0, "shape polyline third point");
}

bool test_interpret_polygon_points_with_comma_success() {
    const auto node = create_node("polygon", {{"points", "7,8 9,10 11,12"}});

    const std::optional<SvgShape> shape = SvgShapeInterpreter::interpret(node);

    if (!expect(shape.has_value(), "shape polygon: expected parsed shape.")) {
        return false;
    }

    if (!expect(shape->type == SvgElementType::Polygon, "shape polygon: unexpected shape type.")) {
        return false;
    }

    if (!expect(std::holds_alternative<PolygonElement>(shape->data),
                "shape polygon: unexpected shape data type.")) {
        return false;
    }

    const auto& points = std::get<PolygonElement>(shape->data).points;
    if (!expect(points.size() == 3, "shape polygon: expected three points.")) {
        return false;
    }
    if (!expect_point(points[0], 7.0, 8.0, "shape polygon first point")) {
        return false;
    }
    if (!expect_point(points[1], 9.0, 10.0, "shape polygon second point")) {
        return false;
    }

    return expect_point(points[2], 11.0, 12.0, "shape polygon third point");
}

}  // namespace

int main() {
    const std::vector<std::pair<const char*, bool (*)()>> test_cases = {
        {"element interpret minimal svg root", test_interpret_svg_root_minimal_success},
        {"element reflect root attributes", test_interpret_svg_root_attributes_reflected},
        {"element collect child shapes", test_interpret_svg_root_collects_child_shapes},
        {"element root must be svg", test_interpret_svg_root_not_svg_returns_nullopt},
        {"element invalid viewBox", test_interpret_svg_root_invalid_view_box_returns_nullopt},
        {"element missing required root attribute", test_interpret_svg_root_missing_required_attribute_returns_nullopt},
        {"element unsupported stroke-linecap", test_interpret_svg_root_invalid_stroke_linecap_returns_nullopt},
        {"element unsupported stroke-linejoin", test_interpret_svg_root_invalid_stroke_linejoin_returns_nullopt},
        {"shape unsupported element", test_interpret_unsupported_shape_returns_nullopt},
        {"shape path missing d", test_interpret_path_missing_d_returns_nullopt},
        {"shape interpret path", test_interpret_path_success},
        {"shape circle missing required attribute", test_interpret_circle_missing_required_attribute_returns_nullopt},
        {"shape circle invalid number", test_interpret_circle_invalid_number_returns_nullopt},
        {"shape interpret circle", test_interpret_circle_success},
        {"shape rect missing required attribute", test_interpret_rect_missing_required_attribute_returns_nullopt},
        {"shape rect invalid rx", test_interpret_rect_invalid_rx_returns_nullopt},
        {"shape rect invalid ry", test_interpret_rect_invalid_ry_returns_nullopt},
        {"shape interpret rect without rx ry", test_interpret_rect_without_rx_ry_success},
        {"shape interpret rect with rx ry", test_interpret_rect_with_rx_ry_success},
        {"shape line missing required attribute", test_interpret_line_missing_required_attribute_returns_nullopt},
        {"shape line invalid number", test_interpret_line_invalid_number_returns_nullopt},
        {"shape interpret line", test_interpret_line_success},
        {"shape ellipse missing required attribute", test_interpret_ellipse_missing_required_attribute_returns_nullopt},
        {"shape ellipse invalid number", test_interpret_ellipse_invalid_number_returns_nullopt},
        {"shape interpret ellipse", test_interpret_ellipse_success},
        {"shape polyline missing points", test_interpret_polyline_missing_points_returns_nullopt},
        {"shape polyline odd points count", test_interpret_polyline_odd_points_count_returns_nullopt},
        {"shape polyline invalid points token", test_interpret_polyline_invalid_points_token_returns_nullopt},
        {"shape interpret polyline with space separated points", test_interpret_polyline_points_split_by_space_success},
        {"shape polygon missing points", test_interpret_polygon_missing_points_returns_nullopt},
        {"shape polygon odd points count", test_interpret_polygon_odd_points_count_returns_nullopt},
        {"shape polygon invalid points token", test_interpret_polygon_invalid_points_token_returns_nullopt},
        {"shape interpret polygon with comma separated points", test_interpret_polygon_points_with_comma_success},
    };

    for (const auto& test_case : test_cases) {
        if (!test_case.second()) {
            std::cerr << "Failed test: " << test_case.first << std::endl;
            return 1;
        }
    }

    return 0;
}
