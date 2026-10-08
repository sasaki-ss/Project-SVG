#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "draw/DrawShape.h"
#include "interpreter/InterpretedSvg.h"
#include "interpreter/PathInstruction.h"
#include "rasterizer/RgbaImage.h"
#include "rasterizer/SvgRasterizer.h"

using svg::draw::DrawShape;
using svg::interpreter::PathInstruction;
using svg::interpreter::PathInstructionType;
using svg::interpreter::Point;
using svg::interpreter::StrokeLineCap;
using svg::interpreter::StrokeLineJoin;
using svg::interpreter::SvgViewBox;
using svg::rasterizer::RgbaColor;
using svg::rasterizer::RgbaImage;
using svg::rasterizer::SvgRasterizer;

class SvgRasterizerTest : public ::testing::Test {
protected:
    static PathInstruction make_move_to(double x, double y) {
        return PathInstruction{PathInstructionType::MoveTo, {{x, y}}};
    }

    static PathInstruction make_line_to(double x, double y) {
        return PathInstruction{PathInstructionType::LineTo, {{x, y}}};
    }

    static PathInstruction make_cubic_bezier_to(
        Point control_point1,
        Point control_point2,
        Point end_point) {
        return PathInstruction{
            PathInstructionType::CubicBezierTo,
            {control_point1, control_point2, end_point},
        };
    }

    static PathInstruction make_close_path() {
        return PathInstruction{PathInstructionType::ClosePath, {}};
    }

    static DrawShape make_shape(
        std::vector<PathInstruction> path_instructions,
        bool has_fill,
        bool has_stroke,
        double stroke_width = 0.0) {
        DrawShape draw_shape;
        draw_shape.path_instructions = std::move(path_instructions);
        draw_shape.has_fill = has_fill;
        draw_shape.has_stroke = has_stroke;
        draw_shape.stroke_width = stroke_width;
        draw_shape.stroke_linecap = StrokeLineCap::Round;
        draw_shape.stroke_linejoin = StrokeLineJoin::Round;
        return draw_shape;
    }

    static SvgViewBox make_view_box(double width, double height) {
        return SvgViewBox{0.0, 0.0, width, height};
    }

    static std::vector<PathInstruction> make_closed_rectangle(
        double x,
        double y,
        double width,
        double height) {
        return {
            make_move_to(x, y),
            make_line_to(x + width, y),
            make_line_to(x + width, y + height),
            make_line_to(x, y + height),
            make_close_path(),
        };
    }

    static std::vector<PathInstruction> make_cubic_circle(
        double center_x,
        double center_y,
        double radius) {
        constexpr double CUBIC_ARC_FACTOR = 0.5522847498307936;
        const double control_offset = radius * CUBIC_ARC_FACTOR;
        return {
            make_move_to(center_x + radius, center_y),
            make_cubic_bezier_to(
                Point{center_x + radius, center_y + control_offset},
                Point{center_x + control_offset, center_y + radius},
                Point{center_x, center_y + radius}),
            make_cubic_bezier_to(
                Point{center_x - control_offset, center_y + radius},
                Point{center_x - radius, center_y + control_offset},
                Point{center_x - radius, center_y}),
            make_cubic_bezier_to(
                Point{center_x - radius, center_y - control_offset},
                Point{center_x - control_offset, center_y - radius},
                Point{center_x, center_y - radius}),
            make_cubic_bezier_to(
                Point{center_x + control_offset, center_y - radius},
                Point{center_x + radius, center_y - control_offset},
                Point{center_x + radius, center_y}),
            make_close_path(),
        };
    }

    static std::uint8_t get_component(
        const RgbaImage& image,
        int x,
        int y,
        std::size_t component_offset) {
        const std::size_t pixel_index =
            static_cast<std::size_t>(y) * static_cast<std::size_t>(image.width) +
            static_cast<std::size_t>(x);
        return image.pixels[pixel_index * 4U + component_offset];
    }

