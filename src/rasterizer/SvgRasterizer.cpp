#include "SvgRasterizer.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace svg{
namespace rasterizer{

auto SvgRasterizer::rasterize(
    const std::vector<draw::DrawShape>& draw_shapes,
    const interpreter::SvgViewBox& view_box,
    int output_width,
    int output_height,
    RgbaColor color)
    -> std::optional<RgbaImage> {
    const auto transform = make_transform(view_box, output_width, output_height);
    if (!transform.has_value()) {
        return std::nullopt;
    }

    auto coverages = make_coverage_buffer(output_width, output_height);
    if (!coverages.has_value()) {
        return std::nullopt;
    }

    for (const draw::DrawShape& draw_shape : draw_shapes) {
        if (!std::isfinite(draw_shape.stroke_width) || draw_shape.stroke_width < 0.0) {
            return std::nullopt;
        }

        const auto subpaths = make_subpaths(draw_shape.path_instructions, *transform);
        if (!subpaths.has_value()) {
            return std::nullopt;
        }

        rasterize_draw_shape(
            *coverages,
            output_width,
            output_height,
            draw_shape,
            *subpaths,
            draw_shape.stroke_width * transform->scale);
    }

    auto output_image = make_output_image(
        output_width,
        output_height,
        *coverages,
        color);
    if (!output_image.has_value()) {
        return std::nullopt;
    }

    apply_group_opacity(*output_image, color.alpha);
    return output_image;
}

auto SvgRasterizer::make_transform(
    const interpreter::SvgViewBox& view_box,
    int output_width,
    int output_height)
    -> std::optional<Transform> {
    if (output_width <= 0 || output_height <= 0) {
        return std::nullopt;
    }
    if (!std::isfinite(view_box.min_x) || !std::isfinite(view_box.min_y)) {
        return std::nullopt;
    }
    if (!std::isfinite(view_box.width) || !std::isfinite(view_box.height)) {
        return std::nullopt;
    }
    if (view_box.width <= 0.0 || view_box.height <= 0.0) {
        return std::nullopt;
    }

    const double width_scale = static_cast<double>(output_width) / view_box.width;
    const double height_scale = static_cast<double>(output_height) / view_box.height;
    const double scale = std::min(width_scale, height_scale);
    if (!std::isfinite(scale) || scale <= 0.0) {
        return std::nullopt;
    }

    const double content_width = view_box.width * scale;
    const double content_height = view_box.height * scale;
    Transform transform;
    transform.scale = scale;
    const double horizontal_margin =
        (static_cast<double>(output_width) - content_width) / 2.0;
    const double vertical_margin =
        (static_cast<double>(output_height) - content_height) / 2.0;
    transform.offset_x = horizontal_margin - view_box.min_x * scale;
    transform.offset_y = vertical_margin - view_box.min_y * scale;
    return transform;
}

auto SvgRasterizer::make_subpaths(
    const std::vector<interpreter::PathInstruction>& path_instructions,
    const Transform& transform)
    -> std::optional<std::vector<Subpath>> {
    std::vector<Subpath> subpaths;
    std::optional<Subpath> current_subpath;
    std::optional<RasterPoint> current_point;
    std::optional<RasterPoint> subpath_start;

    for (const interpreter::PathInstruction& instruction : path_instructions) {
        if (instruction.type == interpreter::PathInstructionType::MoveTo) {
            if (instruction.points.size() != 1U) {
                return std::nullopt;
            }
            if (current_subpath.has_value()) {
                subpaths.push_back(std::move(*current_subpath));
            }

            const auto transformed_point = transform_point(instruction.points[0], transform);
            if (!transformed_point.has_value()) {
                return std::nullopt;
            }

            current_subpath = Subpath{{*transformed_point}, false};
            current_point = *transformed_point;
            subpath_start = *transformed_point;
            continue;
        }

        if (instruction.type == interpreter::PathInstructionType::LineTo) {
            if (instruction.points.size() != 1U || !current_point.has_value()) {
                return std::nullopt;
            }

            const auto transformed_point = transform_point(instruction.points[0], transform);
            if (!transformed_point.has_value()) {
                return std::nullopt;
            }
            if (!current_subpath.has_value()) {
                current_subpath = Subpath{{*current_point}, false};
                subpath_start = *current_point;
            }

            current_subpath->points.push_back(*transformed_point);
            current_point = *transformed_point;
            continue;
        }

        if (instruction.type == interpreter::PathInstructionType::CubicBezierTo) {
            if (instruction.points.size() != 3U || !current_point.has_value()) {
                return std::nullopt;
            }

            const auto control_point1 = transform_point(instruction.points[0], transform);
            const auto control_point2 = transform_point(instruction.points[1], transform);
            const auto end_point = transform_point(instruction.points[2], transform);
            if (!control_point1.has_value() ||
                !control_point2.has_value() ||
                !end_point.has_value()) {
                return std::nullopt;
            }
            if (!current_subpath.has_value()) {
                current_subpath = Subpath{{*current_point}, false};
                subpath_start = *current_point;
            }

            append_cubic_bezier(
                *current_subpath,
                *current_point,
                *control_point1,
                *control_point2,
                *end_point,
                0);
            current_point = *end_point;
            continue;
        }

        if (instruction.type == interpreter::PathInstructionType::ClosePath) {
            if (!instruction.points.empty() || !current_subpath.has_value()) {
                return std::nullopt;
            }

            current_subpath->is_closed = true;
            subpaths.push_back(std::move(*current_subpath));
            current_subpath.reset();
            current_point = subpath_start;
            continue;
        }

        return std::nullopt;
    }

    if (current_subpath.has_value()) {
        subpaths.push_back(std::move(*current_subpath));
    }
    return subpaths;
}

auto SvgRasterizer::transform_point(
    const interpreter::Point& point,
    const Transform& transform)
    -> std::optional<RasterPoint> {
    if (!std::isfinite(point.x) || !std::isfinite(point.y)) {
        return std::nullopt;
    }

    const double transformed_x = point.x * transform.scale + transform.offset_x;
    const double transformed_y = point.y * transform.scale + transform.offset_y;
    if (!std::isfinite(transformed_x) || !std::isfinite(transformed_y)) {
        return std::nullopt;
    }
    return RasterPoint{transformed_x, transformed_y};
}

void SvgRasterizer::append_cubic_bezier(
    Subpath& subpath,
    const RasterPoint& start_point,
    const RasterPoint& control_point1,
    const RasterPoint& control_point2,
    const RasterPoint& end_point,
    int depth) {
    if (depth >= MAX_CUBIC_SUBDIVISION_DEPTH ||
        is_cubic_bezier_flat_enough(
            start_point,
            control_point1,
            control_point2,
            end_point)) {
        subpath.points.push_back(end_point);
        return;
    }

    const RasterPoint start_control1 = calculate_midpoint(start_point, control_point1);
    const RasterPoint controls = calculate_midpoint(control_point1, control_point2);
    const RasterPoint control2_end = calculate_midpoint(control_point2, end_point);
    const RasterPoint first_middle = calculate_midpoint(start_control1, controls);
    const RasterPoint second_middle = calculate_midpoint(controls, control2_end);
    const RasterPoint split_point = calculate_midpoint(first_middle, second_middle);

    append_cubic_bezier(
        subpath,
        start_point,
        start_control1,
        first_middle,
        split_point,
        depth + 1);
    append_cubic_bezier(
        subpath,
        split_point,
        second_middle,
        control2_end,
        end_point,
        depth + 1);
}

bool SvgRasterizer::is_cubic_bezier_flat_enough(
    const RasterPoint& start_point,
    const RasterPoint& control_point1,
    const RasterPoint& control_point2,
    const RasterPoint& end_point) {
    const double first_distance = calculate_point_line_distance(
        control_point1,
        start_point,
        end_point);
    const double second_distance = calculate_point_line_distance(
        control_point2,
        start_point,
        end_point);
    return std::max(first_distance, second_distance) <= CUBIC_FLATTEN_TOLERANCE;
}

double SvgRasterizer::calculate_point_line_distance(
    const RasterPoint& point,
    const RasterPoint& line_start,
    const RasterPoint& line_end) {
    const double delta_x = line_end.x - line_start.x;
    const double delta_y = line_end.y - line_start.y;
    const double length_squared = delta_x * delta_x + delta_y * delta_y;
    if (length_squared <= GEOMETRY_EPSILON) {
        const double point_delta_x = point.x - line_start.x;
        const double point_delta_y = point.y - line_start.y;
        return std::sqrt(point_delta_x * point_delta_x + point_delta_y * point_delta_y);
    }

    const double numerator = std::abs(
        delta_y * point.x -
        delta_x * point.y +
        line_end.x * line_start.y -
        line_end.y * line_start.x);
    return numerator / std::sqrt(length_squared);
}

SvgRasterizer::RasterPoint SvgRasterizer::calculate_midpoint(
    const RasterPoint& first_point,
    const RasterPoint& second_point) {
    return RasterPoint{
        (first_point.x + second_point.x) / 2.0,
        (first_point.y + second_point.y) / 2.0,
    };
}

auto SvgRasterizer::make_stroke_segments(const std::vector<Subpath>& subpaths)
    -> std::vector<Segment> {
    std::vector<Segment> segments;
    for (const Subpath& subpath : subpaths) {
        if (subpath.points.size() < 2U) {
            continue;
        }

        for (std::size_t index = 1; index < subpath.points.size(); ++index) {
            segments.push_back(Segment{subpath.points[index - 1U], subpath.points[index]});
        }
        if (subpath.is_closed) {
            segments.push_back(Segment{subpath.points.back(), subpath.points.front()});
        }
    }
    return segments;
}

auto SvgRasterizer::make_fill_edges(const std::vector<Subpath>& subpaths)
    -> std::vector<std::vector<Segment>> {
    std::vector<std::vector<Segment>> edges_by_subpath;
    for (const Subpath& subpath : subpaths) {
        if (subpath.points.size() < 2U) {
            continue;
        }

        std::vector<Segment> edges;
        for (std::size_t index = 0; index < subpath.points.size(); ++index) {
            edges.push_back(Segment{
                subpath.points[index],
                subpath.points[(index + 1U) % subpath.points.size()],
            });
        }
        edges_by_subpath.push_back(std::move(edges));
    }
    return edges_by_subpath;
}

auto SvgRasterizer::calculate_pixel_bounds(
    const std::vector<Subpath>& subpaths,
    double reach,
    int output_width,
    int output_height)
    -> std::optional<PixelBounds> {
    double min_x = std::numeric_limits<double>::infinity();
    double min_y = std::numeric_limits<double>::infinity();
    double max_x = -std::numeric_limits<double>::infinity();
    double max_y = -std::numeric_limits<double>::infinity();
    for (const Subpath& subpath : subpaths) {
        for (const RasterPoint& point : subpath.points) {
            min_x = std::min(min_x, point.x);
            min_y = std::min(min_y, point.y);
            max_x = std::max(max_x, point.x);
            max_y = std::max(max_y, point.y);
        }
    }
    if (min_x > max_x || min_y > max_y) {
        return std::nullopt;
    }

    const double last_x = static_cast<double>(output_width - 1);
    const double last_y = static_cast<double>(output_height - 1);
    PixelBounds bounds;
    bounds.min_x = static_cast<int>(std::clamp(std::floor(min_x - reach), 0.0, last_x));
    bounds.min_y = static_cast<int>(std::clamp(std::floor(min_y - reach), 0.0, last_y));
    bounds.max_x = static_cast<int>(std::clamp(std::floor(max_x + reach), 0.0, last_x));
    bounds.max_y = static_cast<int>(std::clamp(std::floor(max_y + reach), 0.0, last_y));
    if (max_x + reach < 0.0 || max_y + reach < 0.0) {
        return std::nullopt;
    }
    if (min_x - reach > last_x + 1.0 || min_y - reach > last_y + 1.0) {
        return std::nullopt;
    }
    return bounds;
}

bool SvgRasterizer::is_segment_near_band(
    double first_coordinate,
    double second_coordinate,
    double band_start,
    double band_end,
    double reach) {
    const double segment_min = std::min(first_coordinate, second_coordinate);
    const double segment_max = std::max(first_coordinate, second_coordinate);
    return segment_max + reach >= band_start && segment_min - reach <= band_end;
}

bool SvgRasterizer::is_point_in_fill(
    const RasterPoint& sample_point,
    const std::vector<std::vector<Segment>>& edges_by_subpath) {
    int winding_number = 0;
    for (const std::vector<Segment>& edges : edges_by_subpath) {
        winding_number += calculate_winding_number(sample_point, edges);
    }
    return winding_number != 0;
}

int SvgRasterizer::calculate_winding_number(
    const RasterPoint& sample_point,
    const std::vector<Segment>& edges) {
    int winding_number = 0;
    for (const Segment& edge : edges) {
        const RasterPoint& segment_start = edge.start;
        const RasterPoint& segment_end = edge.end;
        if (is_point_on_line_segment(sample_point, segment_start, segment_end)) {
            return 1;
        }

        const double cross_product =
            (segment_end.x - segment_start.x) * (sample_point.y - segment_start.y) -
            (sample_point.x - segment_start.x) * (segment_end.y - segment_start.y);
        const bool crosses_upward =
            segment_start.y <= sample_point.y && segment_end.y > sample_point.y;
        const bool crosses_downward =
            segment_start.y > sample_point.y && segment_end.y <= sample_point.y;
        if (crosses_upward && cross_product > 0.0) {
            ++winding_number;
        }
        if (crosses_downward && cross_product < 0.0) {
            --winding_number;
        }
    }

    return winding_number;
}

bool SvgRasterizer::is_point_on_line_segment(
    const RasterPoint& sample_point,
    const RasterPoint& segment_start,
    const RasterPoint& segment_end) {
    const double segment_delta_x = segment_end.x - segment_start.x;
    const double segment_delta_y = segment_end.y - segment_start.y;
    const double point_delta_x = sample_point.x - segment_start.x;
    const double point_delta_y = sample_point.y - segment_start.y;
    const double length_squared =
        segment_delta_x * segment_delta_x + segment_delta_y * segment_delta_y;
    if (length_squared <= GEOMETRY_EPSILON) {
        return point_delta_x * point_delta_x + point_delta_y * point_delta_y <=
            GEOMETRY_EPSILON * GEOMETRY_EPSILON;
    }

    const double cross_product =
        segment_delta_x * point_delta_y - segment_delta_y * point_delta_x;
    if (std::abs(cross_product) > GEOMETRY_EPSILON) {
        return false;
    }

    const double dot_product =
        point_delta_x * segment_delta_x + point_delta_y * segment_delta_y;
    if (dot_product < -GEOMETRY_EPSILON) {
        return false;
    }

    return dot_product <= length_squared + GEOMETRY_EPSILON;
}

bool SvgRasterizer::is_point_in_stroke(
    const RasterPoint& sample_point,
    const std::vector<Segment>& segments,
    double radius) {
    for (const Segment& segment : segments) {
        if (is_point_in_round_segment(sample_point, segment.start, segment.end, radius)) {
            return true;
        }
    }
    return false;
}

bool SvgRasterizer::is_point_in_round_segment(
    const RasterPoint& sample_point,
    const RasterPoint& segment_start,
    const RasterPoint& segment_end,
    double radius) {
    const double delta_x = segment_end.x - segment_start.x;
    const double delta_y = segment_end.y - segment_start.y;
    const double length_squared = delta_x * delta_x + delta_y * delta_y;
    if (length_squared <= GEOMETRY_EPSILON) {
        const double point_delta_x = sample_point.x - segment_start.x;
        const double point_delta_y = sample_point.y - segment_start.y;
        const double distance_squared =
            point_delta_x * point_delta_x + point_delta_y * point_delta_y;
        return distance_squared <= radius * radius;
    }

    const double projection =
        ((sample_point.x - segment_start.x) * delta_x +
         (sample_point.y - segment_start.y) * delta_y) /
        length_squared;
    const double clamped_projection = std::clamp(projection, 0.0, 1.0);
    const double closest_x = segment_start.x + clamped_projection * delta_x;
    const double closest_y = segment_start.y + clamped_projection * delta_y;
    const double point_delta_x = sample_point.x - closest_x;
    const double point_delta_y = sample_point.y - closest_y;
    const double distance_squared =
        point_delta_x * point_delta_x + point_delta_y * point_delta_y;
    return distance_squared <= radius * radius;
}

auto SvgRasterizer::make_coverage_buffer(int output_width, int output_height)
    -> std::optional<std::vector<SampleCoverage>> {
    if (output_width <= 0 || output_height <= 0) {
        return std::nullopt;
    }

    const std::size_t width = static_cast<std::size_t>(output_width);
    const std::size_t height = static_cast<std::size_t>(output_height);
    if (width > std::numeric_limits<std::size_t>::max() / height) {
        return std::nullopt;
    }

    try {
        return std::vector<SampleCoverage>(width * height, 0U);
    } catch (...) {
        return std::nullopt;
    }
}

void SvgRasterizer::rasterize_draw_shape(
    std::vector<SampleCoverage>& coverages,
    int output_width,
    int output_height,
    const draw::DrawShape& draw_shape,
    const std::vector<Subpath>& subpaths,
    double stroke_width) {
    const bool has_visible_stroke = draw_shape.has_stroke && stroke_width > 0.0;
    if (!draw_shape.has_fill && !has_visible_stroke) {
        return;
    }

    const double radius = stroke_width / 2.0;
    const double stroke_reach = radius + CULLING_MARGIN;
    std::vector<Segment> stroke_segments;
    if (has_visible_stroke) {
        stroke_segments = make_stroke_segments(subpaths);
    }
    std::vector<std::vector<Segment>> fill_edges;
    if (draw_shape.has_fill) {
        fill_edges = make_fill_edges(subpaths);
    }

    const double bounds_reach = has_visible_stroke ? stroke_reach : CULLING_MARGIN;
    const auto bounds = calculate_pixel_bounds(
        subpaths,
        bounds_reach,
        output_width,
        output_height);
    if (!bounds.has_value()) {
        return;
    }

    std::vector<Segment> row_stroke_segments;
    std::vector<Segment> pixel_stroke_segments;
    std::vector<std::vector<Segment>> row_fill_edges(fill_edges.size());
    for (int pixel_y = bounds->min_y; pixel_y <= bounds->max_y; ++pixel_y) {
        const double row_start = static_cast<double>(pixel_y);
        const double row_end = row_start + 1.0;

        row_stroke_segments.clear();
        for (const Segment& segment : stroke_segments) {
            if (is_segment_near_band(
                    segment.start.y, segment.end.y, row_start, row_end, stroke_reach)) {
                row_stroke_segments.push_back(segment);
            }
        }

        bool has_row_fill_edges = false;
        for (std::size_t subpath_index = 0; subpath_index < fill_edges.size(); ++subpath_index) {
            std::vector<Segment>& row_edges = row_fill_edges[subpath_index];
            row_edges.clear();
            for (const Segment& edge : fill_edges[subpath_index]) {
                if (is_segment_near_band(
                        edge.start.y, edge.end.y, row_start, row_end, CULLING_MARGIN)) {
                    row_edges.push_back(edge);
                }
            }
            has_row_fill_edges = has_row_fill_edges || !row_edges.empty();
        }
        if (row_stroke_segments.empty() && !has_row_fill_edges) {
            continue;
        }

        for (int pixel_x = bounds->min_x; pixel_x <= bounds->max_x; ++pixel_x) {
            const double column_start = static_cast<double>(pixel_x);
            const double column_end = column_start + 1.0;

            pixel_stroke_segments.clear();
            for (const Segment& segment : row_stroke_segments) {
                if (is_segment_near_band(
                        segment.start.x, segment.end.x, column_start, column_end, stroke_reach)) {
                    pixel_stroke_segments.push_back(segment);
                }
            }
            if (pixel_stroke_segments.empty() && !has_row_fill_edges) {
                continue;
            }

            const std::size_t pixel_index =
                static_cast<std::size_t>(pixel_y) * static_cast<std::size_t>(output_width) +
                static_cast<std::size_t>(pixel_x);
            SampleCoverage& coverage = coverages[pixel_index];
            for (int sample_y = 0; sample_y < SUPERSAMPLE_COUNT; ++sample_y) {
                for (int sample_x = 0; sample_x < SUPERSAMPLE_COUNT; ++sample_x) {
                    const SampleCoverage sample_bit = static_cast<SampleCoverage>(
                        1U << (sample_y * SUPERSAMPLE_COUNT + sample_x));
                    if ((coverage & sample_bit) != 0U) {
                        continue;
                    }

                    const double offset_x =
                        (static_cast<double>(sample_x) + 0.5) /
                        static_cast<double>(SUPERSAMPLE_COUNT);
                    const double offset_y =
                        (static_cast<double>(sample_y) + 0.5) /
                        static_cast<double>(SUPERSAMPLE_COUNT);
                    const RasterPoint sample_point{
                        column_start + offset_x,
                        row_start + offset_y,
                    };
                    const bool is_filled =
                        has_row_fill_edges && is_point_in_fill(sample_point, row_fill_edges);
                    const bool is_stroked =
                        !is_filled &&
                        !pixel_stroke_segments.empty() &&
                        is_point_in_stroke(sample_point, pixel_stroke_segments, radius);
                    if (is_filled || is_stroked) {
                        coverage = static_cast<SampleCoverage>(coverage | sample_bit);
                    }
                }
            }
        }
    }
}

int SvgRasterizer::count_covered_samples(SampleCoverage coverage) {
    int covered_count = 0;
    for (SampleCoverage remaining = coverage;
         remaining != 0U;
         remaining = static_cast<SampleCoverage>(remaining & (remaining - 1U))) {
        ++covered_count;
    }
    return covered_count;
}

void SvgRasterizer::apply_group_opacity(
    RgbaImage& image,
    std::uint8_t group_opacity) {
    const double opacity = static_cast<double>(group_opacity) / 255.0;
    for (std::size_t byte_index = 0U;
         byte_index < image.pixels.size();
         byte_index += 4U) {
        const double alpha = static_cast<double>(image.pixels[byte_index + 3U]);
        const std::uint8_t scaled_alpha = to_byte(alpha * opacity);
        image.pixels[byte_index + 3U] = scaled_alpha;
        if (scaled_alpha == 0U) {
            image.pixels[byte_index] = 0U;
            image.pixels[byte_index + 1U] = 0U;
            image.pixels[byte_index + 2U] = 0U;
        }
    }
}

auto SvgRasterizer::make_output_image(
    int output_width,
    int output_height,
    const std::vector<SampleCoverage>& coverages,
    const RgbaColor& color)
    -> std::optional<RgbaImage> {
    const std::size_t width = static_cast<std::size_t>(output_width);
    const std::size_t height = static_cast<std::size_t>(output_height);
    const std::size_t pixel_count = width * height;
    if (coverages.size() != pixel_count) {
        return std::nullopt;
    }
    if (pixel_count > std::numeric_limits<std::size_t>::max() / 4U) {
        return std::nullopt;
    }

    RgbaImage image;
    image.width = output_width;
    image.height = output_height;
    try {
        image.pixels.resize(pixel_count * 4U, 0U);
    } catch (...) {
        return std::nullopt;
    }

    const double sample_count = static_cast<double>(SAMPLE_COUNT_PER_PIXEL);
    for (std::size_t pixel_index = 0; pixel_index < pixel_count; ++pixel_index) {
        const int covered_count = count_covered_samples(coverages[pixel_index]);
        if (covered_count == 0) {
            continue;
        }

        const std::size_t byte_index = pixel_index * 4U;
        image.pixels[byte_index] = color.red;
        image.pixels[byte_index + 1U] = color.green;
        image.pixels[byte_index + 2U] = color.blue;
        image.pixels[byte_index + 3U] =
            to_byte(static_cast<double>(covered_count) / sample_count * 255.0);
    }

    return image;
}

std::uint8_t SvgRasterizer::to_byte(double value) {
    const double clamped_value = std::clamp(value, 0.0, 255.0);
    return static_cast<std::uint8_t>(std::round(clamped_value));
}

}
}
