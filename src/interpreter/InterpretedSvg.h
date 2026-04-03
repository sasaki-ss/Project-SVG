#ifndef PROJECT_SVG_INTERPRETER_INTERPRETED_SVG_H_
#define PROJECT_SVG_INTERPRETER_INTERPRETED_SVG_H_

#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg{
namespace interpreter{

enum class SvgElementType {
    Path,
    Circle,
    Rect,
    Line,
    Ellipse,
    Polyline,
    Polygon,
};

enum class StrokeLineCap {
    Round,
};

enum class StrokeLineJoin {
    Round,
};

struct Point {
    double x;
    double y;
};

struct SvgViewBox {
    double min_x;
    double min_y;
    double width;
    double height;
};

struct SvgRootStyle {
    std::string stroke;
    std::string fill;
    double stroke_width;
    StrokeLineCap stroke_linecap;
    StrokeLineJoin stroke_linejoin;
};

struct PathElement {
    std::string d;
};

struct CircleElement {
    double cx;
    double cy;
    double r;
};

struct RectElement {
    double x;
    double y;
    double width;
    double height;
    std::optional<double> rx;
    std::optional<double> ry;
};

struct LineElement {
    double x1;
    double y1;
    double x2;
    double y2;
};

struct EllipseElement {
    double cx;
    double cy;
    double rx;
    double ry;
};

struct PolylineElement {
    std::vector<Point> points;
};

struct PolygonElement {
    std::vector<Point> points;
};

using SvgShapeData = std::variant<
    PathElement,
    CircleElement,
    RectElement,
    LineElement,
    EllipseElement,
    PolylineElement,
    PolygonElement>;

struct SvgShape {
    SvgElementType type;
    SvgShapeData data;
};

struct InterpretedSvg {
    SvgViewBox view_box;
    double width;
    double height;
    SvgRootStyle root_style;
    std::vector<SvgShape> shapes;
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_INTERPRETED_SVG_H_
