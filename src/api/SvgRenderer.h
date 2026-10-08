#ifndef PROJECT_SVG_API_SVG_RENDERER_H_
#define PROJECT_SVG_API_SVG_RENDERER_H_

#include <filesystem>
#include <optional>
#include <string_view>

#include "rasterizer/RgbaImage.h"

namespace svg{
namespace api{

using RgbaColor = rasterizer::RgbaColor;
using RgbaImage = rasterizer::RgbaImage;

class SvgRenderer {
public:
    static auto render_from_file(
        const std::filesystem::path& svg_file_path,
        int output_width,
        int output_height,
        RgbaColor current_color)
        -> std::optional<RgbaImage>;
    static auto render_from_string(
        std::string_view svg_content,
        int output_width,
        int output_height,
        RgbaColor current_color)
        -> std::optional<RgbaImage>;

private:
    SvgRenderer() = delete;
};

}
}

#endif  // PROJECT_SVG_API_SVG_RENDERER_H_