    static std::uint8_t get_alpha(const RgbaImage& image, int x, int y) {
        return get_component(image, x, y, 3U);
    }

    static double calculate_covered_area(const RgbaImage& image) {
        double covered_area = 0.0;
        for (std::size_t index = 3U; index < image.pixels.size(); index += 4U) {
            covered_area += static_cast<double>(image.pixels[index]) / 255.0;
        }
        return covered_area;
    }
};

TEST_F(SvgRasterizerTest, SameAspectRatioScalesCoordinates) {
    const DrawShape draw_shape = make_shape(
        make_closed_rectangle(1.0, 1.0, 1.0, 1.0),
        true,
        false);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(10.0, 10.0),
        20,
        20,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(get_alpha(*result, 2, 2), 255U);
    EXPECT_EQ(get_alpha(*result, 3, 3), 255U);
    EXPECT_EQ(get_alpha(*result, 1, 2), 0U);
}

TEST_F(SvgRasterizerTest, MeetCentersContentWithHorizontalMargin) {
    const DrawShape draw_shape = make_shape(
        make_closed_rectangle(0.0, 0.0, 10.0, 10.0),
        true,
        false);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(10.0, 10.0),
        20,
        10,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(get_alpha(*result, 4, 5), 0U);
    EXPECT_EQ(get_alpha(*result, 5, 5), 255U);
    EXPECT_EQ(get_alpha(*result, 14, 5), 255U);
    EXPECT_EQ(get_alpha(*result, 15, 5), 0U);
}

TEST_F(SvgRasterizerTest, StrokeCenterPixelIsOpaque) {
    const DrawShape draw_shape = make_shape(
        {make_move_to(1.0, 5.25), make_line_to(19.0, 5.25)},
        false,
        true,
        2.0);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(20.0, 10.0),
        20,
        10,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(get_alpha(*result, 10, 5), 255U);
    EXPECT_GT(get_alpha(*result, 10, 4), 0U);
    EXPECT_LT(get_alpha(*result, 10, 4), 255U);
    EXPECT_EQ(get_alpha(*result, 10, 2), 0U);
}

TEST_F(SvgRasterizerTest, RoundCapCoversSemicircleBeyondEndpoint) {
    const DrawShape draw_shape = make_shape(
        {make_move_to(8.0, 8.0), make_line_to(16.0, 8.0)},
        false,
        true,
        4.0);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(24.0, 24.0),
        24,
        24,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(get_alpha(*result, 7, 8), 255U);
    EXPECT_GT(get_alpha(*result, 6, 8), 0U);
    EXPECT_EQ(get_alpha(*result, 5, 8), 0U);
}

TEST_F(SvgRasterizerTest, ZeroLengthSubpathDrawsDot) {
    const DrawShape draw_shape = make_shape(
        {make_move_to(8.0, 8.0), make_line_to(8.0, 8.0)},
        false,
        true,
        4.0);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(24.0, 24.0),
        24,
        24,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(get_alpha(*result, 7, 8), 255U);
    EXPECT_EQ(get_alpha(*result, 5, 8), 0U);
}

TEST_F(SvgRasterizerTest, MoveOnlySubpathDrawsNothing) {
    const DrawShape draw_shape = make_shape(
        {make_move_to(8.0, 8.0)},
        false,
        true,
        4.0);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(24.0, 24.0),
        24,
        24,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(result.has_value());
    const bool is_fully_transparent = std::all_of(
        result->pixels.begin(),
        result->pixels.end(),
        [](std::uint8_t component) {
            return component == 0U;
        });
    EXPECT_TRUE(is_fully_transparent);
}

TEST_F(SvgRasterizerTest, StrokeJoinDoesNotAccumulateAlpha) {
    const DrawShape draw_shape = make_shape(
        {
            make_move_to(4.0, 8.0),
            make_line_to(12.0, 8.0),
            make_line_to(12.0, 16.0),
        },
        false,
        true,
        4.0);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(24.0, 24.0),
        24,
        24,
        RgbaColor{10U, 20U, 30U, 128U});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(get_alpha(*result, 12, 8), 128U);
}

