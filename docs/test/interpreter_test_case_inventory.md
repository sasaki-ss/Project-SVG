# Interpreter テストケース一覧

## 対象責務
ノード構造から SVG ルート情報および図形情報を解釈する処理に関する既存テストケース一覧。
要素解釈・属性解釈・エラーハンドリングに関わる観点のみを対象とする。

## tests/interpreter/SvgInterpreterTest.cpp

### テストケース表

| No | テストケース名 | 概要 | 区分 | 備考 |
|----|----------------|------|------|------|
| 1 | test_normal_1 | 必須ルート属性のみを持つ `svg` を解釈できることを確認する。 | 正常 | - |
| 2 | test_normal_2 | ルート属性値（viewBox/width/height/style）が `InterpretedSvg` に反映されることを確認する。 | 正常 | - |
| 3 | test_normal_3 | 子要素（`g` 内 `path` と直下 `circle`）から図形が収集されることを確認する。 | 正常 | - |
| 4 | test_normal_4 | `path` 要素の `d` が保持されて解釈できることを確認する。 | 正常 | - |
| 5 | test_normal_5 | `circle` 要素の `cx/cy/r` を数値として解釈できることを確認する。 | 正常 | - |
| 6 | test_normal_6 | `rect` 要素で `rx/ry` なしのケースを解釈できることを確認する。 | 正常 | - |
| 7 | test_normal_7 | `rect` 要素で `rx/ry` ありのケースを解釈できることを確認する。 | 正常 | - |
| 8 | test_normal_8 | `line` 要素の `x1/y1/x2/y2` を数値として解釈できることを確認する。 | 正常 | - |
| 9 | test_normal_9 | `ellipse` 要素の `cx/cy/rx/ry` を数値として解釈できることを確認する。 | 正常 | - |
| 10 | test_normal_10 | `polyline` の空白区切り `points` を点列へ解釈できることを確認する。 | 正常 | - |
| 11 | test_normal_11 | `polygon` のカンマ区切り `points` を点列へ解釈できることを確認する。 | 正常 | - |
| 12 | test_abnormal_1 | ルート要素名が `svg` 以外（`g`）の場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 13 | test_abnormal_2 | `viewBox` の値数が不足している場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 14 | test_abnormal_3 | 必須属性（このテストでは `width`）欠落時に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 15 | test_abnormal_4 | `stroke-linecap` に未対応値（`square`）を指定した場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 16 | test_abnormal_5 | `stroke-linejoin` に未対応値（`bevel`）を指定した場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 17 | test_abnormal_6 | 未対応要素（`g`）を図形解釈した場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 18 | test_abnormal_7 | `path` 要素で必須属性 `d` 欠落時に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 19 | test_abnormal_8 | `circle` 要素で必須属性（このテストでは `cy`）欠落時に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 20 | test_abnormal_9 | `circle` 要素で数値属性に非数値文字列（`cx=invalid`）を指定した場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 21 | test_abnormal_10 | `rect` 要素で必須属性（このテストでは `y`）欠落時に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 22 | test_abnormal_11 | `rect` 要素で `rx` に非数値文字列を指定した場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 23 | test_abnormal_12 | `rect` 要素で `ry` に非数値文字列を指定した場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 24 | test_abnormal_13 | `line` 要素で必須属性（このテストでは `y1`）欠落時に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 25 | test_abnormal_14 | `line` 要素で数値属性に非数値文字列（`y1=invalid`）を指定した場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 26 | test_abnormal_15 | `ellipse` 要素で必須属性（このテストでは `ry`）欠落時に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 27 | test_abnormal_16 | `ellipse` 要素で数値属性に非数値文字列（`rx=invalid`）を指定した場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 28 | test_abnormal_17 | `polyline` 要素で `points` 欠落時に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 29 | test_abnormal_18 | `polyline` の `points` が奇数個トークン（`1 2 3`）の場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 30 | test_abnormal_19 | `polyline` の `points` に非数値トークン（`x`）を含む場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 31 | test_abnormal_20 | `polygon` 要素で `points` 欠落時に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 32 | test_abnormal_21 | `polygon` の `points` が奇数個トークン（`7 8 9`）の場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |
| 33 | test_abnormal_22 | `polygon` の `points` に非数値トークン（`invalid`）を含む場合に解釈失敗（`nullopt`）となることを確認する。 | 異常 | - |

### テストデータ表

