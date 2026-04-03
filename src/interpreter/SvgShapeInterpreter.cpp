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

    return std::nullopt;
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