TEST_F(SvgRasterizerTest, NonzeroFillUsesWindingDirection) {
    std::vector<PathInstruction> same_direction = make_closed_rectangle(2.0, 2.0, 12.0, 12.0);
    const std::vector<PathInstruction> inner_same_direction =
        make_closed_rectangle(5.0, 5.0, 6.0, 6.0);
    same_direction.insert(
        same_direction.end(),
        inner_same_direction.begin(),
        inner_same_direction.end());

    std::vector<PathInstruction> opposite_direction = make_closed_rectangle(2.0, 2.0, 12.0, 12.0);
    opposite_direction.insert(
        opposite_direction.end(),
        {
            make_move_to(5.0, 5.0),
            make_line_to(5.0, 11.0),
            make_line_to(11.0, 11.0),
            make_line_to(11.0, 5.0),
            make_close_path(),
        });

    const auto same_direction_result = SvgRasterizer::rasterize(
        {make_shape(std::move(same_direction), true, false)},
        make_view_box(16.0, 16.0),
        16,
        16,
        RgbaColor{10U, 20U, 30U, 255U});
    const auto opposite_direction_result = SvgRasterizer::rasterize(
        {make_shape(std::move(opposite_direction), true, false)},
        make_view_box(16.0, 16.0),
        16,
        16,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(same_direction_result.has_value());
    ASSERT_TRUE(opposite_direction_result.has_value());
    EXPECT_EQ(get_alpha(*same_direction_result, 7, 7), 255U);
    EXPECT_EQ(get_alpha(*opposite_direction_result, 7, 7), 0U);
}

TEST_F(SvgRasterizerTest, StrokeIsCompositedAfterFill) {
    const DrawShape draw_shape = make_shape(
        make_closed_rectangle(4.0, 4.0, 8.0, 8.0),
        true,
        true,
        4.0);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(16.0, 16.0),
        16,
        16,
        RgbaColor{10U, 20U, 30U, 128U});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(get_alpha(*result, 4, 6), 192U);
}

TEST_F(SvgRasterizerTest, FilledSquareCoverageMatchesArea) {
    constexpr double EXPECTED_AREA = 64.0;
    constexpr double AREA_TOLERANCE = 0.01;
    const DrawShape draw_shape = make_shape(
        make_closed_rectangle(0.0, 0.0, 8.0, 8.0),
        true,
        false);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(8.0, 8.0),
        8,
        8,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(calculate_covered_area(*result), EXPECTED_AREA, AREA_TOLERANCE);
}

TEST_F(SvgRasterizerTest, CubicCircleCoverageMatchesArea) {
    constexpr double RADIUS = 30.0;
    constexpr double EXPECTED_AREA = 3.14159265358979323846 * RADIUS * RADIUS;
    constexpr double AREA_TOLERANCE = 24.0;
    const DrawShape draw_shape = make_shape(
        make_cubic_circle(5.0, 5.0, 3.0),
        true,
        false);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(10.0, 10.0),
        100,
        100,
        RgbaColor{10U, 20U, 30U, 255U});

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(calculate_covered_area(*result), EXPECTED_AREA, AREA_TOLERANCE);
}

TEST_F(SvgRasterizerTest, RgbaImageUsesTightlyPackedRows) {
    const DrawShape draw_shape = make_shape(
        make_closed_rectangle(0.0, 0.0, 1.0, 1.0),
        true,
        false);

    const auto result = SvgRasterizer::rasterize(
        {draw_shape},
        make_view_box(2.0, 1.0),
        2,
        1,
        RgbaColor{10U, 20U, 30U, 40U});

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->width, 2);
    EXPECT_EQ(result->height, 1);
    EXPECT_EQ(
        result->pixels,
        (std::vector<std::uint8_t>{
            10U,
            20U,
            30U,
            40U,
            0U,
            0U,
            0U,
            0U,
        }));
}
