#ifndef PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_
#define PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_

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

class SvgElementInterpreter {
public:
    static std::optional<InterpretedSvg> interpret(const parser::ExtractedNode& root);
    static std::optional<SvgElementType> interpret_element_type(std::string_view element_name);

private:
    SvgElementInterpreter() = delete;

    static std::optional<double> require_parsed_double_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);
    static std::optional<SvgViewBox> require_parsed_view_box_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);
    static std::optional<StrokeLineCap> require_parsed_stroke_linecap_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);
    static std::optional<StrokeLineJoin> require_parsed_stroke_linejoin_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::optional<std::string> find_attribute_value(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);
    static std::vector<std::string_view> split_by_space(std::string_view value);
    static std::optional<double> parse_double(std::string_view value);
    static std::optional<SvgViewBox> parse_view_box(std::string_view value);
    static std::optional<StrokeLineCap> parse_stroke_linecap(std::string_view value);
    static std::optional<StrokeLineJoin> parse_stroke_linejoin(std::string_view value);

    static void traverse_child_nodes(const parser::ExtractedNode& node);
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_
