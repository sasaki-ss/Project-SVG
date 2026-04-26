#ifndef PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_
#define PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_

#include <cstddef>
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
    struct InterpretContext {
        Point current_point{0.0, 0.0};
        Point subpath_start_point{0.0, 0.0};
        bool has_current_point{false};
        bool has_subpath_start_point{false};
    };

    auto interpret_move_to(
        const parser::PathCommand& command)
        -> std::optional<std::vector<PathInstruction>>;
    auto interpret_line_to(
        const parser::PathCommand& command,
        std::size_t start_index = 0)
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

    auto parse_point(
        const parser::PathCommand& command,
        std::size_t index)
        -> std::optional<Point>;
    auto make_absolute_point(
        const Point& point,
        bool is_absolute)
        -> Point;

    InterpretContext context;
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_
