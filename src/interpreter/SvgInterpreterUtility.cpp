#include "SvgInterpreterUtility.h"

#include <charconv>

#include "parser/ExtractedNode.h"

namespace svg{
namespace interpreter{

using svg::parser::ExtractedNode;

std::optional<std::string> SvgInterpreterUtility::find_attribute_value(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    for (const auto& attribute : node.attributes) {
        if (attribute.name == attribute_name) {
            return attribute.value;
        }
    }
    return std::nullopt;
}

std::optional<double> SvgInterpreterUtility::parse_double(std::string_view value) {
    double parsed_value = 0.0;
    const auto* begin = value.data();
    const auto* end = begin + value.size();
    const auto result = std::from_chars(begin, end, parsed_value);
    if (result.ec != std::errc() || result.ptr != end) {
        return std::nullopt;
    }
    return parsed_value;
}

std::optional<double> SvgInterpreterUtility::require_parsed_double_attribute(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    const auto value = find_attribute_value(node, attribute_name);
    if (!value.has_value()) {
        return std::nullopt;
    }
    return parse_double(*value);
}

std::optional<std::optional<double>> SvgInterpreterUtility::parse_optional_double_attribute(
    const ExtractedNode& node,
    std::string_view attribute_name) {
    const auto value = find_attribute_value(node, attribute_name);
    if (!value.has_value()) {
        return std::optional<double>{std::nullopt};
    }

    const auto parsed_value = parse_double(*value);
    if (!parsed_value.has_value()) {
        return std::nullopt;
    }

    return std::optional<double>{*parsed_value};
}

std::vector<std::string_view> SvgInterpreterUtility::split_by_delimiter(
    std::string_view value,
    char delimiter) {
    std::vector<std::string_view> tokens;
    std::size_t position = 0;
    const auto value_size = value.size();

    while (position < value_size) {
        while (position < value_size && value[position] == delimiter) {
            ++position;
        }

        if (position >= value_size) {
            break;
        }

        const auto next_delimiter = value.find(delimiter, position);
        if (next_delimiter == std::string_view::npos) {
            tokens.push_back(value.substr(position));
            break;
        }

        tokens.push_back(value.substr(position, next_delimiter - position));
        position = next_delimiter + 1;
    }

    return tokens;
}

}
}
