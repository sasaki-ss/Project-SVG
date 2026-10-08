# Project-SVG
SVGファイルを入力として受け取り、指定した幅・高さに応じたRGBAバッファを生成するライブラリ

## Build

### Requirements
- CMake 3.16 以上
- C++17 対応のコンパイラ。CI では次の 2 つでビルドとテストを行っている
  - MSVC (Visual Studio 2022)
  - MinGW-w64 (g++)
- テストをビルドする場合は GoogleTest のサブモジュール (`git submodule update --init --recursive`)

### MSVC
```sh
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
cd build && ctest -C Release --output-on-failure
```

### MinGW-w64
```sh
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build && ctest --output-on-failure
```

### Build library target only
```sh
cmake --build build --target project_svg
```

### Build without tests
```sh
cmake -S . -B build -DPROJECT_SVG_BUILD_TESTS=OFF
```

## 他のプロジェクトから使う
`add_subdirectory` で取り込み、`project_svg` にリンクする。取り込まれた側ではテストはビルドされない (`PROJECT_SVG_BUILD_TESTS` の既定は、このリポジトリを直接ビルドしたときだけ ON)。

```cmake
add_subdirectory(path/to/Project_SVG project_svg)
target_link_libraries(your_app PRIVATE project_svg)
```

```cpp
#include "api/SvgRenderer.h"

const auto image = svg::api::SvgRenderer::render_from_file(
    "icon.svg", 64, 64, svg::api::RgbaColor{255, 255, 255, 255});
if (image.has_value()) {
    // image->pixels: 幅 x 高さ x 4 バイト。1 画素は R, G, B, A (ストレートα)、行は上から下、行の間に余白なし
}
```

- `RgbaColor` は `currentColor` に使う色。α はアイコン全体の不透明度として扱う
- 失敗 (読込失敗・解釈失敗・幅か高さが 0 以下) は `std::nullopt`
- MinGW でアプリを作る場合、別の MinGW (Git for Windows 同梱など) の libstdc++ の DLL が PATH 上で先に見つかると起動に失敗することがある。`-static-libgcc -static-libstdc++` でリンクするか、ビルドに使った MinGW の bin を PATH の先頭に置く

## lucide アイコンの取得
lucide リポジトリの `icons/` のみを `assets/lucide/icons/` に取得する（バージョンは固定しない。管理対象外）。
```sh
python tools/lucide/fetch_icons.py
```
