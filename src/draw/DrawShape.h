#ifndef PROJECT_SVG_DRAW_DRAW_SHAPE_H_
#define PROJECT_SVG_DRAW_DRAW_SHAPE_H_

#include <vector>

#include "interpreter/PathInstruction.h"

namespace svg{
namespace draw{

struct DrawShape {
    std::vector<interpreter::PathInstruction> path_instructions;
    bool has_fill;
    bool has_stroke;
    double stroke_width;
    interpreter::StrokeLineCap stroke_linecap;
    interpreter::StrokeLineJoin stroke_linejoin;
};

}
}

#endif  // PROJECT_SVG_DRAW_DRAW_SHAPE_H_
