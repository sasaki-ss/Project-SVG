#include "SvgPathInterpreter.h"

namespace svg{
namespace interpreter{

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret(
    const std::vector<parser::PathCommand>& commands) {
    if (commands.empty()) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (const auto& command : commands) {
        std::optional<std::vector<PathInstruction>> interpreted_command;
        switch (command.type) {
            case parser::PathCommandType::MoveTo:
                interpreted_command = interpret_move_to(command);
                break;
            case parser::PathCommandType::LineTo:
                interpreted_command = interpret_line_to(command);
                break;
            case parser::PathCommandType::HorizontalTo:
                interpreted_command = interpret_horizontal_to(command);
                break;
            case parser::PathCommandType::VerticalTo:
                interpreted_command = interpret_vertical_to(command);
                break;
            case parser::PathCommandType::CubicBezierTo:
                interpreted_command = interpret_cubic_bezier_to(command);
                break;
            case parser::PathCommandType::ClosePath:
                interpreted_command = interpret_close_path(command);
                break;
            case parser::PathCommandType::SmoothCubicBezierTo:
                interpreted_command = interpret_smooth_cubic_bezier_to(command);
                break;
            case parser::PathCommandType::QuadraticBezierTo:
                interpreted_command = interpret_quadratic_bezier_to(command);
                break;
            case parser::PathCommandType::SmoothQuadraticBezierTo:
                interpreted_command = interpret_smooth_quadratic_bezier_to(command);
                break;
            case parser::PathCommandType::ArcTo:
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

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_move_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_line_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_horizontal_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_vertical_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_cubic_bezier_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_close_path(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_smooth_cubic_bezier_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_quadratic_bezier_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_smooth_quadratic_bezier_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

std::optional<std::vector<PathInstruction>> SvgPathInterpreter::interpret_arc_to(
    const parser::PathCommand& command) {
    (void)command;
    return std::nullopt;
}

}
}
