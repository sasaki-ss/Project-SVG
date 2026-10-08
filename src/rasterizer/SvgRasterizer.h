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

    struct SubpathBuildState {
        std::vector<Subpath> subpaths;
        std::optional<Subpath> current_subpath;
        std::optional<RasterPoint> current_point;
        std::optional<RasterPoint> subpath_start;
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

    enum class BandAxis {
        Horizontal,
        Vertical,
    };

    struct ShapeGeometry {
        std::vector<Segment> stroke_segments;
        std::vector<std::vector<Segment>> fill_edges;
        double stroke_radius;
        double stroke_reach;
    };

    struct RowCandidates {
        std::vector<Segment> stroke_segments;
        std::vector<std::vector<Segment>> fill_edges;
        bool has_fill_edges;
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
    static constexpr SampleCoverage FULL_COVERAGE =
        static_cast<SampleCoverage>((1U << SAMPLE_COUNT_PER_PIXEL) - 1U);
    // Upper bound of the distance from a pixel center to its farthest sample, so that a pixel
    // can be decided as a whole only when the per-sample test is guaranteed to agree.
    static constexpr double SAMPLE_REACH_FROM_CENTER = 0.5304;
    static constexpr double WHOLE_PIXEL_DECISION_MARGIN = 1e-6;
    static constexpr double FARTHEST_SAMPLE_OFFSET = 0.5 - 0.5 / SUPERSAMPLE_COUNT;
    static_assert(
        SAMPLE_REACH_FROM_CENTER * SAMPLE_REACH_FROM_CENTER >=
            2.0 * FARTHEST_SAMPLE_OFFSET * FARTHEST_SAMPLE_OFFSET,
        "SAMPLE_REACH_FROM_CENTER must cover the farthest sample of a pixel");

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
    static bool apply_path_instruction(
        SubpathBuildState& state,
        const interpreter::PathInstruction& instruction,
        const Transform& transform);
    static bool apply_move_to(
        SubpathBuildState& state,
        const interpreter::PathInstruction& instruction,
        const Transform& transform);
    static bool apply_line_to(
        SubpathBuildState& state,
        const interpreter::PathInstruction& instruction,
        const Transform& transform);
    static bool apply_cubic_bezier_to(
        SubpathBuildState& state,
        const interpreter::PathInstruction& instruction,
        const Transform& transform);
    static bool apply_close_path(
        SubpathBuildState& state,
        const interpreter::PathInstruction& instruction);
    static void begin_subpath_at_current_point(SubpathBuildState& state);
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
    static double calculate_nearest_segment_distance(
        const RasterPoint& point,
        const std::vector<Segment>& segments);
    static double calculate_point_segment_distance(
        const RasterPoint& point,
        const RasterPoint& segment_start,
        const RasterPoint& segment_end);
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
    static void collect_row_candidates(
        const ShapeGeometry& geometry,
        double row_start,
        RowCandidates& row_candidates);
    static void collect_segments_near_band(
        const std::vector<Segment>& segments,
        BandAxis band_axis,
        double band_start,
        double reach,
        std::vector<Segment>& near_segments);
    static void rasterize_pixel(
        SampleCoverage& coverage,
        const ShapeGeometry& geometry,
        const RowCandidates& row_candidates,
        const RasterPoint& pixel_origin,
        std::vector<Segment>& pixel_stroke_segments);
    static void cover_samples(
        SampleCoverage& coverage,
        const ShapeGeometry& geometry,
        const RowCandidates& row_candidates,
        const RasterPoint& pixel_origin,
        const std::vector<Segment>& pixel_stroke_segments);
    static RasterPoint make_sample_point(const RasterPoint& pixel_origin, int sample_index);
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
