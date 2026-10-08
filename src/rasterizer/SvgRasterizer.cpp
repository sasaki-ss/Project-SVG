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

    const auto samples = make_sample_buffer(output_width, output_height);
    if (!samples.has_value()) {
        return std::nullopt;
    }

    const RgbaColor opaque_color{color.red, color.green, color.blue, 255U};
    std::vector<PremultipliedColor> raster_samples = *samples;
    for (const draw::DrawShape& draw_shape : draw_shapes) {
        if (!std::isfinite(draw_shape.stroke_width) || draw_shape.stroke_width < 0.0) {
            return std::nullopt;
        }

        const auto subpaths = make_subpaths(draw_shape.path_instructions, *transform);
        if (!subpaths.has_value()) {
            return std::nullopt;
        }

        rasterize_draw_shape(
            raster_samples,
            output_width,
            output_height,
            draw_shape,
            *subpaths,
            draw_shape.stroke_width * transform->scale,
            opaque_color);
    }

    auto output_image = make_output_image(
        output_width,
        output_height,
        raster_samples);
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

bool SvgRasterizer::is_point_in_fill(
    const RasterPoint& sample_point,
    const std::vector<Subpath>& subpaths) {
    int winding_number = 0;
    for (const Subpath& subpath : subpaths) {
        winding_number += calculate_winding_number(sample_point, subpath);
    }
    return winding_number != 0;
}

