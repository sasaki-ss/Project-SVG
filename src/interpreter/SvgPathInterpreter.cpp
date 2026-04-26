#include "SvgPathInterpreter.h"

#include "SvgInterpreterUtility.h"

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

    context = InterpretContext{};

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
    constexpr std::size_t MOVE_TO_PARAMETER_PAIR_SIZE = 2;
    const std::size_t parameter_size = command.parameters.size();
    if (parameter_size < MOVE_TO_PARAMETER_PAIR_SIZE
        || parameter_size % MOVE_TO_PARAMETER_PAIR_SIZE != 0) {
        return std::nullopt;
    }

    const auto parsed_point = parse_point(command, 0);
    if (!parsed_point.has_value()) {
        return std::nullopt;
    }

    const auto absolute_point = make_absolute_point(*parsed_point, command.is_absolute);

    std::vector<PathInstruction> instructions;
    PathInstruction move_to_instruction;
    move_to_instruction.type = PathInstructionType::MoveTo;
    move_to_instruction.points.push_back(absolute_point);
    instructions.push_back(move_to_instruction);

    context.current_point = absolute_point;
    context.has_current_point = true;
    context.subpath_start_point = absolute_point;
    context.has_subpath_start_point = true;

    if (parameter_size == MOVE_TO_PARAMETER_PAIR_SIZE) {
        return instructions;
    }

    // M/m parameters after the first pair are interpreted as LineTo.
    const auto line_instructions = interpret_line_to(command, MOVE_TO_PARAMETER_PAIR_SIZE);
    if (!line_instructions.has_value()) {
        return std::nullopt;
    }

    instructions.insert(
        instructions.end(), line_instructions->begin(), line_instructions->end());

    return instructions;
}

auto SvgPathInterpreter::interpret_line_to(
    const PathCommand& command,
    std::size_t start_index)
    -> std::optional<std::vector<PathInstruction>> {
    constexpr std::size_t LINE_TO_PARAMETER_PAIR_SIZE = 2;
    const std::size_t parameter_size = command.parameters.size();
    if (!context.has_current_point) {
        return std::nullopt;
    }

    if (start_index >= parameter_size) {
        return std::nullopt;
    }

    const std::size_t remaining_parameter_size = parameter_size - start_index;
    if (remaining_parameter_size % LINE_TO_PARAMETER_PAIR_SIZE != 0) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (std::size_t index = start_index; index < parameter_size; index += LINE_TO_PARAMETER_PAIR_SIZE) {
        const auto parsed_point = parse_point(command, index);
        if (!parsed_point.has_value()) {
            return std::nullopt;
        }

        const auto absolute_point = make_absolute_point(*parsed_point, command.is_absolute);

        PathInstruction line_to_instruction;
        line_to_instruction.type = PathInstructionType::LineTo;
        line_to_instruction.points.push_back(absolute_point);
        instructions.push_back(line_to_instruction);

        context.current_point = absolute_point;
    }

    return instructions;
}

auto SvgPathInterpreter::interpret_horizontal_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    if (!context.has_current_point) {
        return std::nullopt;
    }

    if (command.parameters.empty()) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (const auto& parameter : command.parameters) {
        const auto parsed_x = SvgInterpreterUtility::parse_double(parameter);
        if (!parsed_x.has_value()) {
            return std::nullopt;
        }

        double x = *parsed_x;
        double y = context.current_point.y;
        if (!command.is_absolute) {
            x = context.current_point.x + *parsed_x;
        }
        const Point point{x, y};

        PathInstruction line_to_instruction;
        line_to_instruction.type = PathInstructionType::LineTo;
        line_to_instruction.points.push_back(point);
        instructions.push_back(line_to_instruction);

        context.current_point = point;
    }

    return instructions;
}

auto SvgPathInterpreter::interpret_vertical_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    if (!context.has_current_point) {
        return std::nullopt;
    }

    if (command.parameters.empty()) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (const auto& parameter : command.parameters) {
        const auto parsed_y = SvgInterpreterUtility::parse_double(parameter);
        if (!parsed_y.has_value()) {
            return std::nullopt;
        }

        double x = context.current_point.x;
        double y = *parsed_y;
        if (!command.is_absolute) {
            y = context.current_point.y + *parsed_y;
        }
        const Point point{x, y};

        PathInstruction line_to_instruction;
        line_to_instruction.type = PathInstructionType::LineTo;
        line_to_instruction.points.push_back(point);
        instructions.push_back(line_to_instruction);

        context.current_point = point;
    }

    return instructions;
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

auto SvgPathInterpreter::parse_point(
    const PathCommand& command,
    std::size_t index)
    -> std::optional<Point> {
    if (index + 1 >= command.parameters.size()) {
        return std::nullopt;
    }

    const auto x = SvgInterpreterUtility::parse_double(command.parameters[index]);
    if (!x.has_value()) {
        return std::nullopt;
    }

    const auto y = SvgInterpreterUtility::parse_double(command.parameters[index + 1]);
    if (!y.has_value()) {
        return std::nullopt;
    }

    return Point{*x, *y};
}

auto SvgPathInterpreter::make_absolute_point(
    const Point& point,
    bool is_absolute)
    -> Point {
    if (is_absolute) {
        return point;
    }

    return Point{
        context.current_point.x + point.x,
        context.current_point.y + point.y,
    };
}

}
}
