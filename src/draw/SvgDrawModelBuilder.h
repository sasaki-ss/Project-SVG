#ifndef PROJECT_SVG_DRAW_SVG_DRAW_MODEL_BUILDER_H_
#define PROJECT_SVG_DRAW_SVG_DRAW_MODEL_BUILDER_H_

#include <optional>
#include <string_view>
#include <vector>

#include "DrawShape.h"
#include "interpreter/InterpretedSvg.h"

namespace svg{
namespace draw{

class SvgDrawModelBuilder {
public:
    static auto build(const interpreter::InterpretedSvg& interpreted_svg)
        -> std::optional<std::vector<DrawShape>>;

private:
    enum class Paint {
        None,
        CurrentColor,
    };

    struct ResolvedStyle {
        bool has_fill;
        bool has_stroke;
        double stroke_width;
        interpreter::StrokeLineCap stroke_linecap;
        interpreter::StrokeLineJoin stroke_linejoin;
    };

    SvgDrawModelBuilder() = delete;

    static auto parse_paint(std::string_view value) -> std::optional<Paint>;
    static auto resolve_style(
        const interpreter::SvgShape& shape,
        Paint root_fill,
        Paint root_stroke,
        const interpreter::SvgRootStyle& root_style)
        -> std::optional<ResolvedStyle>;
    static auto make_path_instructions(const interpreter::SvgShape& shape)
        -> std::optional<std::vector<interpreter::PathInstruction>>;
    static auto make_path_element_instructions(const interpreter::PathElement& element)
        -> std::optional<std::vector<interpreter::PathInstruction>>;
    static auto make_circle_instructions(const interpreter::CircleElement& element)
        -> std::optional<std::vector<interpreter::PathInstruction>>;
    static auto make_rect_instructions(const interpreter::RectElement& element)
        -> std::optional<std::vector<interpreter::PathInstruction>>;
    static auto make_line_instructions(const interpreter::LineElement& element)
        -> std::optional<std::vector<interpreter::PathInstruction>>;
    static auto make_ellipse_instructions(const interpreter::EllipseElement& element)
        -> std::optional<std::vector<interpreter::PathInstruction>>;
    static auto make_polyline_instructions(const interpreter::PolylineElement& element)
        -> std::optional<std::vector<interpreter::PathInstruction>>;
    static auto make_polygon_instructions(const interpreter::PolygonElement& element)
        -> std::optional<std::vector<interpreter::PathInstruction>>;
    static auto make_ellipse_path(
        const interpreter::Point& center,
        double radius_x,
        double radius_y)
        -> std::vector<interpreter::PathInstruction>;
    static auto make_rectangle_path(
        double x,
        double y,
        double width,
        double height)
        -> std::vector<interpreter::PathInstruction>;
    static auto make_rounded_rectangle_path(
        double x,
        double y,
        double width,
        double height,
        double radius_x,
        double radius_y)
        -> std::vector<interpreter::PathInstruction>;
    static auto make_open_polyline_path(const std::vector<interpreter::Point>& points)
        -> std::vector<interpreter::PathInstruction>;
    static auto make_move_to(const interpreter::Point& point)
        -> interpreter::PathInstruction;
    static auto make_line_to(const interpreter::Point& point)
        -> interpreter::PathInstruction;
    static auto make_cubic_bezier_to(
        const interpreter::Point& control_point1,
        const interpreter::Point& control_point2,
        const interpreter::Point& end_point)
        -> interpreter::PathInstruction;
    static auto make_close_path() -> interpreter::PathInstruction;
};

}
}

#endif  // PROJECT_SVG_DRAW_SVG_DRAW_MODEL_BUILDER_H_
