# SvgPathParser 単体テストケース一覧

## 1. 対象
### 対象ファイル
- `src/parser/SvgPathParser.cpp`
- `src/parser/SvgPathParser.h`

### 対象テストファイル
- `tests/parser/SvgPathParserTest.cpp`

### 対象責務
`SvgPathParser::parse(const std::string& d)` が、SVG path 文字列を `std::vector<PathCommand>` に変換できること、および不正入力を `nullopt` として検出できることを確認する。

### 非対象
- 数値文字列の厳密な妥当性検証（数値変換・数値範囲）
- 生成された PathCommand を解釈・描画する後段処理
- `PathCommand` 以外の parser モジュール

## 2. 入力された観点メモ
- 各コマンド（`M/L/H/V/C/Z/S/Q/T/A`）の単体入力を受理できること。
- 複合入力（連結、カンマ区切り、区切り省略、`-` による分割、可変パラメータ数の連続）を受理できること。
- パラメータ不足・不要パラメータ・未対応コマンド・不正文字・空文字を異常として検出できること。
- 複合入力中の異常（後続コマンド不足、不正文字混入等）を検出できること。
- Parser 層としては数値妥当性を厳密判定せず、トークン形式上受理される文字列を保持できること（現実装挙動）。

## 3. テスト方針
- 区分は `正常 / 異常 / 境界` で整理する。
- 期待結果は `parse` の戻り値（`has_value` / `nullopt`）と、必要最小限の `PathCommand` 内容（`type` / `is_absolute` / `parameters`）に限定する。
- 異常ケースは `result == nullopt` を直接検証する。
- 境界ケースは「数値妥当性は別責務」の現実装挙動固定として、受理および文字列保持を確認する。
- 本ドキュメントは既存 `docs/test/path_parser_test_case.md` の観点を、承認ゲート対応フォーマットへ再構築したものである。

## 4. テスト対象I/F
| 種別 | 名称 | 概要 |
|---|---|---|
| クラス | `svg::parser::SvgPathParser` | SVG path 文字列をトークン化し、`PathCommand` 列へ変換する。 |
| 関数 | `SvgPathParser::parse(const std::string& d)` | 入力文字列を解析し、成功時 `std::optional<std::vector<PathCommand>>` を返す。 |

