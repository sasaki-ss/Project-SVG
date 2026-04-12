# Parser テストケース一覧

## 対象責務
文字列からノード構造・要素・属性を抽出する処理に関する既存テストケース一覧。
構文解析・ノード抽出に関わる観点のみを対象とする。

## tests/parser/XmlNodeExtractorTest.cpp

### テストケース表

| No | テストケース名 | 概要 | 区分 | 備考 |
|----|----------------|------|------|------|
| 1 | test_normal_1 | `<svg></svg>` を入力し、`svg` ルート・属性なし・子なしで抽出できることを確認する。 | 正常 | - |
| 2 | test_normal_2 | `svg` の複数属性（ダブルクォート・シングルクォート混在）を抽出できることを確認する。 | 正常 | - |
| 3 | test_normal_3 | `svg` 配下の入れ子要素（`g`→`path`）と兄弟要素（`rect`）を再帰的に抽出できることを確認する。 | 正常 | - |
| 4 | test_normal_4 | 自己終了タグ（`<g/>`, `<path .../>`）を子要素として抽出できることを確認する。 | 正常 | - |
| 5 | test_boundary_1 | XML宣言付き入力から `svg` ルートと子要素を抽出できることを確認する。 | 境界 | - |
| 6 | test_boundary_2 | 先頭・内部・末尾のコメントを無視して要素と属性を抽出できることを確認する。 | 境界 | - |
| 7 | test_boundary_3 | DOCTYPE宣言付き入力から `svg` と子要素を抽出できることを確認する。 | 境界 | - |
| 8 | test_boundary_4 | テキストノードを含む入力で、テキストを無視して要素のみ抽出できることを確認する。 | 境界 | - |
| 9 | test_abnormal_1 | 閉じタグの対応が不一致の入力で抽出失敗（`nullopt`）になることを確認する。 | 異常 | - |
| 10 | test_abnormal_2 | ルート要素が `svg` でない入力で抽出失敗（`nullopt`）になることを確認する。 | 異常 | - |
| 11 | test_abnormal_3 | 属性値がクォートされていない入力で抽出失敗（`nullopt`）になることを確認する。 | 異常 | - |
| 12 | test_abnormal_4 | 属性の `=` が欠落した入力で抽出失敗（`nullopt`）になることを確認する。 | 異常 | - |
| 13 | test_abnormal_5 | 開始タグが不完全な入力で抽出失敗（`nullopt`）になることを確認する。 | 異常 | - |

### テストデータ表

| No | テストデータ |
|----|--------------|
| 1 | xml_content=`<svg></svg>` |
| 2 | xml_content=`<svg width="24" height='16' viewBox="0 0 24 16"></svg>` |
| 3 | xml_content=`<svg><g id="layer"><path d="M0 0" /></g><rect width="10" height="20"></rect></svg>` |
| 4 | xml_content=`<svg><g/><path d="M1 1"/></svg>` |
| 5 | xml_content=`<?xml version="1.0" encoding="UTF-8"?>\n<svg><g/></svg>` |
| 6 | xml_content=`<!-- document comment -->\n<svg><!-- child comment --><g id="layer"/><!-- trailing child comment --></svg>\n<!-- after root comment -->` |
| 7 | xml_content=`<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">\n<svg><rect/></svg>` |
| 8 | xml_content=`<svg>\n  before\n  <g>inside-text<path d="M0 0"/>after-text</g>\n  tail\n</svg>` |
| 9 | xml_content=`<svg><g></svg></g>` |
| 10 | xml_content=`<g></g>` |
| 11 | xml_content=`<svg width=24></svg>` |
| 12 | xml_content=`<svg width "24"></svg>` |
| 13 | xml_content=`<svg` |
