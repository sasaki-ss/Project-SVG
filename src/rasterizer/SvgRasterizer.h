#ifndef PROJECT_SVG_RASTERIZER_SVG_RASTERIZER_H_
#define PROJECT_SVG_RASTERIZER_SVG_RASTERIZER_H_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "RgbaImage.h"
#include "draw/DrawShape.h"
#include "interpreter/InterpretedSvg.h"

namespace svg{
namespace rasterizer{

class SvgRasterizer {
public:
    static auto rasterize(
        const std::vector<draw::DrawShape>& draw_shapes,
        const interpreter::SvgViewBox& view_box,
        int output_width,
        int output_height,
        RgbaColor color)
        -> std::optional<RgbaImage>;

private:
    struct RasterPoint {
        double x;
        double y;
    };

    struct Subpath {
        std::vector<RasterPoint> points;
        bool is_closed;
    };

    struct Segment {
        RasterPoint start;
        RasterPoint end;
    };

    struct PixelBounds {
        int min_x;
        int min_y;
        int max_x;
        int max_y;
    };

    struct Transform {
        double scale;
        double offset_x;
        double offset_y;
    };

    using SampleCoverage = std::uint16_t;

    static constexpr int SUPERSAMPLE_COUNT = 4;
    static constexpr int SAMPLE_COUNT_PER_PIXEL = SUPERSAMPLE_COUNT * SUPERSAMPLE_COUNT;
    static_assert(
        SAMPLE_COUNT_PER_PIXEL <= static_cast<int>(sizeof(SampleCoverage) * 8),
        "each sample of a pixel needs its own coverage bit");
    static constexpr int MAX_CUBIC_SUBDIVISION_DEPTH = 16;
    static constexpr double CUBIC_FLATTEN_TOLERANCE = 0.25;
    static constexpr double GEOMETRY_EPSILON = 1e-9;
    static constexpr double CULLING_MARGIN = 1.0;

    SvgRasterizer() = delete;

    static auto make_transform(
        const interpreter::SvgViewBox& view_box,
        int output_width,
        int output_height)
        -> std::optional<Transform>;
    static auto make_subpaths(
        const std::vector<interpreter::PathInstruction>& path_instructions,
        const Transform& transform)
        -> std::optional<std::vector<Subpath>>;
    static auto transform_point(
        const interpreter::Point& point,
        const Transform& transform)
        -> std::optional<RasterPoint>;
    static void append_cubic_bezier(
        Subpath& subpath,
        const RasterPoint& start_point,
        const RasterPoint& control_point1,
        const RasterPoint& control_point2,
        const RasterPoint& end_point,
        int depth);
    static bool is_cubic_bezier_flat_enough(
        const RasterPoint& start_point,
        const RasterPoint& control_point1,
        const RasterPoint& control_point2,
        const RasterPoint& end_point);
    static double calculate_point_line_distance(
        const RasterPoint& point,
        const RasterPoint& line_start,
        const RasterPoint& line_end);
    static RasterPoint calculate_midpoint(
        const RasterPoint& first_point,
        const RasterPoint& second_point);
    static auto make_stroke_segments(const std::vector<Subpath>& subpaths)
        -> std::vector<Segment>;
    static auto make_fill_edges(const std::vector<Subpath>& subpaths)
        -> std::vector<std::vector<Segment>>;
    static auto calculate_pixel_bounds(
        const std::vector<Subpath>& subpaths,
        double reach,
        int output_width,
        int output_height)
        -> std::optional<PixelBounds>;
    static bool is_segment_near_band(
        double first_coordinate,
        double second_coordinate,
        double band_start,
        double band_end,
        double reach);
    static bool is_point_in_fill(
        const RasterPoint& sample_point,
        const std::vector<std::vector<Segment>>& edges_by_subpath);
    static int calculate_winding_number(
        const RasterPoint& sample_point,
        const std::vector<Segment>& edges);
    static bool is_point_on_line_segment(
        const RasterPoint& sample_point,
        const RasterPoint& segment_start,
        const RasterPoint& segment_end);
    static bool is_point_in_stroke(
        const RasterPoint& sample_point,
        const std::vector<Segment>& segments,
        double radius);
    static bool is_point_in_round_segment(
        const RasterPoint& sample_point,
        const RasterPoint& segment_start,
        const RasterPoint& segment_end,
        double radius);
    static auto make_coverage_buffer(int output_width, int output_height)
        -> std::optional<std::vector<SampleCoverage>>;
    static void rasterize_draw_shape(
        std::vector<SampleCoverage>& coverages,
        int output_width,
        int output_height,
        const draw::DrawShape& draw_shape,
        const std::vector<Subpath>& subpaths,
        double stroke_width);
    static int count_covered_samples(SampleCoverage coverage);
    static void apply_group_opacity(
        RgbaImage& image,
        std::uint8_t group_opacity);
    static auto make_output_image(
        int output_width,
        int output_height,
        const std::vector<SampleCoverage>& coverages,
        const RgbaColor& color)
        -> std::optional<RgbaImage>;
    static std::uint8_t to_byte(double value);
};

}
}

#endif  // PROJECT_SVG_RASTERIZER_SVG_RASTERIZER_H_
