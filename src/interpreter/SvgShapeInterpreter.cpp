#include "SvgShapeInterpreter.h"

#include "SvgInterpreterUtility.h"
#include "parser/ExtractedNode.h"

namespace svg{
namespace interpreter{

using svg::parser::ExtractedNode;

std::optional<SvgShape> SvgShapeInterpreter::interpret(const parser::ExtractedNode& node) {
    const auto element_type = interpret_element_type(node.element_name);
    if (!element_type.has_value()) {
        return std::nullopt;
    }

    switch (*element_type) {
        case SvgElementType::Path:
            return interpret_path(node);
        case SvgElementType::Circle:
            return interpret_circle(node);
        case SvgElementType::Rect:
            return interpret_rect(node);
        case SvgElementType::Line:
            return interpret_line(node);
        case SvgElementType::Ellipse:
            return interpret_ellipse(node);
        case SvgElementType::Polyline:
            return interpret_polyline(node);
        case SvgElementType::Polygon:
            return interpret_polygon(node);
        default:
            return std::nullopt;
    }
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_path(const ExtractedNode& node) {
    const auto d = SvgInterpreterUtility::find_attribute_value(node, "d");
    if (!d.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Path;
    shape.data = PathElement{*d};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_circle(const ExtractedNode& node) {
    const auto cx = SvgInterpreterUtility::require_parsed_double_attribute(node, "cx");
    if (!cx.has_value()) {
        return std::nullopt;
    }
    const auto cy = SvgInterpreterUtility::require_parsed_double_attribute(node, "cy");
    if (!cy.has_value()) {
        return std::nullopt;
    }
    const auto r = SvgInterpreterUtility::require_parsed_double_attribute(node, "r");
    if (!r.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Circle;
    shape.data = CircleElement{*cx, *cy, *r};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_rect(const ExtractedNode& node) {
    const auto x = SvgInterpreterUtility::require_parsed_double_attribute(node, "x");
    if (!x.has_value()) {
        return std::nullopt;
    }
    const auto y = SvgInterpreterUtility::require_parsed_double_attribute(node, "y");
    if (!y.has_value()) {
        return std::nullopt;
    }
    const auto width = SvgInterpreterUtility::require_parsed_double_attribute(node, "width");
    if (!width.has_value()) {
        return std::nullopt;
    }
    const auto height = SvgInterpreterUtility::require_parsed_double_attribute(node, "height");
    if (!height.has_value()) {
        return std::nullopt;
    }

    const auto rx = SvgInterpreterUtility::parse_optional_double_attribute(node, "rx");
    if (!rx.has_value()) {
        return std::nullopt;
    }

    const auto ry = SvgInterpreterUtility::parse_optional_double_attribute(node, "ry");
    if (!ry.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Rect;
    shape.data = RectElement{*x, *y, *width, *height, *rx, *ry};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_line(const ExtractedNode& node) {
    const auto x1 = SvgInterpreterUtility::require_parsed_double_attribute(node, "x1");
    if (!x1.has_value()) {
        return std::nullopt;
    }
    const auto y1 = SvgInterpreterUtility::require_parsed_double_attribute(node, "y1");
    if (!y1.has_value()) {
        return std::nullopt;
    }
    const auto x2 = SvgInterpreterUtility::require_parsed_double_attribute(node, "x2");
    if (!x2.has_value()) {
        return std::nullopt;
    }
    const auto y2 = SvgInterpreterUtility::require_parsed_double_attribute(node, "y2");
    if (!y2.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Line;
    shape.data = LineElement{*x1, *y1, *x2, *y2};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_ellipse(const ExtractedNode& node) {
    const auto cx = SvgInterpreterUtility::require_parsed_double_attribute(node, "cx");
    if (!cx.has_value()) {
        return std::nullopt;
    }
    const auto cy = SvgInterpreterUtility::require_parsed_double_attribute(node, "cy");
    if (!cy.has_value()) {
        return std::nullopt;
    }
    const auto rx = SvgInterpreterUtility::require_parsed_double_attribute(node, "rx");
    if (!rx.has_value()) {
        return std::nullopt;
    }
    const auto ry = SvgInterpreterUtility::require_parsed_double_attribute(node, "ry");
    if (!ry.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Ellipse;
    shape.data = EllipseElement{*cx, *cy, *rx, *ry};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_polyline(const ExtractedNode& node) {
    const auto points_value = SvgInterpreterUtility::find_attribute_value(node, "points");
    if (!points_value.has_value()) {
        return std::nullopt;
    }

    const auto points = parse_points(*points_value);
    if (!points.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Polyline;
    shape.data = PolylineElement{*points};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_polygon(const ExtractedNode& node) {
    const auto points_value = SvgInterpreterUtility::find_attribute_value(node, "points");
    if (!points_value.has_value()) {
        return std::nullopt;
    }

    const auto points = parse_points(*points_value);
    if (!points.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Polygon;
    shape.data = PolygonElement{*points};
    return shape;
}

std::optional<std::vector<Point>> SvgShapeInterpreter::parse_points(std::string_view value) {
    const auto tokens = SvgInterpreterUtility::split_by_delimiter(value, ' ');
    std::vector<double> parsed_values;

    for (const auto token : tokens) {
        const auto comma_separated_values = SvgInterpreterUtility::split_by_delimiter(token, ',');
        for (const auto number_text : comma_separated_values) {
            const auto parsed_number = SvgInterpreterUtility::parse_double(number_text);
            if (!parsed_number.has_value()) {
                return std::nullopt;
            }

            parsed_values.push_back(*parsed_number);
        }
    }

    if (parsed_values.size() % 2 != 0) {
        return std::nullopt;
    }

    std::vector<Point> points;
    for (std::size_t index = 0; index < parsed_values.size(); index += 2) {
        points.push_back(Point{parsed_values[index], parsed_values[index + 1]});
    }

    return points;
}

std::optional<SvgElementType> SvgShapeInterpreter::interpret_element_type(
    std::string_view element_name) {
    if (element_name == "path") {
        return SvgElementType::Path;
    }
    if (element_name == "circle") {
        return SvgElementType::Circle;
    }
    if (element_name == "rect") {
        return SvgElementType::Rect;
    }
    if (element_name == "line") {
        return SvgElementType::Line;
    }
    if (element_name == "ellipse") {
        return SvgElementType::Ellipse;
    }
    if (element_name == "polyline") {
        return SvgElementType::Polyline;
    }
    if (element_name == "polygon") {
        return SvgElementType::Polygon;
    }

    return std::nullopt;
}

}
}
