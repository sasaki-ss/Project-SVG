#ifndef PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_
#define PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_

#include <cstddef>
#include <optional>
#include <string>
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
        Point previous_cubic_control_point{0.0, 0.0};
        Point previous_quadratic_control_point{0.0, 0.0};
        bool has_current_point{false};
        bool has_subpath_start_point{false};
        bool has_previous_cubic_control_point{false};
        bool has_previous_quadratic_control_point{false};
    };

    struct CubicBezierPoints {
        Point control_point1;
        Point control_point2;
        Point end_point;
    };

    struct ArcCommandParameters {
        double radius_x;
        double radius_y;
        double rotation;
        bool is_large_arc;
        bool is_sweep;
        Point end_point;
    };

    struct ArcCenterParameters {
        Point center;
        double radius_x;
        double radius_y;
        double cos_rotation;
        double sin_rotation;
        double start_angle;
        double angle_delta;
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
    auto parse_arc_parameters(
        const parser::PathCommand& command,
        std::size_t index)
        -> std::optional<ArcCommandParameters>;
    bool append_arc_instructions(
        const parser::PathCommand& command,
        std::size_t index,
        std::vector<PathInstruction>& instructions);

    auto parse_point(
        const parser::PathCommand& command,
        std::size_t index)
        -> std::optional<Point>;
    auto make_absolute_point(
        const Point& point,
        bool is_absolute)
        -> Point;
    void reset_previous_control_points(parser::PathCommandType command_type);

    static bool is_cubic_command(parser::PathCommandType command_type);
    static bool is_quadratic_command(parser::PathCommandType command_type);
    static auto make_reflected_point(
        const Point& origin,
        const Point& control_point)
        -> Point;
    static auto make_quadratic_cubic_points(
        const Point& start_point,
        const Point& quadratic_control_point,
        const Point& end_point)
        -> CubicBezierPoints;
    static auto parse_arc_flag(const std::string& parameter) -> std::optional<bool>;
    static auto make_arc_center_parameters(
        const Point& start_point,
        const Point& end_point,
        double radius_x,
        double radius_y,
        double rotation,
        bool is_large_arc,
        bool is_sweep)
        -> std::optional<ArcCenterParameters>;
    static bool append_arc_segments(
        const ArcCenterParameters& arc,
        const Point& end_point,
        std::vector<PathInstruction>& instructions);
    static auto make_arc_segment(
        const ArcCenterParameters& arc,
        double start_angle,
        double angle_delta)
        -> CubicBezierPoints;
    static auto transform_arc_point(
        const ArcCenterParameters& arc,
        double x,
        double y)
        -> Point;

    InterpretContext context;
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_SVG_PATH_INTERPRETER_H_
