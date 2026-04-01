#!/usr/bin/env python3
"""Lucide SVG icons analysis utility.

- Load all `.svg` files under the icons directory
- Parse XML safely
- Extract element tag names and attributes per element
- Continue processing even when parsing errors occur
"""

from __future__ import annotations

import argparse
import json
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any

try:
    from defusedxml import ElementTree as ET  # type: ignore
except Exception:  # pragma: no cover - fallback when dependency is unavailable
    import xml.etree.ElementTree as ET  # type: ignore


@dataclass
class ElementInfo:
    tag: str
    attributes: dict[str, str]


@dataclass
class SvgAnalysis:
    file: str
    elements: list[str]
    attribute_list: list[ElementInfo]
    error: str | None = None


def strip_namespace(tag: str) -> str:
    if "}" in tag:
        return tag.split("}", 1)[1]
    return tag


def analyze_single_svg(svg_path: Path, root_dir: Path) -> SvgAnalysis:
    relative_file = str(svg_path.relative_to(root_dir))

    try:
        tree = ET.parse(svg_path)
        root = tree.getroot()
    except Exception as exc:
        return SvgAnalysis(
            file=relative_file,
            elements=[],
            attribute_list=[],
            error=f"parse_error: {exc}",
        )

    tags: list[str] = []
    attr_list: list[ElementInfo] = []

    for elem in root.iter():
        clean_tag = strip_namespace(elem.tag)
        tags.append(clean_tag)
        attr_list.append(ElementInfo(tag=clean_tag, attributes={k: str(v) for k, v in elem.attrib.items()}))

    return SvgAnalysis(file=relative_file, elements=tags, attribute_list=attr_list)


def analyze_svgs(icons_dir: Path) -> dict[str, Any]:
    svg_paths = sorted(icons_dir.rglob("*.svg"))
    analyses: list[SvgAnalysis] = []

    for svg_path in svg_paths:
        analyses.append(analyze_single_svg(svg_path, icons_dir))

    return {
        "icons_dir": str(icons_dir),
        "svg_count": len(svg_paths),
        "results": [
            {
                **asdict(result),
                "attribute_list": [asdict(item) for item in result.attribute_list],
            }
            for result in analyses
        ],
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Analyze SVG elements and attributes from icons directory")
    parser.add_argument(
        "--icons-dir",
        type=Path,
        default=Path("icons"),
        help="Directory containing SVG files (default: icons)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Optional output JSON path. If omitted, prints JSON to stdout.",
    )
    parser.add_argument(
        "--indent",
        type=int,
        default=2,
        help="JSON indentation spaces (default: 2)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    icons_dir: Path = args.icons_dir

    if not icons_dir.exists() or not icons_dir.is_dir():
        payload = {
            "icons_dir": str(icons_dir),
            "svg_count": 0,
            "results": [],
            "warning": "icons_dir_not_found",
        }
    else:
        payload = analyze_svgs(icons_dir)

    json_text = json.dumps(payload, ensure_ascii=False, indent=args.indent)

    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json_text + "\n", encoding="utf-8")
        print(f"Wrote analysis result to: {args.output}")
    else:
        print(json_text)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
