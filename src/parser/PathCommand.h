#ifndef PROJECT_SVG_PARSER_PATH_COMMAND_H_
#define PROJECT_SVG_PARSER_PATH_COMMAND_H_

#include <vector>

namespace svg{
namespace parser{

enum class PathCommandType {
    MoveTo,
    LineTo,
    HorizontalTo,
    VerticalTo,
    CubicBezierTo,
    ClosePath,
    SmoothCubicBezierTo,
    QuadraticBezierTo,
    SmoothQuadraticBezierTo,
    ArcTo,
};

struct PathCommand {
    PathCommandType type;
    bool is_absolute;
    std::vector<double> parameters;
};

}
}

#endif  // PROJECT_SVG_PARSER_PATH_COMMAND_H_
