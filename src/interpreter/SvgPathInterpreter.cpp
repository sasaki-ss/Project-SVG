#include "SvgPathInterpreter.h"

namespace svg{
namespace interpreter{

using parser::PathCommand;
using parser::PathCommandType;

auto SvgPathInterpreter::interpret(
    const std::vector<PathCommand>& commands)
    -> std::optional<std::vector<PathInstruction>> {
    if (commands.empty()) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (const auto& command : commands) {
        std::optional<std::vector<PathInstruction>> interpreted_command;
        switch (command.type) {
            case PathCommandType::MoveTo:
                interpreted_command = interpret_move_to(command);
                break;
            case PathCommandType::LineTo:
                interpreted_command = interpret_line_to(command);
                break;
            case PathCommandType::HorizontalTo:
                interpreted_command = interpret_horizontal_to(command);
                break;
            case PathCommandType::VerticalTo:
                interpreted_command = interpret_vertical_to(command);
                break;
            case PathCommandType::CubicBezierTo:
                interpreted_command = interpret_cubic_bezier_to(command);
                break;
            case PathCommandType::ClosePath:
                interpreted_command = interpret_close_path(command);
                break;
            case PathCommandType::SmoothCubicBezierTo:
                interpreted_command = interpret_smooth_cubic_bezier_to(command);
                break;
            case PathCommandType::QuadraticBezierTo:
                interpreted_command = interpret_quadratic_bezier_to(command);
                break;
            case PathCommandType::SmoothQuadraticBezierTo:
                interpreted_command = interpret_smooth_quadratic_bezier_to(command);
                break;
            case PathCommandType::ArcTo:
                interpreted_command = interpret_arc_to(command);
                break;
            default:
                return std::nullopt;
        }

        if (!interpreted_command.has_value()) {
            return std::nullopt;
        }

        instructions.insert(
            instructions.end(), interpreted_command->begin(), interpreted_command->end());
    }

    return instructions;
}

auto SvgPathInterpreter::interpret_move_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_line_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_horizontal_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_vertical_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_cubic_bezier_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_close_path(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_smooth_cubic_bezier_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_quadratic_bezier_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_smooth_quadratic_bezier_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

auto SvgPathInterpreter::interpret_arc_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    (void)command;
    return std::nullopt;
}

}
}
