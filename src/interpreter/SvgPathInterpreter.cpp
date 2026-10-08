#include "SvgPathInterpreter.h"

#include <algorithm>
#include <cmath>

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
        reset_previous_control_points(command.type);

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
        const double y = context.current_point.y;
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

        const double x = context.current_point.x;
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
    constexpr std::size_t CUBIC_BEZIER_PARAMETER_SIZE = 6;
    const std::size_t parameter_size = command.parameters.size();
    if (!context.has_current_point
        || parameter_size < CUBIC_BEZIER_PARAMETER_SIZE
        || parameter_size % CUBIC_BEZIER_PARAMETER_SIZE != 0) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (std::size_t index = 0; index < parameter_size;
         index += CUBIC_BEZIER_PARAMETER_SIZE) {
        const auto parsed_control_point1 = parse_point(command, index);
        const auto parsed_control_point2 = parse_point(command, index + 2);
        const auto parsed_end_point = parse_point(command, index + 4);
        if (!parsed_control_point1.has_value()
            || !parsed_control_point2.has_value()
            || !parsed_end_point.has_value()) {
            return std::nullopt;
        }

        const Point control_point1 = make_absolute_point(
            *parsed_control_point1,
            command.is_absolute);
        const Point control_point2 = make_absolute_point(
            *parsed_control_point2,
            command.is_absolute);
        const Point end_point = make_absolute_point(
            *parsed_end_point,
            command.is_absolute);

        PathInstruction instruction;
        instruction.type = PathInstructionType::CubicBezierTo;
        instruction.points = {control_point1, control_point2, end_point};
        instructions.push_back(instruction);

        context.current_point = end_point;
        context.previous_cubic_control_point = control_point2;
        context.has_previous_cubic_control_point = true;
    }

    return instructions;
}

auto SvgPathInterpreter::interpret_close_path(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    if (!command.parameters.empty()
        || !context.has_current_point
        || !context.has_subpath_start_point) {
        return std::nullopt;
    }

    PathInstruction instruction;
    instruction.type = PathInstructionType::ClosePath;

    context.current_point = context.subpath_start_point;
    return std::vector<PathInstruction>{instruction};
}

auto SvgPathInterpreter::interpret_smooth_cubic_bezier_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    constexpr std::size_t SMOOTH_CUBIC_BEZIER_PARAMETER_SIZE = 4;
    const std::size_t parameter_size = command.parameters.size();
    if (!context.has_current_point
        || parameter_size < SMOOTH_CUBIC_BEZIER_PARAMETER_SIZE
        || parameter_size % SMOOTH_CUBIC_BEZIER_PARAMETER_SIZE != 0) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (std::size_t index = 0; index < parameter_size;
         index += SMOOTH_CUBIC_BEZIER_PARAMETER_SIZE) {
        const auto parsed_control_point2 = parse_point(command, index);
        const auto parsed_end_point = parse_point(command, index + 2);
        if (!parsed_control_point2.has_value() || !parsed_end_point.has_value()) {
            return std::nullopt;
        }

        Point control_point1 = context.current_point;
        if (context.has_previous_cubic_control_point) {
            control_point1 = make_reflected_point(
                context.current_point,
                context.previous_cubic_control_point);
        }

        const Point control_point2 = make_absolute_point(
            *parsed_control_point2,
            command.is_absolute);
        const Point end_point = make_absolute_point(
            *parsed_end_point,
            command.is_absolute);

        PathInstruction instruction;
        instruction.type = PathInstructionType::CubicBezierTo;
        instruction.points = {control_point1, control_point2, end_point};
        instructions.push_back(instruction);

        context.current_point = end_point;
        context.previous_cubic_control_point = control_point2;
        context.has_previous_cubic_control_point = true;
    }

    return instructions;
}

