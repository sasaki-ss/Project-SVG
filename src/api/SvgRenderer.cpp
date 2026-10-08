#include "SvgRenderer.h"

#include "draw/SvgDrawModelBuilder.h"
#include "interpreter/SvgElementInterpreter.h"
#include "loader/SvgLoader.h"
#include "parser/XmlNodeExtractor.h"
#include "rasterizer/SvgRasterizer.h"

namespace svg{
namespace api{

auto SvgRenderer::render_from_file(
    const std::filesystem::path& svg_file_path,
    int output_width,
    int output_height,
    RgbaColor current_color)
    -> std::optional<RgbaImage> {
    const auto svg_content = loader::SvgLoader::load_from_file(svg_file_path);
    if (!svg_content.has_value()) {
        return std::nullopt;
    }

    return render_from_string(*svg_content, output_width, output_height, current_color);
}

auto SvgRenderer::render_from_string(
    std::string_view svg_content,
    int output_width,
    int output_height,
    RgbaColor current_color)
    -> std::optional<RgbaImage> {
    if (output_width <= 0 || output_height <= 0) {
        return std::nullopt;
    }

    parser::XmlNodeExtractor extractor(svg_content);
    const auto extracted_root = extractor.extract_from_xml();
    if (!extracted_root.has_value()) {
        return std::nullopt;
    }

    const auto interpreted_svg = interpreter::SvgElementInterpreter::interpret(*extracted_root);
    if (!interpreted_svg.has_value()) {
        return std::nullopt;
    }

    const auto draw_shapes = draw::SvgDrawModelBuilder::build(*interpreted_svg);
    if (!draw_shapes.has_value()) {
        return std::nullopt;
    }

    return rasterizer::SvgRasterizer::rasterize(
        *draw_shapes,
        interpreted_svg->view_box,
        output_width,
        output_height,
        current_color);
}

}
}