int SvgRasterizer::calculate_winding_number(
    const RasterPoint& sample_point,
    const Subpath& subpath) {
    if (subpath.points.size() < 2U) {
        return 0;
    }

    int winding_number = 0;
    for (std::size_t index = 0; index < subpath.points.size(); ++index) {
        const RasterPoint& segment_start = subpath.points[index];
        const RasterPoint& segment_end = subpath.points[
            (index + 1U) % subpath.points.size()];
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
    const std::vector<Subpath>& subpaths,
    double stroke_width) {
    if (stroke_width <= 0.0) {
        return false;
    }

    const double radius = stroke_width / 2.0;
    for (const Subpath& subpath : subpaths) {
        if (subpath.points.size() < 2U) {
            continue;
        }

        for (std::size_t index = 1; index < subpath.points.size(); ++index) {
            if (is_point_in_round_segment(
                    sample_point,
                    subpath.points[index - 1U],
                    subpath.points[index],
                    radius)) {
                return true;
            }
        }

        if (subpath.is_closed &&
            is_point_in_round_segment(
                sample_point,
                subpath.points.back(),
                subpath.points.front(),
                radius)) {
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

void SvgRasterizer::paint_sample(
    PremultipliedColor& destination,
    const RgbaColor& source_color) {
    const double source_alpha = static_cast<double>(source_color.alpha) / 255.0;
    const double inverse_source_alpha = 1.0 - source_alpha;
    destination.red = static_cast<double>(source_color.red) / 255.0 * source_alpha +
        destination.red * inverse_source_alpha;
    destination.green = static_cast<double>(source_color.green) / 255.0 * source_alpha +
        destination.green * inverse_source_alpha;
    destination.blue = static_cast<double>(source_color.blue) / 255.0 * source_alpha +
        destination.blue * inverse_source_alpha;
    destination.alpha = source_alpha + destination.alpha * inverse_source_alpha;
}

auto SvgRasterizer::make_sample_buffer(int output_width, int output_height)
    -> std::optional<std::vector<PremultipliedColor>> {
    if (output_width <= 0 || output_height <= 0) {
        return std::nullopt;
    }

    const std::size_t width = static_cast<std::size_t>(output_width);
    const std::size_t height = static_cast<std::size_t>(output_height);
    if (width > std::numeric_limits<std::size_t>::max() / height) {
        return std::nullopt;
    }
    const std::size_t pixel_count = width * height;
    if (pixel_count >
        std::numeric_limits<std::size_t>::max() /
            static_cast<std::size_t>(SAMPLE_COUNT_PER_PIXEL)) {
        return std::nullopt;
    }

    try {
        return std::vector<PremultipliedColor>(
            pixel_count * static_cast<std::size_t>(SAMPLE_COUNT_PER_PIXEL),
            PremultipliedColor{0.0, 0.0, 0.0, 0.0});
    } catch (...) {
        return std::nullopt;
    }
}

void SvgRasterizer::rasterize_draw_shape(
    std::vector<PremultipliedColor>& samples,
    int output_width,
    int output_height,
    const draw::DrawShape& draw_shape,
    const std::vector<Subpath>& subpaths,
    double stroke_width,
    const RgbaColor& color) {
    for (int pixel_y = 0; pixel_y < output_height; ++pixel_y) {
        for (int pixel_x = 0; pixel_x < output_width; ++pixel_x) {
            for (int sample_y = 0; sample_y < SUPERSAMPLE_COUNT; ++sample_y) {
                for (int sample_x = 0; sample_x < SUPERSAMPLE_COUNT; ++sample_x) {
                    const double offset_x =
                        (static_cast<double>(sample_x) + 0.5) /
                        static_cast<double>(SUPERSAMPLE_COUNT);
                    const double offset_y =
                        (static_cast<double>(sample_y) + 0.5) /
                        static_cast<double>(SUPERSAMPLE_COUNT);
                    const RasterPoint sample_point{
                        static_cast<double>(pixel_x) + offset_x,
                        static_cast<double>(pixel_y) + offset_y,
                    };
                    const std::size_t sample_index =
                        (static_cast<std::size_t>(pixel_y) *
                            static_cast<std::size_t>(output_width) +
                         static_cast<std::size_t>(pixel_x)) *
                            static_cast<std::size_t>(SAMPLE_COUNT_PER_PIXEL) +
                        static_cast<std::size_t>(sample_y * SUPERSAMPLE_COUNT + sample_x);
                    PremultipliedColor& destination = samples[sample_index];
                    if (draw_shape.has_fill && is_point_in_fill(sample_point, subpaths)) {
                        paint_sample(destination, color);
                    }
                    if (draw_shape.has_stroke &&
                        is_point_in_stroke(
                            sample_point,
                            subpaths,
                            stroke_width)) {
                        paint_sample(destination, color);
                    }
                }
            }
        }
    }
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
    const std::vector<PremultipliedColor>& samples)
    -> std::optional<RgbaImage> {
    const std::size_t width = static_cast<std::size_t>(output_width);
    const std::size_t height = static_cast<std::size_t>(output_height);
    if (width > std::numeric_limits<std::size_t>::max() / height) {
        return std::nullopt;
    }
    const std::size_t pixel_count = width * height;
    if (pixel_count > std::numeric_limits<std::size_t>::max() / 4U) {
        return std::nullopt;
    }
    if (samples.size() !=
        pixel_count * static_cast<std::size_t>(SAMPLE_COUNT_PER_PIXEL)) {
        return std::nullopt;
    }

    RgbaImage image;
    image.width = output_width;
    image.height = output_height;
    try {
        image.pixels.resize(pixel_count * 4U);
    } catch (...) {
        return std::nullopt;
    }

    for (std::size_t pixel_index = 0; pixel_index < pixel_count; ++pixel_index) {
        PremultipliedColor accumulated{0.0, 0.0, 0.0, 0.0};
        for (int sample_index = 0; sample_index < SAMPLE_COUNT_PER_PIXEL; ++sample_index) {
            const PremultipliedColor& sample = samples[
                pixel_index * static_cast<std::size_t>(SAMPLE_COUNT_PER_PIXEL) +
                static_cast<std::size_t>(sample_index)];
            accumulated.red += sample.red;
            accumulated.green += sample.green;
            accumulated.blue += sample.blue;
            accumulated.alpha += sample.alpha;
        }

        const double sample_count = static_cast<double>(SAMPLE_COUNT_PER_PIXEL);
        accumulated.red /= sample_count;
        accumulated.green /= sample_count;
        accumulated.blue /= sample_count;
        accumulated.alpha /= sample_count;

        const std::size_t byte_index = pixel_index * 4U;
        image.pixels[byte_index + 3U] = to_byte(accumulated.alpha * 255.0);
        if (accumulated.alpha <= 0.0) {
            image.pixels[byte_index] = 0U;
            image.pixels[byte_index + 1U] = 0U;
            image.pixels[byte_index + 2U] = 0U;
            continue;
        }

        image.pixels[byte_index] =
            to_byte(accumulated.red / accumulated.alpha * 255.0);
        image.pixels[byte_index + 1U] =
            to_byte(accumulated.green / accumulated.alpha * 255.0);
        image.pixels[byte_index + 2U] =
            to_byte(accumulated.blue / accumulated.alpha * 255.0);
    }

    return image;
}

std::uint8_t SvgRasterizer::to_byte(double value) {
    const double clamped_value = std::clamp(value, 0.0, 255.0);
    return static_cast<std::uint8_t>(std::round(clamped_value));
}

}
}