auto SvgPathInterpreter::interpret_quadratic_bezier_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    constexpr std::size_t QUADRATIC_BEZIER_PARAMETER_SIZE = 4;
    const std::size_t parameter_size = command.parameters.size();
    if (!context.has_current_point
        || parameter_size < QUADRATIC_BEZIER_PARAMETER_SIZE
        || parameter_size % QUADRATIC_BEZIER_PARAMETER_SIZE != 0) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (std::size_t index = 0; index < parameter_size;
         index += QUADRATIC_BEZIER_PARAMETER_SIZE) {
        const auto parsed_control_point = parse_point(command, index);
        const auto parsed_end_point = parse_point(command, index + 2);
        if (!parsed_control_point.has_value() || !parsed_end_point.has_value()) {
            return std::nullopt;
        }

        const Point control_point = make_absolute_point(
            *parsed_control_point,
            command.is_absolute);
        const Point end_point = make_absolute_point(
            *parsed_end_point,
            command.is_absolute);
        const CubicBezierPoints cubic = make_quadratic_cubic_points(
            context.current_point,
            control_point,
            end_point);

        PathInstruction instruction;
        instruction.type = PathInstructionType::CubicBezierTo;
        instruction.points = {
            cubic.control_point1,
            cubic.control_point2,
            cubic.end_point,
        };
        instructions.push_back(instruction);

        context.current_point = end_point;
        context.previous_quadratic_control_point = control_point;
        context.has_previous_quadratic_control_point = true;
    }

    return instructions;
}

auto SvgPathInterpreter::interpret_smooth_quadratic_bezier_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    constexpr std::size_t SMOOTH_QUADRATIC_BEZIER_PARAMETER_SIZE = 2;
    const std::size_t parameter_size = command.parameters.size();
    if (!context.has_current_point
        || parameter_size < SMOOTH_QUADRATIC_BEZIER_PARAMETER_SIZE
        || parameter_size % SMOOTH_QUADRATIC_BEZIER_PARAMETER_SIZE != 0) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (std::size_t index = 0; index < parameter_size;
         index += SMOOTH_QUADRATIC_BEZIER_PARAMETER_SIZE) {
        const auto parsed_end_point = parse_point(command, index);
        if (!parsed_end_point.has_value()) {
            return std::nullopt;
        }

        Point control_point = context.current_point;
        if (context.has_previous_quadratic_control_point) {
            control_point = make_reflected_point(
                context.current_point,
                context.previous_quadratic_control_point);
        }

        const Point end_point = make_absolute_point(
            *parsed_end_point,
            command.is_absolute);
        const CubicBezierPoints cubic = make_quadratic_cubic_points(
            context.current_point,
            control_point,
            end_point);

        PathInstruction instruction;
        instruction.type = PathInstructionType::CubicBezierTo;
        instruction.points = {
            cubic.control_point1,
            cubic.control_point2,
            cubic.end_point,
        };
        instructions.push_back(instruction);

        context.current_point = end_point;
        context.previous_quadratic_control_point = control_point;
        context.has_previous_quadratic_control_point = true;
    }

    return instructions;
}

auto SvgPathInterpreter::interpret_arc_to(
    const PathCommand& command)
    -> std::optional<std::vector<PathInstruction>> {
    constexpr std::size_t ARC_PARAMETER_SIZE = 7;

    const std::size_t parameter_size = command.parameters.size();
    if (!context.has_current_point
        || parameter_size < ARC_PARAMETER_SIZE
        || parameter_size % ARC_PARAMETER_SIZE != 0) {
        return std::nullopt;
    }

    std::vector<PathInstruction> instructions;
    for (std::size_t index = 0; index < parameter_size;
         index += ARC_PARAMETER_SIZE) {
        if (!append_arc_instructions(command, index, instructions)) {
            return std::nullopt;
        }
    }

    return instructions;
}

