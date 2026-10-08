#ifndef PROJECT_SVG_RASTERIZER_RGBA_IMAGE_H_
#define PROJECT_SVG_RASTERIZER_RGBA_IMAGE_H_

#include <cstdint>
#include <vector>

namespace svg{
namespace rasterizer{

struct RgbaColor {
    std::uint8_t red;
    std::uint8_t green;
    std::uint8_t blue;
    std::uint8_t alpha;
};

struct RgbaImage {
    int width;
    int height;
    std::vector<std::uint8_t> pixels;
};

}
}

#endif  // PROJECT_SVG_RASTERIZER_RGBA_IMAGE_H_
