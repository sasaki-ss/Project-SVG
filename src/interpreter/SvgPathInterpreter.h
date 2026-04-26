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

    auto interpret(
        const std::vector<parser::PathCommand>& commands)
        -> std::optional<std::vector<PathInstruction>>;

private:
    auto interpret_move_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_line_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_horizontal_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_vertical_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_cubic_bezier_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_close_path(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_smooth_cubic_bezier_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_quadratic_bezier_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_smooth_quadratic_bezier_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_arc_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_
