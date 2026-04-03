#include "SvgElementInterpreter.h"

namespace svg{
namespace interpreter{

std::optional<InterpretedSvg> SvgElementInterpreter::interpret(const parser::ExtractedNode& root) {
    if (root.element_name != "svg") {
        return std::nullopt;
    }

    for (const auto& child_node : root.children) {
        traverse_child_nodes(child_node);
    }

    // この段階では属性解釈や shape 実体化を行わない。
    // InterpretedSvg を構築するための必須情報が不足しているため失敗を返す。
    return std::nullopt;
}

std::optional<SvgElementType> SvgElementInterpreter::interpret_element_type(std::string_view element_name) {
    if (element_name == "path") {
        return SvgElementType::Path;
    }
    if (element_name == "circle") {
        return SvgElementType::Circle;
    }
    if (element_name == "rect") {
        return SvgElementType::Rect;
    }
    if (element_name == "line") {
        return SvgElementType::Line;
    }
    if (element_name == "ellipse") {
        return SvgElementType::Ellipse;
    }
    if (element_name == "polyline") {
        return SvgElementType::Polyline;
    }
    if (element_name == "polygon") {
        return SvgElementType::Polygon;
    }

    return std::nullopt;
}

void SvgElementInterpreter::traverse_child_nodes(const parser::ExtractedNode& node) {
    const auto element_type = interpret_element_type(node.element_name);
    (void)element_type;

    for (const auto& child_node : node.children) {
        traverse_child_nodes(child_node);
    }
}

}
}
