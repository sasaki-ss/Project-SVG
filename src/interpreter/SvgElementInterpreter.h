#ifndef PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_
#define PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_

#include <optional>
#include <string>
#include <string_view>

#include "InterpretedSvg.h"
#include "../parser/ExtractedNode.h"

namespace svg{
namespace interpreter{

class SvgElementInterpreter {
public:
    static std::optional<InterpretedSvg> interpret(const parser::ExtractedNode& root);
    static std::optional<SvgElementType> interpret_element_type(std::string_view element_name);

private:
    SvgElementInterpreter() = delete;

    static std::optional<std::string> find_attribute_value(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);
    static std::optional<double> parse_double(std::string_view value);
    static std::optional<SvgViewBox> parse_view_box(std::string_view value);
    static std::optional<StrokeLineCap> parse_stroke_linecap(std::string_view value);
    static std::optional<StrokeLineJoin> parse_stroke_linejoin(std::string_view value);

    static void traverse_child_nodes(const parser::ExtractedNode& node);
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_