| No | テストデータ |
|----|--------------|
| 1 | element_name=`svg`, attributes={viewBox=`0 0 24 24`, width=`24`, height=`24`, stroke=`currentColor`, fill=`none`, stroke-width=`2`, stroke-linecap=`round`, stroke-linejoin=`round`} |
| 2 | element_name=`svg`, attributes={viewBox=`1 2 30 40`, width=`30`, height=`40`, stroke=`#333`, fill=`none`, stroke-width=`1.5`, stroke-linecap=`round`, stroke-linejoin=`round`} |
| 3 | root=`svg`(必須属性), children={g{path(d=`M1 1L2 2`)}, circle(cx=`3`, cy=`4`, r=`5`)} |
| 4 | element_name=`path`, attributes={d=`M0 0L10 10`} |
| 5 | element_name=`circle`, attributes={cx=`5`, cy=`6`, r=`7`} |
| 6 | element_name=`rect`, attributes={x=`1`, y=`2`, width=`10`, height=`20`} |
| 7 | element_name=`rect`, attributes={x=`1`, y=`2`, width=`10`, height=`20`, rx=`3`, ry=`4`} |
| 8 | element_name=`line`, attributes={x1=`1`, y1=`2`, x2=`3`, y2=`4`} |
| 9 | element_name=`ellipse`, attributes={cx=`3`, cy=`4`, rx=`5`, ry=`6`} |
| 10 | element_name=`polyline`, attributes={points=`1 2 3 4 5 6`} |
| 11 | element_name=`polygon`, attributes={points=`7,8 9,10 11,12`} |
| 12 | element_name=`g`, attributes={viewBox=`0 0 24 24`, width=`24`, height=`24`, stroke=`currentColor`, fill=`none`, stroke-width=`2`, stroke-linecap=`round`, stroke-linejoin=`round`} |
| 13 | element_name=`svg`, attributes={viewBox=`0 0 24`, width=`24`, height=`24`, stroke=`currentColor`, fill=`none`, stroke-width=`2`, stroke-linecap=`round`, stroke-linejoin=`round`} |
| 14 | element_name=`svg`, attributes={viewBox=`0 0 24 24`, height=`24`, stroke=`currentColor`, fill=`none`, stroke-width=`2`, stroke-linecap=`round`, stroke-linejoin=`round`} |
| 15 | element_name=`svg`, attributes={viewBox=`0 0 24 24`, width=`24`, height=`24`, stroke=`currentColor`, fill=`none`, stroke-width=`2`, stroke-linecap=`square`, stroke-linejoin=`round`} |
| 16 | element_name=`svg`, attributes={viewBox=`0 0 24 24`, width=`24`, height=`24`, stroke=`currentColor`, fill=`none`, stroke-width=`2`, stroke-linecap=`round`, stroke-linejoin=`bevel`} |
| 17 | element_name=`g`, attributes={} |
| 18 | element_name=`path`, attributes={} |
| 19 | element_name=`circle`, attributes={cx=`5`, r=`7`} |
| 20 | element_name=`circle`, attributes={cx=`invalid`, cy=`6`, r=`7`} |
| 21 | element_name=`rect`, attributes={x=`1`, width=`10`, height=`20`} |
| 22 | element_name=`rect`, attributes={x=`1`, y=`2`, width=`10`, height=`20`, rx=`invalid`} |
| 23 | element_name=`rect`, attributes={x=`1`, y=`2`, width=`10`, height=`20`, ry=`invalid`} |
| 24 | element_name=`line`, attributes={x1=`1`, x2=`3`, y2=`4`} |
| 25 | element_name=`line`, attributes={x1=`1`, y1=`invalid`, x2=`3`, y2=`4`} |
| 26 | element_name=`ellipse`, attributes={cx=`3`, cy=`4`, rx=`5`} |
| 27 | element_name=`ellipse`, attributes={cx=`3`, cy=`4`, rx=`invalid`, ry=`6`} |
| 28 | element_name=`polyline`, attributes={} |
| 29 | element_name=`polyline`, attributes={points=`1 2 3`} |
| 30 | element_name=`polyline`, attributes={points=`1 2 x 4`} |
| 31 | element_name=`polygon`, attributes={} |
| 32 | element_name=`polygon`, attributes={points=`7 8 9`} |
| 33 | element_name=`polygon`, attributes={points=`7 8 invalid 10`} |
