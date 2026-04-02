# tests

## 目的

このディレクトリは Project SVG のテストコードを配置するためのものです。  
対象ごとにディレクトリを切り、今後テストを追加していく前提とします。

現在は `parser` 配下に `XmlNodeExtractor` のテストを配置しています。

---

## ディレクトリ構成

```text
tests/
 ├─ CMakeLists.txt
 └─ parser/
     └─ XmlNodeExtractorTest.cpp
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
ctest --test-dir build --output-on-failure
```

個別テストを実行したい場合は、生成されたテスト実行ファイルを直接実行してください。

---

## テスト追加ルール

- テストコードは `tests` 配下に配置する
- 対象ごとにディレクトリを切る
- 新しいテストを追加した場合は `tests/CMakeLists.txt` に対象を追加する
- 本体実装の仕様変更とテスト追加は分けて扱う