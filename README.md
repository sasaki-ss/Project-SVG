# Project-SVG
SVGファイルを入力として受け取り、指定した幅・高さに応じたRGBAバッファを生成するライブラリ

## Build

### Requirements
- CMake
- g++（C++17対応）

### Build
```sh
mkdir build
cd build
cmake ..
cmake --build .
```

### Build library target only
```sh
cmake --build . --target project_svg
```

### Run tests
```sh
ctest --output-on-failure
```

### Build without tests
```sh
cmake -DBUILD_TESTING=OFF ..
cmake --build .
```

## lucide アイコンの取得
lucide リポジトリの `icons/` のみを `assets/lucide/icons/` に取得する（バージョンは固定しない。管理対象外）。
```sh
python tools/lucide/fetch_icons.py
```
