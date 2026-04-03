#ifndef PROJECT_SVG_INTERPRETER_PATH_INSTRUCTION_H_
#define PROJECT_SVG_INTERPRETER_PATH_INSTRUCTION_H_

#include <vector>

#include "InterpretedSvg.h"

namespace svg{
namespace interpreter{

enum class PathInstructionType {
    MoveTo,
    LineTo,
    CubicBezierTo,
    ClosePath,
};

struct PathInstruction {
    PathInstructionType type;
    std::vector<Point> points;
};

}
}

#endif  // PROJECT_SVG_INTERPRETER_PATH_INSTRUCTION_H_
