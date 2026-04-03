#include "SvgElementInterpreter.h"

#include <array>
#include <charconv>

namespace svg{
namespace interpreter{

std::optional<InterpretedSvg> SvgElementInterpreter::interpret(const parser::ExtractedNode& root) {
    if (root.element_name != "svg") {
        return std::nullopt;
    }

    const auto view_box_value = find_attribute_value(root, "viewBox");
    const auto width_value = find_attribute_value(root, "width");
    const auto height_value = find_attribute_value(root, "height");
    const auto stroke_value = find_attribute_value(root, "stroke");
    const auto fill_value = find_attribute_value(root, "fill");
    const auto stroke_width_value = find_attribute_value(root, "stroke-width");
    const auto stroke_linecap_value = find_attribute_value(root, "stroke-linecap");
    const auto stroke_linejoin_value = find_attribute_value(root, "stroke-linejoin");

    if (!view_box_value.has_value() ||
        !width_value.has_value() ||
        !height_value.has_value() ||
        !stroke_value.has_value() ||
        !fill_value.has_value() ||
        !stroke_width_value.has_value() ||
        !stroke_linecap_value.has_value() ||
        !stroke_linejoin_value.has_value()) {
        return std::nullopt;
    }

    const auto parsed_view_box = parse_view_box(*view_box_value);
    const auto parsed_width = parse_double(*width_value);
    const auto parsed_height = parse_double(*height_value);
    const auto parsed_stroke_width = parse_double(*stroke_width_value);
    const auto parsed_stroke_linecap = parse_stroke_linecap(*stroke_linecap_value);
    const auto parsed_stroke_linejoin = parse_stroke_linejoin(*stroke_linejoin_value);

    if (!parsed_view_box.has_value() ||
        !parsed_width.has_value() ||
        !parsed_height.has_value() ||
        !parsed_stroke_width.has_value() ||
        !parsed_stroke_linecap.has_value() ||
        !parsed_stroke_linejoin.has_value()) {
        return std::nullopt;
    }

    for (const auto& child_node : root.children) {
        traverse_child_nodes(child_node);
    }

    InterpretedSvg interpreted_svg{
        .view_box = *parsed_view_box,
        .width = *parsed_width,
        .height = *parsed_height,
        .root_style = SvgRootStyle{
            .stroke = *stroke_value,
            .fill = *fill_value,
            .stroke_width = *parsed_stroke_width,
            .stroke_linecap = *parsed_stroke_linecap,
            .stroke_linejoin = *parsed_stroke_linejoin,
        },
        .shapes = {},
    };
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
    const auto element_type = interpret_element_type(node.element_name);
    (void)element_type;

    for (const auto& child_node : node.children) {
        traverse_child_nodes(child_node);
    }
}

std::optional<std::string> SvgElementInterpreter::find_attribute_value(
    const parser::ExtractedNode& node,
    std::string_view attribute_name) {
    for (const auto& attribute : node.attributes) {
        if (attribute.name == attribute_name) {
            return attribute.value;
        }
    }
    return std::nullopt;
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
    std::array<double, 4> parsed_values = {0.0, 0.0, 0.0, 0.0};
    std::size_t index = 0;
    std::size_t position = 0;

    while (position < value.size()) {
        while (position < value.size() && value[position] == ' ') {
            ++position;
        }

        if (position >= value.size()) {
            break;
        }

        const auto next_delimiter = value.find(' ', position);
        const std::string_view token = next_delimiter == std::string_view::npos
            ? value.substr(position)
            : value.substr(position, next_delimiter - position);

        if (index >= parsed_values.size()) {
            return std::nullopt;
        }

        const auto parsed_token = parse_double(token);
        if (!parsed_token.has_value()) {
            return std::nullopt;
        }
        parsed_values[index] = *parsed_token;
        ++index;

        if (next_delimiter == std::string_view::npos) {
            position = value.size();
        } else {
            position = next_delimiter + 1;
        }
    }

    if (index != parsed_values.size()) {
        return std::nullopt;
    }

    return SvgViewBox{
        .min_x = parsed_values[0],
        .min_y = parsed_values[1],
        .width = parsed_values[2],
        .height = parsed_values[3],
    };
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
