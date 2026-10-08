#include <gtest/gtest.h>

#include <cmath>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "interpreter/PathInstruction.h"
#include "interpreter/SvgPathInterpreter.h"
#include "parser/PathCommand.h"

namespace {

using svg::interpreter::PathInstruction;
using svg::interpreter::PathInstructionType;
using svg::interpreter::Point;
using svg::interpreter::SvgPathInterpreter;
using svg::parser::PathCommand;
using svg::parser::PathCommandType;

PathCommand make_command(
    PathCommandType type,
    bool is_absolute,
    std::initializer_list<std::string_view> parameters) {
    PathCommand command;
    command.type = type;
    command.is_absolute = is_absolute;
    for (const std::string_view parameter : parameters) {
        command.parameters.emplace_back(parameter);
    }
    return command;
}

PathCommand make_move_to(
    std::string_view x,
    std::string_view y,
    bool is_absolute = true) {
    return make_command(PathCommandType::MoveTo, is_absolute, {x, y});
}

auto interpret_commands(const std::vector<PathCommand>& commands)
    -> std::optional<std::vector<PathInstruction>> {
    SvgPathInterpreter interpreter;
    return interpreter.interpret(commands);
}

void expect_point(const Point& point, double expected_x, double expected_y) {
    EXPECT_DOUBLE_EQ(point.x, expected_x);
    EXPECT_DOUBLE_EQ(point.y, expected_y);
}

void expect_cubic(
    const PathInstruction& instruction,
    const Point& control_point1,
    const Point& control_point2,
    const Point& end_point) {
    ASSERT_EQ(instruction.type, PathInstructionType::CubicBezierTo);
    ASSERT_EQ(instruction.points.size(), 3U);
    expect_point(
        instruction.points[0],
        control_point1.x,
        control_point1.y);
    expect_point(
        instruction.points[1],
        control_point2.x,
        control_point2.y);
    expect_point(instruction.points[2], end_point.x, end_point.y);
}

Point evaluate_cubic(
    const Point& start_point,
    const PathInstruction& instruction,
    double t) {
    const double inverse_t = 1.0 - t;
    const double start_weight = inverse_t * inverse_t * inverse_t;
    const double control_point1_weight = 3.0 * inverse_t * inverse_t * t;
    const double control_point2_weight = 3.0 * inverse_t * t * t;
    const double end_weight = t * t * t;
    return Point{
        start_weight * start_point.x
            + control_point1_weight * instruction.points[0].x
            + control_point2_weight * instruction.points[1].x
            + end_weight * instruction.points[2].x,
        start_weight * start_point.y
            + control_point1_weight * instruction.points[0].y
            + control_point2_weight * instruction.points[1].y
            + end_weight * instruction.points[2].y,
    };
}

TEST(SvgPathInterpreterTest, MoveToAbsoluteUsesGivenPoint) {
    const auto result = interpret_commands({make_move_to("5", "6")});

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    ASSERT_EQ(result->at(0).type, PathInstructionType::MoveTo);
    ASSERT_EQ(result->at(0).points.size(), 1U);
    expect_point(result->at(0).points[0], 5.0, 6.0);
}

TEST(SvgPathInterpreterTest, MoveToRelativeAtPathStartUsesOrigin) {
    const auto result = interpret_commands({make_move_to("5", "6", false)});

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    ASSERT_EQ(result->at(0).type, PathInstructionType::MoveTo);
    ASSERT_EQ(result->at(0).points.size(), 1U);
    expect_point(result->at(0).points[0], 5.0, 6.0);
}

TEST(SvgPathInterpreterTest, LineToAbsoluteUsesGivenPoint) {
    const auto result = interpret_commands({
        make_move_to("1", "2"),
        make_command(PathCommandType::LineTo, true, {"3", "4"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::LineTo);
    expect_point(result->at(1).points[0], 3.0, 4.0);
}

TEST(SvgPathInterpreterTest, LineToRelativeAddsCurrentPoint) {
    const auto result = interpret_commands({
        make_move_to("1", "2"),
        make_command(PathCommandType::LineTo, false, {"3", "4"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::LineTo);
    expect_point(result->at(1).points[0], 4.0, 6.0);
}

TEST(SvgPathInterpreterTest, HorizontalToAbsoluteUsesCurrentY) {
    const auto result = interpret_commands({
        make_move_to("1", "2"),
        make_command(PathCommandType::HorizontalTo, true, {"5"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::LineTo);
    expect_point(result->at(1).points[0], 5.0, 2.0);
}

TEST(SvgPathInterpreterTest, HorizontalToRelativeAddsCurrentX) {
    const auto result = interpret_commands({
        make_move_to("1", "2"),
        make_command(PathCommandType::HorizontalTo, false, {"5"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::LineTo);
    expect_point(result->at(1).points[0], 6.0, 2.0);
}

TEST(SvgPathInterpreterTest, VerticalToAbsoluteUsesCurrentX) {
    const auto result = interpret_commands({
        make_move_to("1", "2"),
        make_command(PathCommandType::VerticalTo, true, {"5"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::LineTo);
    expect_point(result->at(1).points[0], 1.0, 5.0);
}

TEST(SvgPathInterpreterTest, VerticalToRelativeAddsCurrentY) {
    const auto result = interpret_commands({
        make_move_to("1", "2"),
        make_command(PathCommandType::VerticalTo, false, {"5"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::LineTo);
    expect_point(result->at(1).points[0], 1.0, 7.0);
}

TEST(SvgPathInterpreterTest, CubicBezierToAbsoluteUsesGivenPoints) {
    const auto result = interpret_commands({
        make_move_to("0", "0"),
        make_command(
            PathCommandType::CubicBezierTo,
            true,
            {"1", "2", "3", "4", "5", "6"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_cubic(result->at(1), {1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0});
}

TEST(SvgPathInterpreterTest, CubicBezierToRelativeAddsCurrentPoint) {
    const auto result = interpret_commands({
        make_move_to("10", "20"),
        make_command(
            PathCommandType::CubicBezierTo,
            false,
            {"1", "2", "3", "4", "5", "6"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_cubic(result->at(1), {11.0, 22.0}, {13.0, 24.0}, {15.0, 26.0});
}

TEST(SvgPathInterpreterTest, ClosePathAbsoluteRestoresSubpathStart) {
    const auto result = interpret_commands({
        make_move_to("2", "3"),
        make_command(PathCommandType::LineTo, true, {"4", "3"}),
        make_command(PathCommandType::ClosePath, true, {}),
        make_command(PathCommandType::LineTo, true, {"5", "6"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 4U);
    ASSERT_EQ(result->at(2).type, PathInstructionType::ClosePath);
    EXPECT_TRUE(result->at(2).points.empty());
    expect_point(result->at(3).points[0], 5.0, 6.0);
}

TEST(SvgPathInterpreterTest, ClosePathRelativeRestoresSubpathStartForFollowingCommand) {
    const auto result = interpret_commands({
        make_move_to("2", "3"),
        make_command(PathCommandType::LineTo, true, {"4", "3"}),
        make_command(PathCommandType::ClosePath, false, {}),
        make_command(PathCommandType::LineTo, false, {"1", "2"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 4U);
    ASSERT_EQ(result->at(2).type, PathInstructionType::ClosePath);
    EXPECT_TRUE(result->at(2).points.empty());
    expect_point(result->at(3).points[0], 3.0, 5.0);
}

TEST(SvgPathInterpreterTest, SmoothCubicAbsoluteReflectsPreviousCubicControlPoint) {
    const auto result = interpret_commands({
        make_move_to("0", "0"),
        make_command(
            PathCommandType::CubicBezierTo,
            true,
            {"1", "1", "2", "3", "4", "5"}),
        make_command(
            PathCommandType::SmoothCubicBezierTo,
            true,
            {"8", "9", "10", "11"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_cubic(result->at(2), {6.0, 7.0}, {8.0, 9.0}, {10.0, 11.0});
}

TEST(SvgPathInterpreterTest, SmoothCubicRelativeUsesCurrentPointWithoutCubicPredecessor) {
    const auto result = interpret_commands({
        make_move_to("10", "10"),
        make_command(PathCommandType::LineTo, true, {"11", "11"}),
        make_command(
            PathCommandType::SmoothCubicBezierTo,
            false,
            {"2", "3", "4", "5"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_cubic(result->at(2), {11.0, 11.0}, {13.0, 14.0}, {15.0, 16.0});
}

TEST(SvgPathInterpreterTest, QuadraticBezierAbsoluteConvertsToCubicBezier) {
    const auto result = interpret_commands({
        make_move_to("0", "0"),
        make_command(PathCommandType::QuadraticBezierTo, true, {"3", "3", "6", "0"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_cubic(result->at(1), {2.0, 2.0}, {4.0, 2.0}, {6.0, 0.0});
}

TEST(SvgPathInterpreterTest, QuadraticBezierRelativeConvertsToCubicBezier) {
    const auto result = interpret_commands({
        make_move_to("10", "10"),
        make_command(PathCommandType::QuadraticBezierTo, false, {"3", "3", "6", "0"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_cubic(result->at(1), {12.0, 12.0}, {14.0, 12.0}, {16.0, 10.0});
}

TEST(SvgPathInterpreterTest, SmoothQuadraticAbsoluteReflectsPreviousQuadraticControlPoint) {
    const auto result = interpret_commands({
        make_move_to("0", "0"),
        make_command(PathCommandType::QuadraticBezierTo, true, {"2", "2", "4", "0"}),
        make_command(PathCommandType::SmoothQuadraticBezierTo, true, {"8", "0"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_cubic(
        result->at(2),
        {5.333333333333333, -1.333333333333333},
        {6.666666666666667, -1.333333333333333},
        {8.0, 0.0});
}

TEST(
    SvgPathInterpreterTest,
    SmoothQuadraticRelativeUsesCurrentPointWithoutQuadraticPredecessor) {
    const auto result = interpret_commands({
        make_move_to("10", "10"),
        make_command(PathCommandType::LineTo, true, {"11", "10"}),
        make_command(PathCommandType::SmoothQuadraticBezierTo, false, {"4", "0"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_cubic(
        result->at(2),
        {11.0, 10.0},
        {12.333333333333334, 10.0},
        {15.0, 10.0});
}

TEST(SvgPathInterpreterTest, ArcAbsoluteQuarterCircleUsesOneCubicBezier) {
    constexpr double ARC_RADIUS_TOLERANCE = 0.001;

    const auto result = interpret_commands({
        make_move_to("1", "0"),
        make_command(PathCommandType::ArcTo, true, {"1", "1", "0", "0", "1", "0", "1"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    expect_cubic(
        result->at(1),
        {1.0, 0.5522847498307933},
        {0.5522847498307933, 1.0},
        {0.0, 1.0});
    const Point middle_point = evaluate_cubic({1.0, 0.0}, result->at(1), 0.5);
    EXPECT_NEAR(std::hypot(middle_point.x, middle_point.y), 1.0, ARC_RADIUS_TOLERANCE);
}

TEST(SvgPathInterpreterTest, ArcRelativeQuarterCircleUsesRelativeEndpointOnly) {
    const auto result = interpret_commands({
        make_move_to("1", "0"),
        make_command(PathCommandType::ArcTo, false, {"1", "1", "0", "0", "1", "-1", "1"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::CubicBezierTo);
    ASSERT_EQ(result->at(1).points.size(), 3U);
    expect_point(result->at(1).points[2], 0.0, 1.0);
}

TEST(SvgPathInterpreterTest, ArcHalfCircleUsesTwoCubicBezierSegments) {
    constexpr double ARC_RADIUS_TOLERANCE = 0.001;

    const auto result = interpret_commands({
        make_move_to("1", "0"),
        make_command(PathCommandType::ArcTo, true, {"1", "1", "0", "0", "1", "-1", "0"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::CubicBezierTo);
    ASSERT_EQ(result->at(2).type, PathInstructionType::CubicBezierTo);
    expect_point(result->at(2).points[2], -1.0, 0.0);

    const Point first_middle_point = evaluate_cubic({1.0, 0.0}, result->at(1), 0.5);
    const Point second_middle_point = evaluate_cubic(
        result->at(1).points[2],
        result->at(2),
        0.5);
    EXPECT_NEAR(
        std::hypot(first_middle_point.x, first_middle_point.y),
        1.0,
        ARC_RADIUS_TOLERANCE);
    EXPECT_NEAR(
        std::hypot(second_middle_point.x, second_middle_point.y),
        1.0,
        ARC_RADIUS_TOLERANCE);
}

TEST(SvgPathInterpreterTest, ArcWithZeroRadiusConvertsToLineTo) {
    const auto result = interpret_commands({
        make_move_to("0", "0"),
        make_command(PathCommandType::ArcTo, true, {"0", "1", "0", "0", "1", "2", "3"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 2U);
    ASSERT_EQ(result->at(1).type, PathInstructionType::LineTo);
    ASSERT_EQ(result->at(1).points.size(), 1U);
    expect_point(result->at(1).points[0], 2.0, 3.0);
}

TEST(SvgPathInterpreterTest, ArcWithSmallRadiiExpandsToReachEndpoints) {
    constexpr double ARC_RADIUS_TOLERANCE = 0.001;

    const auto result = interpret_commands({
        make_move_to("0", "0"),
        make_command(PathCommandType::ArcTo, true, {"1", "1", "0", "0", "1", "10", "0"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_point(result->at(2).points[2], 10.0, 0.0);
    const Point middle_point = evaluate_cubic({0.0, 0.0}, result->at(1), 0.5);
    EXPECT_NEAR(std::hypot(middle_point.x - 5.0, middle_point.y), 5.0, ARC_RADIUS_TOLERANCE);
}

TEST(SvgPathInterpreterTest, ArcWithRotationUsesRotatedEllipseAxes) {
    constexpr double ARC_RADIUS_TOLERANCE = 0.001;

    const auto result = interpret_commands({
        make_move_to("0", "2"),
        make_command(PathCommandType::ArcTo, true, {"2", "1", "90", "0", "1", "0", "-2"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 3U);
    expect_point(result->at(2).points[2], 0.0, -2.0);
    const Point middle_point = evaluate_cubic({0.0, 2.0}, result->at(1), 0.5);
    const double normalized_radius = std::sqrt(
        middle_point.x * middle_point.x
        + middle_point.y * middle_point.y / 4.0);
    EXPECT_NEAR(normalized_radius, 1.0, ARC_RADIUS_TOLERANCE);
}

TEST(SvgPathInterpreterTest, ArcWithMatchingEndpointsProducesNoDrawingInstruction) {
    const auto result = interpret_commands({
        make_move_to("1", "2"),
        make_command(PathCommandType::ArcTo, true, {"3", "4", "0", "0", "1", "1", "2"}),
    });

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->size(), 1U);
    ASSERT_EQ(result->at(0).type, PathInstructionType::MoveTo);
}

TEST(SvgPathInterpreterTest, DrawingCommandWithoutCurrentPointFails) {
    const auto result = interpret_commands({
        make_command(PathCommandType::CubicBezierTo, true, {"1", "2", "3", "4", "5", "6"}),
    });

    EXPECT_FALSE(result.has_value());
}

}  // namespace
