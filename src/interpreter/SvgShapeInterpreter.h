#ifndef PROJECT_SVG_INTERPRETER_SVG_SHAPE_INTERPRETER_H_
#define PROJECT_SVG_INTERPRETER_SVG_SHAPE_INTERPRETER_H_

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "InterpretedSvg.h"

namespace svg {
namespace parser {
struct ExtractedNode;
}
}

namespace svg{
namespace interpreter{

class SvgShapeInterpreter {
public:
    static std::optional<SvgShape> interpret(const parser::ExtractedNode& node);

    static std::optional<std::string> find_attribute_value(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::optional<double> parse_double(std::string_view value);

    static std::optional<double> require_parsed_double_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::vector<std::string_view> split_by_space(std::string_view value);

private:
    SvgShapeInterpreter() = delete;

    static std::optional<SvgElementType> interpret_element_type(
        std::string_view element_name);

    static std::optional<SvgShape> interpret_path(const parser::ExtractedNode& node);

    static std::optional<SvgShape> interpret_circle(const parser::ExtractedNode& node);

    static std::optional<SvgShape> interpret_rect(const parser::ExtractedNode& node);

    static std::optional<SvgShape> interpret_line(const parser::ExtractedNode& node);

    static std::optional<SvgShape> interpret_ellipse(const parser::ExtractedNode& node);
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_SHAPE_INTERPRETER_H_
