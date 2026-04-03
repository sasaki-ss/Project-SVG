#include "SvgShapeInterpreter.h"

#include <charconv>

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
        default:
            return std::nullopt;
    }
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_path(const ExtractedNode& node) {
    const auto d = find_attribute_value(node, "d");
    if (!d.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Path;
    shape.data = PathElement{*d};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_circle(const ExtractedNode& node) {
    const auto cx = require_parsed_double_attribute(node, "cx");
    if (!cx.has_value()) {
        return std::nullopt;
    }
    const auto cy = require_parsed_double_attribute(node, "cy");
    if (!cy.has_value()) {
        return std::nullopt;
    }
    const auto r = require_parsed_double_attribute(node, "r");
    if (!r.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Circle;
    shape.data = CircleElement{*cx, *cy, *r};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_rect(const ExtractedNode& node) {
    const auto x = require_parsed_double_attribute(node, "x");
    if (!x.has_value()) {
        return std::nullopt;
    }
    const auto y = require_parsed_double_attribute(node, "y");
    if (!y.has_value()) {
        return std::nullopt;
    }
    const auto width = require_parsed_double_attribute(node, "width");
    if (!width.has_value()) {
        return std::nullopt;
    }
    const auto height = require_parsed_double_attribute(node, "height");
    if (!height.has_value()) {
        return std::nullopt;
    }

    const auto rx = parse_optional_double_attribute(node, "rx");
    const auto rx_value = find_attribute_value(node, "rx");
    if (rx_value.has_value() && !rx.has_value()) {
        return std::nullopt;
    }

    const auto ry = parse_optional_double_attribute(node, "ry");
    const auto ry_value = find_attribute_value(node, "ry");
    if (ry_value.has_value() && !ry.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Rect;
    shape.data = RectElement{*x, *y, *width, *height, rx, ry};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_line(const ExtractedNode& node) {
    const auto x1 = require_parsed_double_attribute(node, "x1");
    if (!x1.has_value()) {
        return std::nullopt;
    }
    const auto y1 = require_parsed_double_attribute(node, "y1");
    if (!y1.has_value()) {
        return std::nullopt;
    }
    const auto x2 = require_parsed_double_attribute(node, "x2");
    if (!x2.has_value()) {
        return std::nullopt;
    }
    const auto y2 = require_parsed_double_attribute(node, "y2");
    if (!y2.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Line;
    shape.data = LineElement{*x1, *y1, *x2, *y2};
    return shape;
}

std::optional<SvgShape> SvgShapeInterpreter::interpret_ellipse(const ExtractedNode& node) {
    const auto cx = require_parsed_double_attribute(node, "cx");
    if (!cx.has_value()) {
        return std::nullopt;
    }
    const auto cy = require_parsed_double_attribute(node, "cy");
    if (!cy.has_value()) {
        return std::nullopt;
    }
    const auto rx = require_parsed_double_attribute(node, "rx");
    if (!rx.has_value()) {
        return std::nullopt;
    }
    const auto ry = require_parsed_double_attribute(node, "ry");
    if (!ry.has_value()) {
        return std::nullopt;
    }

    SvgShape shape;
    shape.type = SvgElementType::Ellipse;
    shape.data = EllipseElement{*cx, *cy, *rx, *ry};
    return shape;
}

std::optional<std::string> SvgShapeInterpreter::find_attribute_value(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    for (const auto& attribute : node.attributes) {
        if (attribute.name == attribute_name) {
            return attribute.value;
        }
    }
    return std::nullopt;
}

std::optional<double> SvgShapeInterpreter::parse_double(std::string_view value) {
    double parsed_value = 0.0;
    const auto* begin = value.data();
    const auto* end = begin + value.size();
    const auto result = std::from_chars(begin, end, parsed_value);
    if (result.ec != std::errc() || result.ptr != end) {
        return std::nullopt;
    }
    return parsed_value;
}

std::optional<double> SvgShapeInterpreter::require_parsed_double_attribute(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    const auto value = find_attribute_value(node, attribute_name);
    if (!value.has_value()) {
        return std::nullopt;
    }
    return parse_double(*value);
}

std::optional<double> SvgShapeInterpreter::parse_optional_double_attribute(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    const auto value = find_attribute_value(node, attribute_name);
    if (!value.has_value()) {
        return std::nullopt;
    }
    return parse_double(*value);
}

std::vector<std::string_view> SvgShapeInterpreter::split_by_space(std::string_view value) {
    std::vector<std::string_view> tokens;
    std::size_t position = 0;
    const auto value_size = value.size();

    while (position < value_size) {
        while (position < value_size && value[position] == ' ') {
            ++position;
        }

        if (position >= value_size) {
            break;
        }

        const auto next_delimiter = value.find(' ', position);
        if (next_delimiter == std::string_view::npos) {
            tokens.push_back(value.substr(position));
            break;
        }

        tokens.push_back(value.substr(position, next_delimiter - position));
        position = next_delimiter + 1;
    }

    return tokens;
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
