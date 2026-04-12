| No | 対象ファイル | テストケース名 | 概要 | テストデータ(値) | 区分 | 備考 |
|----|--------------|----------------|------|------------------|------|------|
| 1 | tests/parser/XmlNodeExtractorTest.cpp | test_extract_minimal_svg | `<svg></svg>` を入力し、`svg` ルート・属性なし・子なしで抽出できることを確認する。 | xml_content=`<svg></svg>` | 正常 | - |
| 2 | tests/parser/XmlNodeExtractorTest.cpp | test_extract_svg_attributes | `svg` の複数属性（ダブルクォート・シングルクォート混在）を抽出できることを確認する。 | xml_content=`<svg width="24" height='16' viewBox="0 0 24 16"></svg>` | 正常 | - |
| 3 | tests/parser/XmlNodeExtractorTest.cpp | test_extract_children_recursively | `svg` 配下の入れ子要素（`g`→`path`）と兄弟要素（`rect`）を再帰的に抽出できることを確認する。 | xml_content=`<svg><g id="layer"><path d="M0 0" /></g><rect width="10" height="20"></rect></svg>` | 正常 | - |
| 4 | tests/parser/XmlNodeExtractorTest.cpp | test_extract_self_closing_nodes | 自己終了タグ（`<g/>`, `<path .../>`）を子要素として抽出できることを確認する。 | xml_content=`<svg><g/><path d="M1 1"/></svg>` | 正常 | - |
| 5 | tests/parser/XmlNodeExtractorTest.cpp | test_extract_svg_with_xml_declaration | XML宣言付き入力から `svg` ルートと子要素を抽出できることを確認する。 | xml_content=`<?xml version="1.0" encoding="UTF-8"?>\n<svg><g/></svg>` | 正常 | - |
| 6 | tests/parser/XmlNodeExtractorTest.cpp | test_extract_svg_ignoring_comments | 先頭・内部・末尾のコメントを無視して要素と属性を抽出できることを確認する。 | xml_content=`<!-- document comment -->\n<svg><!-- child comment --><g id="layer"/><!-- trailing child comment --></svg>\n<!-- after root comment -->` | 正常 | - |
| 7 | tests/parser/XmlNodeExtractorTest.cpp | test_extract_svg_with_doctype | DOCTYPE宣言付き入力から `svg` と子要素を抽出できることを確認する。 | xml_content=`<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">\n<svg><rect/></svg>` | 正常 | - |
| 8 | tests/parser/XmlNodeExtractorTest.cpp | test_extract_svg_ignoring_text_nodes | テキストノードを含む入力で、テキストを無視して要素のみ抽出できることを確認する。 | xml_content=`<svg>\n  before\n  <g>inside-text<path d="M0 0"/>after-text</g>\n  tail\n</svg>` | 正常 | - |
| 9 | tests/parser/XmlNodeExtractorTest.cpp | test_fail_on_mismatched_closing_tag | 閉じタグの対応が不一致の入力で抽出失敗（`nullopt`）になることを確認する。 | xml_content=`<svg><g></svg></g>` | 異常 | - |
| 10 | tests/parser/XmlNodeExtractorTest.cpp | test_fail_when_root_is_not_svg | ルート要素が `svg` でない入力で抽出失敗（`nullopt`）になることを確認する。 | xml_content=`<g></g>` | 異常 | - |
| 11 | tests/parser/XmlNodeExtractorTest.cpp | test_fail_when_attribute_value_is_not_quoted | 属性値がクォートされていない入力で抽出失敗（`nullopt`）になることを確認する。 | xml_content=`<svg width=24></svg>` | 異常 | - |
| 12 | tests/parser/XmlNodeExtractorTest.cpp | test_fail_when_attribute_equal_is_missing | 属性の `=` が欠落した入力で抽出失敗（`nullopt`）になることを確認する。 | xml_content=`<svg width "24"></svg>` | 異常 | - |
| 13 | tests/parser/XmlNodeExtractorTest.cpp | test_fail_when_start_tag_is_incomplete | 開始タグが不完全な入力で抽出失敗（`nullopt`）になることを確認する。 | xml_content=`<svg` | 異常 | - |
