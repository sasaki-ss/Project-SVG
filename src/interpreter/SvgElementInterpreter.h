#ifndef PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_
#define PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_

#include <optional>
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

    static void traverse_child_nodes(const parser::ExtractedNode& node);
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_ELEMENT_INTERPRETER_H_