## 5. テストケース表
| No | 区分 | テストケース名 | 概要 | 観点 | 入力概要 | 期待結果 | 備考 |
|---:|---|---|---|---|---|---|---|
| 1 | 正常 | `test_normal_001` | MoveTo absolute を単体受理できる | コマンド単体 | `M 10 20` | `result.has_value() == true` かつ `commands.size()==1`、`type==MoveTo`、`is_absolute==true`、`parameters==["10","20"]` | - |
| 2 | 正常 | `test_normal_002` | LineTo relative を単体受理できる | コマンド単体 | `l 30 40` | `result.has_value() == true` かつ `commands.size()==1`、`type==LineTo`、`is_absolute==false`、`parameters==["30","40"]` | - |
| 3 | 正常 | `test_normal_003` | Horizontal を単体受理できる | コマンド単体 | `H 50` | `result.has_value() == true` かつ `commands.size()==1`、`type==HorizontalTo`、`parameters==["50"]` | - |
| 4 | 正常 | `test_normal_004` | Vertical を単体受理できる | コマンド単体 | `v 60` | `result.has_value() == true` かつ `commands.size()==1`、`type==VerticalTo`、`is_absolute==false`、`parameters==["60"]` | - |
| 5 | 正常 | `test_normal_005` | Cubic Bezier を単体受理できる | コマンド単体 | `C 1 2 3 4 5 6` | `result.has_value() == true` かつ `commands.size()==1`、`type==CubicBezierTo`、`parameters.size()==6` | - |
| 6 | 正常 | `test_normal_006` | ClosePath を単体受理できる | コマンド単体 | `Z` | `result.has_value() == true` かつ `commands.size()==1`、`type==ClosePath`、`parameters.empty()==true` | - |
| 7 | 正常 | `test_normal_007` | Smooth Cubic を単体受理できる | コマンド単体 | `s 1 2 3 4` | `result.has_value() == true` かつ `commands.size()==1`、`type==SmoothCubicBezierTo`、`is_absolute==false`、`parameters.size()==4` | - |
| 8 | 正常 | `test_normal_008` | Quadratic を単体受理できる | コマンド単体 | `Q 1 2 3 4` | `result.has_value() == true` かつ `commands.size()==1`、`type==QuadraticBezierTo`、`parameters.size()==4` | - |
| 9 | 正常 | `test_normal_009` | Smooth Quadratic を単体受理できる | コマンド単体 | `t 7 8` | `result.has_value() == true` かつ `commands.size()==1`、`type==SmoothQuadraticBezierTo`、`is_absolute==false`、`parameters.size()==2` | - |
| 10 | 正常 | `test_normal_010` | Arc を単体受理できる | コマンド単体 | `A 1 2 0 0 1 30 40` | `result.has_value() == true` かつ `commands.size()==1`、`type==ArcTo`、`parameters.size()==7` | - |
| 11 | 正常 | `test_normal_011` | MoveTo と LineTo の連結を受理できる | 複合入力 | `M10 20 L30 40` | `result.has_value() == true` かつ `commands.size()==2`、1件目 `MoveTo`、2件目 `LineTo` | - |
| 12 | 正常 | `test_normal_012` | ClosePath を含む複合入力を受理できる | 複合入力 | `M 10 20 L 30 40 Z` | `result.has_value() == true` かつ `commands.size()==3`、3件目 `ClosePath` で `parameters.empty()==true` | - |
| 13 | 正常 | `test_normal_013` | カンマ区切り複合入力を受理できる | 複合入力 | `M10,20L30,40` | `result.has_value() == true` かつ `commands.size()==2`、各コマンドの `parameters.size()==2` | - |
| 14 | 正常 | `test_normal_014` | 区切りなしコマンド連結を受理できる | 複合入力 | `M10 20H30V40` | `result.has_value() == true` かつ `commands.size()==3`、`MoveTo`→`HorizontalTo`→`VerticalTo` の順 | - |
| 15 | 正常 | `test_normal_015` | `-` によるパラメータ分割を受理できる | 複合入力 | `M10-20L30-40` | `result.has_value() == true` かつ 1件目 `parameters==["10","-20"]`、2件目 `parameters==["30","-40"]` | - |
| 16 | 正常 | `test_normal_016` | 異なる必要パラメータ数の連続遷移を受理できる | 複合入力 | `M10 20L30 40H50` | `result.has_value() == true` かつ `commands.size()==3`、`parameters.size()==2,2,1` | - |
| 17 | 異常 | `test_abnormal_001` | MoveTo のパラメータ不足を検出できる | パラメータ不足 | `M 10` | `result == nullopt` | - |
| 18 | 異常 | `test_abnormal_002` | LineTo のパラメータ不足を検出できる | パラメータ不足 | `L 10` | `result == nullopt` | - |
| 19 | 異常 | `test_abnormal_003` | Horizontal のパラメータ不足を検出できる | パラメータ不足 | `H` | `result == nullopt` | - |
| 20 | 異常 | `test_abnormal_004` | Vertical のパラメータ不足を検出できる | パラメータ不足 | `V` | `result == nullopt` | - |
| 21 | 異常 | `test_abnormal_005` | Cubic Bezier のパラメータ不足を検出できる | パラメータ不足 | `C 1 2 3 4 5` | `result == nullopt` | - |
| 22 | 異常 | `test_abnormal_006` | ClosePath の不要パラメータを検出できる | パラメータ不正 | `Z 1` | `result == nullopt` | - |
| 23 | 異常 | `test_abnormal_007` | Smooth Cubic のパラメータ不足を検出できる | パラメータ不足 | `S 1 2 3` | `result == nullopt` | - |
| 24 | 異常 | `test_abnormal_008` | Quadratic のパラメータ不足を検出できる | パラメータ不足 | `Q 1 2 3` | `result == nullopt` | - |
| 25 | 異常 | `test_abnormal_009` | Smooth Quadratic のパラメータ不足を検出できる | パラメータ不足 | `T 1` | `result == nullopt` | - |
| 26 | 異常 | `test_abnormal_010` | Arc のパラメータ不足を検出できる | パラメータ不足 | `A 1 2 0 0 1 30` | `result == nullopt` | - |
| 27 | 異常 | `test_abnormal_011` | 先頭不正文字を検出できる | 不正文字 | `@ 10 20` | `result == nullopt` | - |
| 28 | 異常 | `test_abnormal_012` | トークン開始位置の不正文字を検出できる | 不正文字 | `M @ 20` | `result == nullopt` | - |
| 29 | 異常 | `test_abnormal_013` | パラメータ間の不正文字を検出できる | 不正文字 | `M 10 ? 20` | `result == nullopt` | - |
| 30 | 異常 | `test_abnormal_014` | 未対応コマンドを検出できる | 未対応コマンド | `X 10 20` | `result == nullopt` | - |
| 31 | 異常 | `test_abnormal_015` | コマンド欠如を検出できる | 構文不正 | `10 20` | `result == nullopt` | - |
| 32 | 異常 | `test_abnormal_016` | 空文字入力を検出できる | 構文不正 | ``（空文字）`` | `result == nullopt` | - |
| 33 | 異常 | `test_abnormal_017` | 実質的にパラメータが存在しない入力を検出できる | 構文不正 | `M , ,` | `result == nullopt` | - |
| 34 | 異常 | `test_abnormal_018` | パラメータ末尾の不正文字を検出できる | 不正文字 | `M 10 #` | `result == nullopt` | - |
| 35 | 異常 | `test_abnormal_019` | 複合入力で後続コマンドのパラメータ不足を検出できる | 複合異常 | `M10 20 L30` | `result == nullopt` | - |
| 36 | 異常 | `test_abnormal_020` | 複合入力で ClosePath の不要パラメータを検出できる | 複合異常 | `M10 20 Z 1` | `result == nullopt` | - |
| 37 | 異常 | `test_abnormal_021` | 複合入力で未対応コマンド混入を検出できる | 複合異常 | `M10 20 X30 40` | `result == nullopt` | - |
| 38 | 異常 | `test_abnormal_022` | 複合入力で不正文字混入を検出できる | 複合異常 | `M10 20 ? 30 40` | `result == nullopt` | - |
| 39 | 異常 | `test_abnormal_023` | 複合入力で途中パラメータ不正を検出できる | 複合異常 | `M10 20 L30 x40` | `result == nullopt` | - |
| 40 | 境界 | `test_boundary_001` | 非数値文字混在パラメータを現実装として受理できる | 現実装許容 | `M1&0 20` | `result.has_value() == true` かつ `parameters==["1&0","20"]` | - |
| 41 | 境界 | `test_boundary_002` | 英字混在パラメータを現実装として受理できる | 現実装許容 | `M10x20 30` | `result.has_value() == true` かつ `parameters==["10x20","30"]` | - |
| 42 | 境界 | `test_boundary_003` | 記号混在パラメータを現実装として受理できる | 現実装許容 | `M1_0 20` | `result.has_value() == true` かつ `parameters==["1_0","20"]` | - |
| 43 | 境界 | `test_boundary_004` | Cubic で非数値混在でも個数一致なら受理できる | 現実装許容 | `C1x2 3 4 5 6 7` | `result.has_value() == true` かつ `type==CubicBezierTo`、`parameters==["1x2","3","4","5","6","7"]` | - |
| 44 | 境界 | `test_boundary_005` | 指数表記風文字列を現実装として受理できる | 現実装許容 | `M1e3 20` | `result.has_value() == true` かつ `parameters==["1e3","20"]` | - |

