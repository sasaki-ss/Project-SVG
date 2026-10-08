#!/usr/bin/env python3
"""Fetch only the `icons/` directory of the lucide repository.

The version is intentionally not pinned: each run follows the default branch.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

LUCIDE_REPOSITORY_URL = "https://github.com/lucide-icons/lucide.git"
DEFAULT_OUTPUT_DIR = Path(__file__).resolve().parents[2] / "assets" / "lucide"


def run_git(arguments: list[str], working_dir: Path | None = None) -> None:
    subprocess.run(["git", *arguments], cwd=working_dir, check=True)


def clone_icons(output_dir: Path) -> None:
    output_dir.parent.mkdir(parents=True, exist_ok=True)
    run_git([
        "clone",
        "--depth", "1",
        "--filter=blob:none",
        "--sparse",
        LUCIDE_REPOSITORY_URL,
        str(output_dir),
    ])
    restrict_to_icons(output_dir)


def restrict_to_icons(output_dir: Path) -> None:
    run_git(["sparse-checkout", "set", "--no-cone", "/icons/"], output_dir)


def update_icons(output_dir: Path) -> None:
    restrict_to_icons(output_dir)
    run_git(["fetch", "--depth", "1", "origin", "HEAD"], output_dir)
    run_git(["reset", "--hard", "FETCH_HEAD"], output_dir)


def main() -> int:
    parser = argparse.ArgumentParser(description="Fetch lucide icons (icons/ only).")
    parser.add_argument(
        "--output-dir",
        type=Path,
        default=DEFAULT_OUTPUT_DIR,
        help="clone destination (default: assets/lucide)",
    )
    args = parser.parse_args()
    output_dir: Path = args.output_dir.resolve()

    try:
        if (output_dir / ".git").is_dir():
            update_icons(output_dir)
        else:
            clone_icons(output_dir)
    except subprocess.CalledProcessError as exc:
        print(f"git command failed: {exc}", file=sys.stderr)
        return 1

    icon_count = len(list((output_dir / "icons").glob("*.svg")))
    print(f"{icon_count} icons in {output_dir / 'icons'}")
    return 0 if icon_count > 0 else 1


if __name__ == "__main__":
    sys.exit(main())
