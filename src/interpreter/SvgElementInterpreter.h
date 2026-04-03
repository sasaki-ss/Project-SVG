#ifndef PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_
#define PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_

#include <optional>
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

private:
    SvgElementInterpreter() = delete;

    static std::optional<SvgViewBox> require_parsed_view_box_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::optional<StrokeLineCap> require_parsed_stroke_linecap_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::optional<StrokeLineJoin> require_parsed_stroke_linejoin_attribute(
        const parser::ExtractedNode& node,
        std::string_view attribute_name);

    static std::vector<SvgShape> traverse_child_nodes(
        const parser::ExtractedNode& node);

    static std::optional<SvgViewBox> parse_view_box(std::string_view value);
    static std::optional<StrokeLineCap> parse_stroke_linecap(std::string_view value);
    static std::optional<StrokeLineJoin> parse_stroke_linejoin(std::string_view value);
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_