auto SvgPathInterpreter::parse_arc_parameters(
    const PathCommand& command,
    std::size_t index)
    -> std::optional<ArcCommandParameters> {
    constexpr std::size_t LARGE_ARC_FLAG_INDEX = 3;
    constexpr std::size_t SWEEP_FLAG_INDEX = 4;
    constexpr std::size_t END_POINT_INDEX = 5;

    const auto parsed_radius_x = SvgInterpreterUtility::parse_double(
        command.parameters[index]);
    const auto parsed_radius_y = SvgInterpreterUtility::parse_double(
        command.parameters[index + 1]);
    const auto parsed_rotation = SvgInterpreterUtility::parse_double(
        command.parameters[index + 2]);
    const auto large_arc_flag = parse_arc_flag(
        command.parameters[index + LARGE_ARC_FLAG_INDEX]);
    const auto sweep_flag = parse_arc_flag(
        command.parameters[index + SWEEP_FLAG_INDEX]);
    const auto parsed_end_point = parse_point(
        command,
        index + END_POINT_INDEX);
    if (!parsed_radius_x.has_value()
        || !parsed_radius_y.has_value()
        || !parsed_rotation.has_value()
        || !large_arc_flag.has_value()
        || !sweep_flag.has_value()
        || !parsed_end_point.has_value()) {
        return std::nullopt;
    }

    ArcCommandParameters parameters;
    parameters.radius_x = *parsed_radius_x;
    parameters.radius_y = *parsed_radius_y;
    parameters.rotation = *parsed_rotation;
    parameters.is_large_arc = *large_arc_flag;
    parameters.is_sweep = *sweep_flag;
    parameters.end_point = make_absolute_point(*parsed_end_point, command.is_absolute);
    return parameters;
}

bool SvgPathInterpreter::append_arc_instructions(
    const PathCommand& command,
    std::size_t index,
    std::vector<PathInstruction>& instructions) {
    const auto parameters = parse_arc_parameters(command, index);
    if (!parameters.has_value()) {
        return false;
    }

    const Point end_point = parameters->end_point;
    if (end_point.x == context.current_point.x
        && end_point.y == context.current_point.y) {
        context.current_point = end_point;
        return true;
    }

    const double radius_x = std::abs(parameters->radius_x);
    const double radius_y = std::abs(parameters->radius_y);
    if (radius_x == 0.0 || radius_y == 0.0) {
        PathInstruction instruction;
        instruction.type = PathInstructionType::LineTo;
        instruction.points = {end_point};
        instructions.push_back(instruction);
        context.current_point = end_point;
        return true;
    }

    const auto arc = make_arc_center_parameters(
        context.current_point,
        end_point,
        radius_x,
        radius_y,
        parameters->rotation,
        parameters->is_large_arc,
        parameters->is_sweep);
    if (!arc.has_value()) {
        return false;
    }

    if (!append_arc_segments(*arc, end_point, instructions)) {
        return false;
    }

    context.current_point = end_point;
    return true;
}

