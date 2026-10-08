#include "SvgDrawModelBuilder.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "interpreter/SvgPathInterpreter.h"
#include "parser/SvgPathParser.h"

namespace svg{
namespace draw{

auto SvgDrawModelBuilder::build(const interpreter::InterpretedSvg& interpreted_svg)
    -> std::optional<std::vector<DrawShape>> {
    const auto root_fill = parse_paint(interpreted_svg.root_style.fill);
    const auto root_stroke = parse_paint(interpreted_svg.root_style.stroke);
    if (!root_fill.has_value() || !root_stroke.has_value()) {
        return std::nullopt;
    }

    std::vector<DrawShape> draw_shapes;

    for (const interpreter::SvgShape& shape : interpreted_svg.shapes) {
        const auto resolved_style = resolve_style(
            shape,
            *root_fill,
            *root_stroke,
            interpreted_svg.root_style);
        if (!resolved_style.has_value()) {
            return std::nullopt;
        }

        const auto path_instructions = make_path_instructions(shape);
        if (!path_instructions.has_value()) {
            return std::nullopt;
        }
        if (path_instructions->empty()) {
            continue;
        }

        DrawShape draw_shape;
        draw_shape.path_instructions = *path_instructions;
        draw_shape.has_fill = resolved_style->has_fill;
        draw_shape.has_stroke = resolved_style->has_stroke;
        draw_shape.stroke_width = resolved_style->stroke_width;
        draw_shape.stroke_linecap = resolved_style->stroke_linecap;
        draw_shape.stroke_linejoin = resolved_style->stroke_linejoin;
        draw_shapes.push_back(std::move(draw_shape));
    }

    return draw_shapes;
}

auto SvgDrawModelBuilder::parse_paint(std::string_view value) -> std::optional<Paint> {
    if (value == "none") {
        return Paint::None;
    }
    if (value == "currentColor") {
        return Paint::CurrentColor;
    }
    return std::nullopt;
}

auto SvgDrawModelBuilder::resolve_style(
    const interpreter::SvgShape& shape,
    Paint root_fill,
    Paint root_stroke,
    const interpreter::SvgRootStyle& root_style)
    -> std::optional<ResolvedStyle> {
    Paint fill = root_fill;
    if (shape.fill.has_value()) {
        const auto parsed_fill = parse_paint(*shape.fill);
        if (!parsed_fill.has_value()) {
            return std::nullopt;
        }
        fill = *parsed_fill;
    }

    Paint stroke = root_stroke;
    if (shape.stroke.has_value()) {
        const auto parsed_stroke = parse_paint(*shape.stroke);
        if (!parsed_stroke.has_value()) {
            return std::nullopt;
        }
        stroke = *parsed_stroke;
    }

    ResolvedStyle style;
    style.has_fill = fill == Paint::CurrentColor;
    style.has_stroke = stroke == Paint::CurrentColor;
    style.stroke_width = root_style.stroke_width;
    style.stroke_linecap = root_style.stroke_linecap;
    style.stroke_linejoin = root_style.stroke_linejoin;
    if (shape.type == interpreter::SvgElementType::Line) {
        style.has_fill = false;
    }
    return style;
}

auto SvgDrawModelBuilder::make_path_instructions(const interpreter::SvgShape& shape)
    -> std::optional<std::vector<interpreter::PathInstruction>> {
    switch (shape.type) {
        case interpreter::SvgElementType::Path:
            return make_path_element_instructions(
                std::get<interpreter::PathElement>(shape.data));
        case interpreter::SvgElementType::Circle:
            return make_circle_instructions(
                std::get<interpreter::CircleElement>(shape.data));
        case interpreter::SvgElementType::Rect:
            return make_rect_instructions(
                std::get<interpreter::RectElement>(shape.data));
        case interpreter::SvgElementType::Line:
            return make_line_instructions(
                std::get<interpreter::LineElement>(shape.data));
        case interpreter::SvgElementType::Ellipse:
            return make_ellipse_instructions(
                std::get<interpreter::EllipseElement>(shape.data));
        case interpreter::SvgElementType::Polyline:
            return make_polyline_instructions(
                std::get<interpreter::PolylineElement>(shape.data));
        case interpreter::SvgElementType::Polygon:
            return make_polygon_instructions(
                std::get<interpreter::PolygonElement>(shape.data));
        default:
            return std::nullopt;
    }
}

auto SvgDrawModelBuilder::make_path_element_instructions(
    const interpreter::PathElement& element)
    -> std::optional<std::vector<interpreter::PathInstruction>> {
    parser::SvgPathParser parser;
    const auto commands = parser.parse(element.d);
    if (!commands.has_value()) {
        return std::nullopt;
    }

    interpreter::SvgPathInterpreter interpreter;
    return interpreter.interpret(*commands);
}

auto SvgDrawModelBuilder::make_circle_instructions(
    const interpreter::CircleElement& element)
    -> std::optional<std::vector<interpreter::PathInstruction>> {
    if (element.r < 0.0) {
        return std::nullopt;
    }
    if (element.r == 0.0) {
        return std::vector<interpreter::PathInstruction>{};
    }

    return make_ellipse_path(
        interpreter::Point{element.cx, element.cy},
        element.r,
        element.r);
}

auto SvgDrawModelBuilder::make_rect_instructions(
    const interpreter::RectElement& element)
    -> std::optional<std::vector<interpreter::PathInstruction>> {
    if (element.width < 0.0 || element.height < 0.0) {
        return std::nullopt;
    }
    if (element.width == 0.0 || element.height == 0.0) {
        return std::vector<interpreter::PathInstruction>{};
    }

    double radius_x = element.rx.value_or(0.0);
    double radius_y = element.ry.value_or(0.0);
    if (element.rx.has_value() && !element.ry.has_value()) {
        radius_y = radius_x;
    }
    if (!element.rx.has_value() && element.ry.has_value()) {
        radius_x = radius_y;
    }
    if (radius_x < 0.0 || radius_y < 0.0) {
        return std::nullopt;
    }

    radius_x = std::min(radius_x, element.width / 2.0);
    radius_y = std::min(radius_y, element.height / 2.0);
    if (radius_x == 0.0 || radius_y == 0.0) {
        return make_rectangle_path(element.x, element.y, element.width, element.height);
    }

    return make_rounded_rectangle_path(
        element.x,
        element.y,
        element.width,
        element.height,
        radius_x,
        radius_y);
}

auto SvgDrawModelBuilder::make_line_instructions(
    const interpreter::LineElement& element)
    -> std::optional<std::vector<interpreter::PathInstruction>> {
    return std::vector<interpreter::PathInstruction>{
        make_move_to(interpreter::Point{element.x1, element.y1}),
        make_line_to(interpreter::Point{element.x2, element.y2}),
    };
}

auto SvgDrawModelBuilder::make_ellipse_instructions(
    const interpreter::EllipseElement& element)
    -> std::optional<std::vector<interpreter::PathInstruction>> {
    if (element.rx < 0.0 || element.ry < 0.0) {
        return std::nullopt;
    }
    if (element.rx == 0.0 || element.ry == 0.0) {
        return std::vector<interpreter::PathInstruction>{};
    }

    return make_ellipse_path(
        interpreter::Point{element.cx, element.cy},
        element.rx,
        element.ry);
}

auto SvgDrawModelBuilder::make_polyline_instructions(
    const interpreter::PolylineElement& element)
    -> std::optional<std::vector<interpreter::PathInstruction>> {
    return make_open_polyline_path(element.points);
}

auto SvgDrawModelBuilder::make_polygon_instructions(
    const interpreter::PolygonElement& element)
    -> std::optional<std::vector<interpreter::PathInstruction>> {
    std::vector<interpreter::PathInstruction> instructions = make_open_polyline_path(
        element.points);
    if (!instructions.empty()) {
        instructions.push_back(make_close_path());
    }
    return instructions;
}

auto SvgDrawModelBuilder::make_ellipse_path(
    const interpreter::Point& center,
    double radius_x,
    double radius_y)
    -> std::vector<interpreter::PathInstruction> {
    constexpr double QUARTER_ARC_ANGLE = 1.57079632679489661923;
    constexpr double CUBIC_ARC_FACTOR = 4.0 / 3.0;
    const double control_point_scale = CUBIC_ARC_FACTOR
        * std::tan(QUARTER_ARC_ANGLE / 4.0);
    const double control_offset_x = radius_x * control_point_scale;
    const double control_offset_y = radius_y * control_point_scale;

    const interpreter::Point right{center.x + radius_x, center.y};
    const interpreter::Point bottom{center.x, center.y + radius_y};
    const interpreter::Point left{center.x - radius_x, center.y};
    const interpreter::Point top{center.x, center.y - radius_y};

    return {
        make_move_to(right),
        make_cubic_bezier_to(
            interpreter::Point{right.x, right.y + control_offset_y},
            interpreter::Point{bottom.x + control_offset_x, bottom.y},
            bottom),
        make_cubic_bezier_to(
            interpreter::Point{bottom.x - control_offset_x, bottom.y},
            interpreter::Point{left.x, left.y + control_offset_y},
            left),
        make_cubic_bezier_to(
            interpreter::Point{left.x, left.y - control_offset_y},
            interpreter::Point{top.x - control_offset_x, top.y},
            top),
        make_cubic_bezier_to(
            interpreter::Point{top.x + control_offset_x, top.y},
            interpreter::Point{right.x, right.y - control_offset_y},
            right),
        make_close_path(),
    };
}

auto SvgDrawModelBuilder::make_rectangle_path(
    double x,
    double y,
    double width,
    double height)
    -> std::vector<interpreter::PathInstruction> {
    const interpreter::Point top_left{x, y};
    const interpreter::Point top_right{x + width, y};
    const interpreter::Point bottom_right{x + width, y + height};
    const interpreter::Point bottom_left{x, y + height};

    return {
        make_move_to(top_left),
        make_line_to(top_right),
        make_line_to(bottom_right),
        make_line_to(bottom_left),
        make_close_path(),
    };
}

auto SvgDrawModelBuilder::make_rounded_rectangle_path(
    double x,
    double y,
    double width,
    double height,
    double radius_x,
    double radius_y)
    -> std::vector<interpreter::PathInstruction> {
    constexpr double QUARTER_ARC_ANGLE = 1.57079632679489661923;
    constexpr double CUBIC_ARC_FACTOR = 4.0 / 3.0;
    const double control_point_scale = CUBIC_ARC_FACTOR
        * std::tan(QUARTER_ARC_ANGLE / 4.0);
    const double control_offset_x = radius_x * control_point_scale;
    const double control_offset_y = radius_y * control_point_scale;

    const double right = x + width;
    const double bottom = y + height;
    const interpreter::Point top_start{x + radius_x, y};
    const interpreter::Point top_end{right - radius_x, y};
    const interpreter::Point right_start{right, y + radius_y};
    const interpreter::Point right_end{right, bottom - radius_y};
    const interpreter::Point bottom_start{right - radius_x, bottom};
    const interpreter::Point bottom_end{x + radius_x, bottom};
    const interpreter::Point left_start{x, bottom - radius_y};
    const interpreter::Point left_end{x, y + radius_y};

    return {
        make_move_to(top_start),
        make_line_to(top_end),
        make_cubic_bezier_to(
            interpreter::Point{top_end.x + control_offset_x, top_end.y},
            interpreter::Point{right_start.x, right_start.y - control_offset_y},
            right_start),
        make_line_to(right_end),
        make_cubic_bezier_to(
            interpreter::Point{right_end.x, right_end.y + control_offset_y},
            interpreter::Point{bottom_start.x + control_offset_x, bottom_start.y},
            bottom_start),
        make_line_to(bottom_end),
        make_cubic_bezier_to(
            interpreter::Point{bottom_end.x - control_offset_x, bottom_end.y},
            interpreter::Point{left_start.x, left_start.y + control_offset_y},
            left_start),
        make_line_to(left_end),
        make_cubic_bezier_to(
            interpreter::Point{left_end.x, left_end.y - control_offset_y},
            interpreter::Point{top_start.x - control_offset_x, top_start.y},
            top_start),
        make_close_path(),
    };
}

auto SvgDrawModelBuilder::make_open_polyline_path(
    const std::vector<interpreter::Point>& points)
    -> std::vector<interpreter::PathInstruction> {
    if (points.empty()) {
        return {};
    }

    std::vector<interpreter::PathInstruction> instructions;
    instructions.push_back(make_move_to(points[0]));
    for (std::size_t index = 1; index < points.size(); ++index) {
        instructions.push_back(make_line_to(points[index]));
    }
    return instructions;
}

auto SvgDrawModelBuilder::make_move_to(const interpreter::Point& point)
    -> interpreter::PathInstruction {
    return interpreter::PathInstruction{
        interpreter::PathInstructionType::MoveTo,
        {point},
    };
}

auto SvgDrawModelBuilder::make_line_to(const interpreter::Point& point)
    -> interpreter::PathInstruction {
    return interpreter::PathInstruction{
        interpreter::PathInstructionType::LineTo,
        {point},
    };
}

auto SvgDrawModelBuilder::make_cubic_bezier_to(
    const interpreter::Point& control_point1,
    const interpreter::Point& control_point2,
    const interpreter::Point& end_point)
    -> interpreter::PathInstruction {
    return interpreter::PathInstruction{
        interpreter::PathInstructionType::CubicBezierTo,
        {control_point1, control_point2, end_point},
    };
}

auto SvgDrawModelBuilder::make_close_path() -> interpreter::PathInstruction {
    return interpreter::PathInstruction{
        interpreter::PathInstructionType::ClosePath,
        {},
    };
}

}
}
