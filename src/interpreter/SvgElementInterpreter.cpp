#include "SvgElementInterpreter.h"

#include <array>
#include <charconv>

#include "../parser/ExtractedNode.h"

namespace svg{
namespace interpreter{

using svg::parser::ExtractedNode;

std::optional<InterpretedSvg> SvgElementInterpreter::interpret(const parser::ExtractedNode& root) {
    if (root.element_name != "svg") {
        return std::nullopt;
    }

    const auto parsed_view_box = require_parsed_view_box_attribute(root, "viewBox");
    const auto parsed_width = require_parsed_double_attribute(root, "width");
    const auto parsed_height = require_parsed_double_attribute(root, "height");
    const auto stroke_value = find_attribute_value(root, "stroke");
    const auto fill_value = find_attribute_value(root, "fill");
    const auto parsed_stroke_width = require_parsed_double_attribute(root, "stroke-width");
    const auto parsed_stroke_linecap = require_parsed_stroke_linecap_attribute(root, "stroke-linecap");
    const auto parsed_stroke_linejoin = require_parsed_stroke_linejoin_attribute(root, "stroke-linejoin");

    if (!parsed_view_box.has_value() ||
        !parsed_width.has_value() ||
        !parsed_height.has_value() ||
        !stroke_value.has_value() ||
        !fill_value.has_value() ||
        !parsed_stroke_width.has_value() ||
        !parsed_stroke_linecap.has_value() ||
        !parsed_stroke_linejoin.has_value()) {
        return std::nullopt;
    }

    for (const auto& child_node : root.children) {
        traverse_child_nodes(child_node);
    }

    InterpretedSvg interpreted_svg;
    interpreted_svg.view_box = *parsed_view_box;
    interpreted_svg.width = *parsed_width;
    interpreted_svg.height = *parsed_height;
    interpreted_svg.root_style.stroke = *stroke_value;
    interpreted_svg.root_style.fill = *fill_value;
    interpreted_svg.root_style.stroke_width = *parsed_stroke_width;
    interpreted_svg.root_style.stroke_linecap = *parsed_stroke_linecap;
    interpreted_svg.root_style.stroke_linejoin = *parsed_stroke_linejoin;
    return interpreted_svg;
}

std::optional<SvgElementType> SvgElementInterpreter::interpret_element_type(std::string_view element_name) {
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

void SvgElementInterpreter::traverse_child_nodes(const parser::ExtractedNode& node) {
    for (const auto& child_node : node.children) {
        traverse_child_nodes(child_node);
    }
}

std::optional<double> SvgElementInterpreter::require_parsed_double_attribute(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    const auto value = find_attribute_value(node, attribute_name);
    if (!value.has_value()) {
        return std::nullopt;
    }
    return parse_double(*value);
}

std::optional<SvgViewBox> SvgElementInterpreter::require_parsed_view_box_attribute(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    const auto value = find_attribute_value(node, attribute_name);
    if (!value.has_value()) {
        return std::nullopt;
    }
    return parse_view_box(*value);
}

std::optional<StrokeLineCap> SvgElementInterpreter::require_parsed_stroke_linecap_attribute(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    const auto value = find_attribute_value(node, attribute_name);
    if (!value.has_value()) {
        return std::nullopt;
    }
    return parse_stroke_linecap(*value);
}

std::optional<StrokeLineJoin> SvgElementInterpreter::require_parsed_stroke_linejoin_attribute(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    const auto value = find_attribute_value(node, attribute_name);
    if (!value.has_value()) {
        return std::nullopt;
    }
    return parse_stroke_linejoin(*value);
}

std::optional<std::string> SvgElementInterpreter::find_attribute_value(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    for (const auto& attribute : node.attributes) {
        if (attribute.name == attribute_name) {
            return attribute.value;
        }
    }
    return std::nullopt;
}

std::vector<std::string_view> SvgElementInterpreter::split_by_space(std::string_view value) {
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

std::optional<double> SvgElementInterpreter::parse_double(std::string_view value) {
    double parsed_value = 0.0;
    const auto* begin = value.data();
    const auto* end = begin + value.size();
    const auto result = std::from_chars(begin, end, parsed_value);
    if (result.ec != std::errc() || result.ptr != end) {
        return std::nullopt;
    }
    return parsed_value;
}

std::optional<SvgViewBox> SvgElementInterpreter::parse_view_box(std::string_view value) {
    const auto tokens = split_by_space(value);
    if (tokens.size() != 4) {
        return std::nullopt;
    }

    std::array<double, 4> parsed_values = {0.0, 0.0, 0.0, 0.0};
    for (std::size_t index = 0; index < tokens.size(); ++index) {
        const auto parsed_token = parse_double(tokens[index]);
        if (!parsed_token.has_value()) {
            return std::nullopt;
        }
        parsed_values[index] = *parsed_token;
    }

    SvgViewBox view_box;
    view_box.min_x = parsed_values[0];
    view_box.min_y = parsed_values[1];
    view_box.width = parsed_values[2];
    view_box.height = parsed_values[3];
    return view_box;
}

std::optional<StrokeLineCap> SvgElementInterpreter::parse_stroke_linecap(std::string_view value) {
    if (value == "round") {
        return StrokeLineCap::Round;
    }
    return std::nullopt;
}

std::optional<StrokeLineJoin> SvgElementInterpreter::parse_stroke_linejoin(std::string_view value) {
    if (value == "round") {
        return StrokeLineJoin::Round;
    }
    return std::nullopt;
}

}
}