bool SvgPathInterpreter::append_arc_segments(
    const ArcCenterParameters& arc,
    const Point& end_point,
    std::vector<PathInstruction>& instructions) {
    constexpr double MAX_ARC_SEGMENT_ANGLE = 1.57079632679489661923;

    const std::size_t segment_count = static_cast<std::size_t>(std::ceil(
        std::abs(arc.angle_delta) / MAX_ARC_SEGMENT_ANGLE));
    if (segment_count == 0) {
        return false;
    }

    const double segment_angle = arc.angle_delta
        / static_cast<double>(segment_count);
    for (std::size_t segment_index = 0;
         segment_index < segment_count;
         ++segment_index) {
        CubicBezierPoints cubic = make_arc_segment(
            arc,
            arc.start_angle
                + segment_angle * static_cast<double>(segment_index),
            segment_angle);
        if (segment_index + 1 == segment_count) {
            cubic.end_point = end_point;
        }

        PathInstruction instruction;
        instruction.type = PathInstructionType::CubicBezierTo;
        instruction.points = {
            cubic.control_point1,
            cubic.control_point2,
            cubic.end_point,
        };
        instructions.push_back(instruction);
    }
    return true;
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

void SvgPathInterpreter::reset_previous_control_points(
    PathCommandType command_type) {
    if (!is_cubic_command(command_type)) {
        context.has_previous_cubic_control_point = false;
    }

    if (!is_quadratic_command(command_type)) {
        context.has_previous_quadratic_control_point = false;
    }
}

bool SvgPathInterpreter::is_cubic_command(PathCommandType command_type) {
    return command_type == PathCommandType::CubicBezierTo
        || command_type == PathCommandType::SmoothCubicBezierTo;
}

bool SvgPathInterpreter::is_quadratic_command(PathCommandType command_type) {
    return command_type == PathCommandType::QuadraticBezierTo
        || command_type == PathCommandType::SmoothQuadraticBezierTo;
}

auto SvgPathInterpreter::make_reflected_point(
    const Point& origin,
    const Point& control_point)
    -> Point {
    return Point{
        2.0 * origin.x - control_point.x,
        2.0 * origin.y - control_point.y,
    };
}

auto SvgPathInterpreter::make_quadratic_cubic_points(
    const Point& start_point,
    const Point& quadratic_control_point,
    const Point& end_point)
    -> CubicBezierPoints {
    constexpr double CONTROL_POINT_RATIO = 2.0 / 3.0;

    CubicBezierPoints cubic;
    cubic.control_point1 = Point{
        start_point.x + CONTROL_POINT_RATIO
            * (quadratic_control_point.x - start_point.x),
        start_point.y + CONTROL_POINT_RATIO
            * (quadratic_control_point.y - start_point.y),
    };
    cubic.control_point2 = Point{
        end_point.x + CONTROL_POINT_RATIO
            * (quadratic_control_point.x - end_point.x),
        end_point.y + CONTROL_POINT_RATIO
            * (quadratic_control_point.y - end_point.y),
    };
    cubic.end_point = end_point;
    return cubic;
}

auto SvgPathInterpreter::parse_arc_flag(
    const std::string& parameter)
    -> std::optional<bool> {
    if (parameter == "0") {
        return false;
    }

    if (parameter == "1") {
        return true;
    }

    return std::nullopt;
}

auto SvgPathInterpreter::make_arc_center_parameters(
    const Point& start_point,
    const Point& end_point,
    double radius_x,
    double radius_y,
    double rotation,
    bool is_large_arc,
    bool is_sweep)
    -> std::optional<ArcCenterParameters> {
    constexpr double PI = 3.14159265358979323846;
    constexpr double DEGREES_TO_RADIANS = PI / 180.0;

    const double rotation_radians = rotation * DEGREES_TO_RADIANS;
    const double cos_rotation = std::cos(rotation_radians);
    const double sin_rotation = std::sin(rotation_radians);
    const double half_delta_x = (start_point.x - end_point.x) / 2.0;
    const double half_delta_y = (start_point.y - end_point.y) / 2.0;
    const double transformed_start_x = cos_rotation * half_delta_x
        + sin_rotation * half_delta_y;
    const double transformed_start_y = -sin_rotation * half_delta_x
        + cos_rotation * half_delta_y;

    const double radius_x_squared = radius_x * radius_x;
    const double radius_y_squared = radius_y * radius_y;
    const double transformed_start_x_squared = transformed_start_x
        * transformed_start_x;
    const double transformed_start_y_squared = transformed_start_y
        * transformed_start_y;
    const double radius_scale = transformed_start_x_squared / radius_x_squared
        + transformed_start_y_squared / radius_y_squared;
    if (radius_scale > 1.0) {
        const double scale = std::sqrt(radius_scale);
        radius_x *= scale;
        radius_y *= scale;
    }

    const double adjusted_radius_x_squared = radius_x * radius_x;
    const double adjusted_radius_y_squared = radius_y * radius_y;
    const double numerator = adjusted_radius_x_squared * adjusted_radius_y_squared
        - adjusted_radius_x_squared * transformed_start_y_squared
        - adjusted_radius_y_squared * transformed_start_x_squared;
    const double denominator = adjusted_radius_x_squared
        * transformed_start_y_squared
        + adjusted_radius_y_squared * transformed_start_x_squared;
    if (denominator == 0.0) {
        return std::nullopt;
    }

    double center_factor = std::sqrt(std::max(0.0, numerator / denominator));
    if (is_large_arc == is_sweep) {
        center_factor = -center_factor;
    }

    const double transformed_center_x = center_factor
        * (radius_x * transformed_start_y / radius_y);
    const double transformed_center_y = center_factor
        * (-radius_y * transformed_start_x / radius_x);
    const Point center{
        cos_rotation * transformed_center_x
            - sin_rotation * transformed_center_y
            + (start_point.x + end_point.x) / 2.0,
        sin_rotation * transformed_center_x
            + cos_rotation * transformed_center_y
            + (start_point.y + end_point.y) / 2.0,
    };

    const double start_vector_x = (transformed_start_x - transformed_center_x)
        / radius_x;
    const double start_vector_y = (transformed_start_y - transformed_center_y)
        / radius_y;
    const double end_vector_x = (-transformed_start_x - transformed_center_x)
        / radius_x;
    const double end_vector_y = (-transformed_start_y - transformed_center_y)
        / radius_y;
    const double start_angle = std::atan2(start_vector_y, start_vector_x);
    double angle_delta = std::atan2(
        start_vector_x * end_vector_y - start_vector_y * end_vector_x,
        start_vector_x * end_vector_x + start_vector_y * end_vector_y);
    if (!is_sweep && angle_delta > 0.0) {
        angle_delta -= 2.0 * PI;
    }
    if (is_sweep && angle_delta < 0.0) {
        angle_delta += 2.0 * PI;
    }

    ArcCenterParameters arc;
    arc.center = center;
    arc.radius_x = radius_x;
    arc.radius_y = radius_y;
    arc.cos_rotation = cos_rotation;
    arc.sin_rotation = sin_rotation;
    arc.start_angle = start_angle;
    arc.angle_delta = angle_delta;
    return arc;
}

auto SvgPathInterpreter::make_arc_segment(
    const ArcCenterParameters& arc,
    double start_angle,
    double angle_delta)
    -> CubicBezierPoints {
    constexpr double CUBIC_ARC_FACTOR = 4.0 / 3.0;

    const double end_angle = start_angle + angle_delta;
    const double control_point_scale = CUBIC_ARC_FACTOR
        * std::tan(angle_delta / 4.0);
    const double start_cos = std::cos(start_angle);
    const double start_sin = std::sin(start_angle);
    const double end_cos = std::cos(end_angle);
    const double end_sin = std::sin(end_angle);

    CubicBezierPoints cubic;
    cubic.control_point1 = transform_arc_point(
        arc,
        start_cos - control_point_scale * start_sin,
        start_sin + control_point_scale * start_cos);
    cubic.control_point2 = transform_arc_point(
        arc,
        end_cos + control_point_scale * end_sin,
        end_sin - control_point_scale * end_cos);
    cubic.end_point = transform_arc_point(arc, end_cos, end_sin);
    return cubic;
}

auto SvgPathInterpreter::transform_arc_point(
    const ArcCenterParameters& arc,
    double x,
    double y)
    -> Point {
    return Point{
        arc.center.x + arc.cos_rotation * arc.radius_x * x
            - arc.sin_rotation * arc.radius_y * y,
        arc.center.y + arc.sin_rotation * arc.radius_x * x
            + arc.cos_rotation * arc.radius_y * y,
    };
}

}
}
