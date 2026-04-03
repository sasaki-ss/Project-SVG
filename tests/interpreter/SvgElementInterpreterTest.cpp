#include <iostream>
#include <string>
#include <string_view>

#include "ExtractedNode.h"
#include "SvgElementInterpreter.h"

namespace {

bool expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << std::endl;
        return false;
    }
    return true;
}

svg::parser::ExtractedNode build_svg_root_node() {
    return svg::parser::ExtractedNode{
        .element_name = "svg",
        .attributes = {
            {.name = "viewBox", .value = "0 0 24 24"},
            {.name = "width", .value = "24"},
            {.name = "height", .value = "24"},
            {.name = "stroke", .value = "currentColor"},
            {.name = "fill", .value = "none"},
            {.name = "stroke-width", .value = "2"},
            {.name = "stroke-linecap", .value = "round"},
            {.name = "stroke-linejoin", .value = "round"},
        },
        .children = {
            svg::parser::ExtractedNode{.element_name = "g", .attributes = {}, .children = {
                svg::parser::ExtractedNode{.element_name = "path", .attributes = {}, .children = {}},
            }},
            svg::parser::ExtractedNode{.element_name = "rect", .attributes = {}, .children = {}},
        },
    };
}

bool test_interpret_builds_root_only_svg() {
    const auto root = build_svg_root_node();
    const auto interpreted = svg::interpreter::SvgElementInterpreter::interpret(root);

    if (!expect(interpreted.has_value(), "interpret-root: expected interpreted svg.")) {
        return false;
    }

    if (!expect(interpreted->view_box.min_x == 0.0 && interpreted->view_box.min_y == 0.0,
                "interpret-root: unexpected viewBox min coordinates.")) {
        return false;
    }

    if (!expect(interpreted->view_box.width == 24.0 && interpreted->view_box.height == 24.0,
                "interpret-root: unexpected viewBox size.")) {
        return false;
    }

    if (!expect(interpreted->width == 24.0 && interpreted->height == 24.0,
                "interpret-root: unexpected svg width / height.")) {
        return false;
    }

    if (!expect(interpreted->root_style.stroke == "currentColor",
                "interpret-root: unexpected stroke value.")) {
        return false;
    }

    if (!expect(interpreted->root_style.fill == "none", "interpret-root: unexpected fill value.")) {
        return false;
    }

    if (!expect(interpreted->root_style.stroke_width == 2.0,
                "interpret-root: unexpected stroke width value.")) {
        return false;
    }

    if (!expect(interpreted->root_style.stroke_linecap == svg::interpreter::StrokeLineCap::Round,
                "interpret-root: unexpected stroke-linecap value.")) {
        return false;
    }

    if (!expect(interpreted->root_style.stroke_linejoin == svg::interpreter::StrokeLineJoin::Round,
                "interpret-root: unexpected stroke-linejoin value.")) {
        return false;
    }

    return expect(interpreted->shapes.empty(), "interpret-root: shapes must be empty at this stage.");
}

bool test_interpret_returns_nullopt_for_missing_required_attribute() {
    auto root = build_svg_root_node();
    root.attributes.pop_back();

    const auto interpreted = svg::interpreter::SvgElementInterpreter::interpret(root);
    return expect(!interpreted.has_value(), "interpret-missing-attr: expected std::nullopt.");
}

bool test_interpret_returns_nullopt_for_invalid_root_element() {
    auto root = build_svg_root_node();
    root.element_name = "g";

    const auto interpreted = svg::interpreter::SvgElementInterpreter::interpret(root);
    return expect(!interpreted.has_value(), "interpret-invalid-root: expected std::nullopt.");
}

bool test_interpret_returns_nullopt_for_invalid_numeric_value() {
    auto root = build_svg_root_node();
    root.attributes[0].value = "0 0 24";

    const auto interpreted = svg::interpreter::SvgElementInterpreter::interpret(root);
    return expect(!interpreted.has_value(), "interpret-invalid-number: expected std::nullopt.");
}

bool test_interpret_element_type_mapping() {
    const auto path_type = svg::interpreter::SvgElementInterpreter::interpret_element_type("path");
    if (!expect(path_type.has_value() && *path_type == svg::interpreter::SvgElementType::Path,
                "element-type: expected path type.")) {
        return false;
    }

    const auto unsupported_type = svg::interpreter::SvgElementInterpreter::interpret_element_type("g");
    return expect(!unsupported_type.has_value(), "element-type: unsupported type must return std::nullopt.");
}

}  // namespace

int main() {
    if (!test_interpret_builds_root_only_svg()) {
        return 1;
    }

    if (!test_interpret_returns_nullopt_for_missing_required_attribute()) {
        return 1;
    }

    if (!test_interpret_returns_nullopt_for_invalid_root_element()) {
        return 1;
    }

    if (!test_interpret_returns_nullopt_for_invalid_numeric_value()) {
        return 1;
    }

    if (!test_interpret_element_type_mapping()) {
        return 1;
    }

    return 0;
}
