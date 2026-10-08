#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <string_view>

#include "api/SvgRenderer.h"

using svg::api::RgbaColor;
using svg::api::SvgRenderer;

TEST(SvgRendererTest, RendersLucideStyleSvgString) {
    const std::string_view svg_content = R"svg(
<svg
    viewBox="0 0 24 24"
    width="24"
    height="24"
    stroke="currentColor"
    fill="none"
    stroke-width="2"
    stroke-linecap="round"
    stroke-linejoin="round">
    <path d="M3 12H21"/>
</svg>
)svg";

    const auto result = SvgRenderer::render_from_string(
        svg_content,
        24,
        24,
        RgbaColor{12U, 34U, 56U, 255U});

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->pixels.size(), 24U * 24U * 4U);
    const std::size_t pixel_index = (12U * 24U + 12U) * 4U;
    EXPECT_EQ(result->pixels[pixel_index], 12U);
    EXPECT_EQ(result->pixels[pixel_index + 1U], 34U);
    EXPECT_EQ(result->pixels[pixel_index + 2U], 56U);
    EXPECT_EQ(result->pixels[pixel_index + 3U], 255U);
}

TEST(SvgRendererTest, RendersSvgFileFromConfiguredFixturePath) {
    const std::filesystem::path fixture_path(PROJECT_SVG_API_FIXTURE_PATH);

    const auto result = SvgRenderer::render_from_file(
        fixture_path,
        24,
        24,
        RgbaColor{12U, 34U, 56U, 255U});

    ASSERT_TRUE(result.has_value());
    ASSERT_EQ(result->pixels.size(), 24U * 24U * 4U);
    const std::size_t pixel_index = (12U * 24U + 12U) * 4U;
    EXPECT_EQ(result->pixels[pixel_index + 3U], 255U);
}

TEST(SvgRendererTest, InvalidSvgStringReturnsNullopt) {
    const auto result = SvgRenderer::render_from_string(
        "<svg>",
        24,
        24,
        RgbaColor{12U, 34U, 56U, 255U});

    EXPECT_FALSE(result.has_value());
}

TEST(SvgRendererTest, MissingSvgFileReturnsNullopt) {
    const std::filesystem::path fixture_path(PROJECT_SVG_API_FIXTURE_PATH);
    const std::filesystem::path missing_path = fixture_path.parent_path() / "Missing.svg";

    const auto result = SvgRenderer::render_from_file(
        missing_path,
        24,
        24,
        RgbaColor{12U, 34U, 56U, 255U});

    EXPECT_FALSE(result.has_value());
}

TEST(SvgRendererTest, ZeroOutputWidthReturnsNullopt) {
    const std::string_view svg_content = R"svg(
<svg
    viewBox="0 0 24 24"
    width="24"
    height="24"
    stroke="currentColor"
    fill="none"
    stroke-width="2"
    stroke-linecap="round"
    stroke-linejoin="round">
    <path d="M3 12H21"/>
</svg>
)svg";

    const auto result = SvgRenderer::render_from_string(
        svg_content,
        0,
        24,
        RgbaColor{12U, 34U, 56U, 255U});

    EXPECT_FALSE(result.has_value());
}
