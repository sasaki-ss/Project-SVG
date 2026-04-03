#ifndef PROJECT_SVG_INTERPRETER_SVG_INTERPRETER_UTILITY_H_
#define PROJECT_SVG_INTERPRETER_SVG_INTERPRETER_UTILITY_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace svg {
namespace parser {
struct ExtractedNode;
}
}

namespace svg{
namespace interpreter{

class SvgInterpreterUtility {
public:
    static std::optional<std::string> find_attribute_value(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::optional<double> parse_double(std::string_view value);

    static std::optional<double> require_parsed_double_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::optional<std::optional<double>> parse_optional_double_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::vector<std::string_view> split_by_delimiter(
        std::string_view value,
        char delimiter);

private:
    SvgInterpreterUtility() = delete;
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_INTERPRETER_UTILITY_H_