## 6. テストデータ表
| No | テストデータ | 期待データ | 補足 |
|---:|---|---|---|
| 1 | `M 10 20` | `MoveTo(abs), params=["10","20"]` | 単体 |
| 2 | `l 30 40` | `LineTo(rel), params=["30","40"]` | 単体 |
| 3 | `H 50` | `HorizontalTo(abs), params=["50"]` | 単体 |
| 4 | `v 60` | `VerticalTo(rel), params=["60"]` | 単体 |
| 5 | `C 1 2 3 4 5 6` | `CubicBezierTo(abs), params.size()==6` | 単体 |
| 6 | `Z` | `ClosePath(abs), params.empty()==true` | 単体 |
| 7 | `s 1 2 3 4` | `SmoothCubicBezierTo(rel), params.size()==4` | 単体 |
| 8 | `Q 1 2 3 4` | `QuadraticBezierTo(abs), params.size()==4` | 単体 |
| 9 | `t 7 8` | `SmoothQuadraticBezierTo(rel), params.size()==2` | 単体 |
| 10 | `A 1 2 0 0 1 30 40` | `ArcTo(abs), params.size()==7` | 単体 |
| 11 | `M10 20 L30 40` | `commands.size()==2` | 複合 |
| 12 | `M 10 20 L 30 40 Z` | `commands.size()==3 (末尾 ClosePath)` | 複合 |
| 13 | `M10,20L30,40` | `commands.size()==2` | 複合 |
| 14 | `M10 20H30V40` | `MoveTo→HorizontalTo→VerticalTo` | 複合 |
| 15 | `M10-20L30-40` | `"-" で分割される` | 複合 |
| 16 | `M10 20L30 40H50` | `params.size()==2,2,1` | 複合 |
| 17 | `M 10` | `nullopt` | パラメータ不足 |
| 18 | `L 10` | `nullopt` | パラメータ不足 |
| 19 | `H` | `nullopt` | パラメータ不足 |
| 20 | `V` | `nullopt` | パラメータ不足 |
| 21 | `C 1 2 3 4 5` | `nullopt` | パラメータ不足 |
| 22 | `Z 1` | `nullopt` | 不要パラメータ |
| 23 | `S 1 2 3` | `nullopt` | パラメータ不足 |
| 24 | `Q 1 2 3` | `nullopt` | パラメータ不足 |
| 25 | `T 1` | `nullopt` | パラメータ不足 |
| 26 | `A 1 2 0 0 1 30` | `nullopt` | パラメータ不足 |
| 27 | `@ 10 20` | `nullopt` | 不正文字 |
| 28 | `M @ 20` | `nullopt` | 不正文字 |
| 29 | `M 10 ? 20` | `nullopt` | 不正文字 |
| 30 | `X 10 20` | `nullopt` | 未対応コマンド |
| 31 | `10 20` | `nullopt` | コマンド欠如 |
| 32 | ``（空文字）`` | `nullopt` | 空入力 |
| 33 | `M , ,` | `nullopt` | パラメータ欠如 |
| 34 | `M 10 #` | `nullopt` | 不正文字 |
| 35 | `M10 20 L30` | `nullopt` | 複合異常 |
| 36 | `M10 20 Z 1` | `nullopt` | 複合異常 |
| 37 | `M10 20 X30 40` | `nullopt` | 複合異常 |
| 38 | `M10 20 ? 30 40` | `nullopt` | 複合異常 |
| 39 | `M10 20 L30 x40` | `nullopt` | 複合異常 |
| 40 | `M1&0 20` | `MoveTo(abs), params=["1&0","20"]` | 境界 |
| 41 | `M10x20 30` | `MoveTo(abs), params=["10x20","30"]` | 境界 |
| 42 | `M1_0 20` | `MoveTo(abs), params=["1_0","20"]` | 境界 |
| 43 | `C1x2 3 4 5 6 7` | `CubicBezierTo(abs), params.size()==6` | 境界 |
| 44 | `M1e3 20` | `MoveTo(abs), params=["1e3","20"]` | 境界 |

## 7. 保留・確認事項
| No | 内容 | 理由 | 対応方針 |
|---:|---|---|---|
| - | なし | - | - |
