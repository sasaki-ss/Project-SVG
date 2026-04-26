#ifndef PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_
#define PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_

#include <optional>
#include <vector>

#include "PathInstruction.h"
#include "parser/PathCommand.h"

namespace svg{
namespace interpreter{

class SvgPathInterpreter {
public:
    SvgPathInterpreter() = default;

    std::optional<std::vector<PathInstruction>> interpret(
        const std::vector<parser::PathCommand>& commands);

private:
    std::optional<std::vector<PathInstruction>> interpret_move_to(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_line_to(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_horizontal_to(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_vertical_to(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_cubic_bezier_to(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_close_path(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_smooth_cubic_bezier_to(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_quadratic_bezier_to(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_smooth_quadratic_bezier_to(
        const parser::PathCommand& command);
    std::optional<std::vector<PathInstruction>> interpret_arc_to(
        const parser::PathCommand& command);
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_
