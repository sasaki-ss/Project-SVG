# tests

## 目的

このディレクトリは Project SVG の単体テスト (GoogleTest) を配置するためのものです。
`src/` のディレクトリ (層) ごとにテストのディレクトリを切ります。

---

## ディレクトリ構成

```text
tests/
 ├─ parser/        # XmlNodeExtractor / SvgPathParser
 ├─ interpreter/   # SvgElementInterpreter / SvgShapeInterpreter / SvgPathInterpreter
 ├─ draw/          # SvgDrawModelBuilder
 ├─ rasterizer/    # SvgRasterizer
 └─ api/           # SvgRenderer (公開 API)。fixtures/ にテスト用 SVG
```

---

## ビルド手順

ルートディレクトリで以下を実行します。

```bash
cmake -S . -B build
cmake --build build
```

---

## テスト実行手順

ビルド後、以下でテストを実行します。

```bash
cd build && ctest --output-on-failure
```

個別テストを実行したい場合は、生成されたテスト実行ファイル (`project_svg_<層>_tests`) を直接実行してください。

---

## テスト追加ルール

- テストコードは `tests/<層>/` 配下に配置する。既存の層のディレクトリに置いた `*.cpp` は、ルートの `CMakeLists.txt` が GLOB で拾う
- 新しい層のディレクトリを作る場合は、ルートの `CMakeLists.txt` にテストターゲットを追加する (既存の `project_svg_<層>_tests` の定義が前例)
- テストのルールは `docs/test_coding_guidelines.md` に従う
