# SVG path コマンド別設計メモ

## 目的

本資料は、`SvgPathInterpreter` の設計判断に使用するため、SVG path コマンドごとに以下を整理する。

- コマンドが持つパラメータ
- 絶対座標の場合の扱い
- 相対座標の場合の変換方法
- 特殊条件
- Project SVG における設計判断ポイント

---

## 前提

SVG path コマンドは、大文字が絶対座標、小文字が相対座標を表す。

| 絶対 | 相対 | 意味 |
|---|---|---|
| `M` | `m` | MoveTo |
| `L` | `l` | LineTo |
| `H` | `h` | HorizontalTo |
| `V` | `v` | VerticalTo |
| `C` | `c` | CubicBezierTo |
| `S` | `s` | SmoothCubicBezierTo |
| `Q` | `q` | QuadraticBezierTo |
| `T` | `t` | SmoothQuadraticBezierTo |
| `A` | `a` | ArcTo |
| `Z` | `z` | ClosePath |

`SvgPathInterpreter` では、`SvgPathParser` が生成した `PathCommand` 列を受け取り、描画・上位保持に利用できる `PathInstruction` 列へ変換する。

Project SVG では、`SvgPathInterpreter` の出口では原則として座標を絶対座標へ正規化する。

相対座標の基本式は以下とする。

- `absolute_x = current_x + dx`
- `absolute_y = current_y + dy`

`current_point` は現在位置を表す。  
`subpath_start_point` は現在の subpath の開始位置を表し、`Z/z` で戻る先になる。

---

## SvgPathInterpreter で吸収したいこと

`SvgPathInterpreter` では、以下を吸収する。

- `PathCommand` の文字列パラメータを数値へ変換する
- 相対座標を絶対座標へ変換する
- `M/m` の2組目以降を `LineTo` に変換する
- `H/h` を `LineTo` に変換する
- `V/v` を `LineTo` に変換する
- `current_point` を管理する
- `subpath_start_point` を管理する
- 必要に応じて前回の制御点を管理する

`SvgPathInterpreter` より上位の層では、相対座標や `H/V` の差異を意識しない方針とする。

---

## 現状の PathInstructionType

現状の `PathInstructionType` は以下を想定する。

| 種別 | 用途 |
|---|---|
| `MoveTo` | 現在位置を移動する |
| `LineTo` | 現在位置から指定点へ直線を引く |
| `CubicBezierTo` | 現在位置から3次ベジェ曲線を引く |
| `ClosePath` | 現在の subpath を閉じる |

この前提では、`H/h` と `V/v` は `LineTo` に正規化する。  
`S/s` は `CubicBezierTo` に正規化可能。  
`Q/q`、`T/t`、`A/a` は追加設計判断が必要。

---

# 1. M / m: MoveTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `M x y` | 絶対座標の MoveTo |
| `m dx dy` | 相対座標の MoveTo |

2個で1組。

| パラメータ | 意味 |
|---|---|
| `x` / `dx` | 移動先X座標 / 現在座標からのX差分 |
| `y` / `dy` | 移動先Y座標 / 現在座標からのY差分 |

## 絶対座標

`M x y` は `MoveTo(x, y)` として扱う。

## 相対座標

`m dx dy` は `MoveTo(current_x + dx, current_y + dy)` として扱う。

ただし、path先頭などで `current_point` が未設定の場合は `(0, 0)` 基準で扱う。

その場合は `MoveTo(dx, dy)` になる。

## 特殊条件

`M/m` は複数ペアを持てる。

例: `M 10 10 20 20 30 30`

この場合、1組目だけ `MoveTo` として扱う。  
2組目以降は暗黙的に `LineTo` として扱う。

- `MoveTo(10, 10)`
- `LineTo(20, 20)`
- `LineTo(30, 30)`

`M/m` の1組目を処理したタイミングで、以下を更新する。

- `current_point = move_to_point`
- `subpath_start_point = move_to_point`

2組目以降の `LineTo` では、`current_point` のみ更新する。

---

# 2. L / l: LineTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `L x y` | 絶対座標の LineTo |
| `l dx dy` | 相対座標の LineTo |

2個で1組。

| パラメータ | 意味 |
|---|---|
| `x` / `dx` | 線の終点X座標 / 現在座標からのX差分 |
| `y` / `dy` | 線の終点Y座標 / 現在座標からのY差分 |

## 絶対座標

`L x y` は `LineTo(x, y)` として扱う。

## 相対座標

`l dx dy` は `LineTo(current_x + dx, current_y + dy)` として扱う。

## 特殊条件

`current_point` が必要。

`M/m` なしで `L/l` が来た場合は不正扱いでよい。

複数ペアを持てる。

例: `L 10 10 20 20`

- `LineTo(10, 10)`
- `LineTo(20, 20)`

処理後は以下を更新する。

- `current_point = line_to_point`

---

# 3. H / h: HorizontalTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `H x` | 絶対座標の HorizontalTo |
| `h dx` | 相対座標の HorizontalTo |

1個で1組。

| パラメータ | 意味 |
|---|---|
| `x` / `dx` | X座標 / 現在座標からのX差分 |

Y座標は `current_point.y` を使う。

## 絶対座標

`H x` は `LineTo(x, current_y)` として扱う。

## 相対座標

`h dx` は `LineTo(current_x + dx, current_y)` として扱う。

## 特殊条件

`PathInstructionType` に `HorizontalTo` を持たせない場合、`LineTo` に正規化する。

`current_point` が必要。

複数パラメータを持てる。

例: `H 10 20`

- `LineTo(10, current_y)`
- `LineTo(20, current_y)`

ただし2つ目以降の `current_y` は、直前の処理後の現在座標のYを使う。  
HorizontalTo ではYは変化しないため、通常は同じYになる。

処理後は以下を更新する。

- `current_point = line_to_point`

---

# 4. V / v: VerticalTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `V y` | 絶対座標の VerticalTo |
| `v dy` | 相対座標の VerticalTo |

1個で1組。

| パラメータ | 意味 |
|---|---|
| `y` / `dy` | Y座標 / 現在座標からのY差分 |

X座標は `current_point.x` を使う。

## 絶対座標

`V y` は `LineTo(current_x, y)` として扱う。

## 相対座標

`v dy` は `LineTo(current_x, current_y + dy)` として扱う。

## 特殊条件

`PathInstructionType` に `VerticalTo` を持たせない場合、`LineTo` に正規化する。

`current_point` が必要。

複数パラメータを持てる。

例: `V 10 20`

- `LineTo(current_x, 10)`
- `LineTo(current_x, 20)`

ただし2つ目以降の `current_x` は、直前の処理後の現在座標のXを使う。  
VerticalTo ではXは変化しないため、通常は同じXになる。

処理後は以下を更新する。

- `current_point = line_to_point`

---

# 5. C / c: CubicBezierTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `C x1 y1 x2 y2 x y` | 絶対座標の CubicBezierTo |
| `c dx1 dy1 dx2 dy2 dx dy` | 相対座標の CubicBezierTo |

6個で1組。

| パラメータ | 意味 |
|---|---|
| `x1` / `dx1` | 第1制御点X |
| `y1` / `dy1` | 第1制御点Y |
| `x2` / `dx2` | 第2制御点X |
| `y2` / `dy2` | 第2制御点Y |
| `x` / `dx` | 終点X |
| `y` / `dy` | 終点Y |

## 絶対座標

`C x1 y1 x2 y2 x y` は以下として扱う。

- `control1 = (x1, y1)`
- `control2 = (x2, y2)`
- `end = (x, y)`

## 相対座標

`c dx1 dy1 dx2 dy2 dx dy` は以下として扱う。

- `control1 = (current_x + dx1, current_y + dy1)`
- `control2 = (current_x + dx2, current_y + dy2)`
- `end = (current_x + dx, current_y + dy)`

## 特殊条件

`current_point` が必要。

複数組を持てる。

処理後は以下を更新する。

- `current_point = end`

`S/s` 対応を考える場合は、第2制御点を記録する必要がある。

- `previous_cubic_control2 = control2`

直後のコマンドが `S/s` でない場合でも、次コマンド判定のために「直前が Cubic 系かどうか」を管理する必要がある。

---

# 6. S / s: SmoothCubicBezierTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `S x2 y2 x y` | 絶対座標の SmoothCubicBezierTo |
| `s dx2 dy2 dx dy` | 相対座標の SmoothCubicBezierTo |

4個で1組。

| パラメータ | 意味 |
|---|---|
| `x2` / `dx2` | 第2制御点X |
| `y2` / `dy2` | 第2制御点Y |
| `x` / `dx` | 終点X |
| `y` / `dy` | 終点Y |

第1制御点はパラメータに存在しない。  
直前の Cubic 系コマンドから自動計算する。

## 第1制御点の計算

直前が `C/c` または `S/s` の場合、直前の第2制御点を現在座標を中心に反転する。

- `control1.x = 2 * current_x - previous_cubic_control2.x`
- `control1.y = 2 * current_y - previous_cubic_control2.y`

直前が Cubic 系でない場合は、現在座標を第1制御点とする。

- `control1 = current_point`

## 絶対座標

`S x2 y2 x y` は以下として扱う。

- `control1 = reflected_control_point`
- `control2 = (x2, y2)`
- `end = (x, y)`

## 相対座標

`s dx2 dy2 dx dy` は以下として扱う。

- `control1 = reflected_control_point`
- `control2 = (current_x + dx2, current_y + dy2)`
- `end = (current_x + dx, current_y + dy)`

## 特殊条件

`current_point` が必要。

現状の `PathInstructionType` が `CubicBezierTo` を持っているため、`S/s` は `CubicBezierTo` に正規化できる。

処理後は以下を更新する。

- `current_point = end`
- `previous_cubic_control2 = control2`

また、次コマンドのために「直前が Cubic 系である」状態を保持する。

---

# 7. Q / q: QuadraticBezierTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `Q x1 y1 x y` | 絶対座標の QuadraticBezierTo |
| `q dx1 dy1 dx dy` | 相対座標の QuadraticBezierTo |

4個で1組。

| パラメータ | 意味 |
|---|---|
| `x1` / `dx1` | 制御点X |
| `y1` / `dy1` | 制御点Y |
| `x` / `dx` | 終点X |
| `y` / `dy` | 終点Y |

## 絶対座標

`Q x1 y1 x y` は以下として扱う。

- `control = (x1, y1)`
- `end = (x, y)`

## 相対座標

`q dx1 dy1 dx dy` は以下として扱う。

- `control = (current_x + dx1, current_y + dy1)`
- `end = (current_x + dx, current_y + dy)`

## 特殊条件

`current_point` が必要。

複数組を持てる。

現状の `PathInstructionType` に `QuadraticBezierTo` がないため、設計判断が必要。

候補は以下。

1. `PathInstructionType::QuadraticBezierTo` を追加する
2. `CubicBezierTo` に変換する
3. MVPでは未対応にする

`CubicBezierTo` に変換する場合は以下。

- `P0 = current_point`
- `Q1 = quadratic_control`
- `P2 = end`
- `C1 = P0 + 2/3 * (Q1 - P0)`
- `C2 = P2 + 2/3 * (Q1 - P2)`

成分で書くと以下。

- `cubic_control1.x = P0.x + (2.0 / 3.0) * (Q1.x - P0.x)`
- `cubic_control1.y = P0.y + (2.0 / 3.0) * (Q1.y - P0.y)`
- `cubic_control2.x = P2.x + (2.0 / 3.0) * (Q1.x - P2.x)`
- `cubic_control2.y = P2.y + (2.0 / 3.0) * (Q1.y - P2.y)`

処理後は以下を更新する。

- `current_point = end`
- `previous_quadratic_control = control`

また、次コマンドのために「直前が Quadratic 系である」状態を保持する。

---

# 8. T / t: SmoothQuadraticBezierTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `T x y` | 絶対座標の SmoothQuadraticBezierTo |
| `t dx dy` | 相対座標の SmoothQuadraticBezierTo |

2個で1組。

| パラメータ | 意味 |
|---|---|
| `x` / `dx` | 終点X |
| `y` / `dy` | 終点Y |

制御点はパラメータに存在しない。  
直前の Quadratic 系コマンドから自動計算する。

## 制御点の計算

直前が `Q/q` または `T/t` の場合、直前の Quadratic 制御点を現在座標を中心に反転する。

- `control.x = 2 * current_x - previous_quadratic_control.x`
- `control.y = 2 * current_y - previous_quadratic_control.y`

直前が Quadratic 系でない場合は、現在座標を制御点とする。

- `control = current_point`

## 絶対座標

`T x y` は以下として扱う。

- `end = (x, y)`

## 相対座標

`t dx dy` は以下として扱う。

- `end = (current_x + dx, current_y + dy)`

## 特殊条件

`current_point` が必要。

現状の `PathInstructionType` に `QuadraticBezierTo` がないため、`Q/q` と同じ設計判断が必要。

`CubicBezierTo` に変換する場合は以下。

- `P0 = current_point`
- `Q1 = reflected_quadratic_control`
- `P2 = end`
- `C1 = P0 + 2/3 * (Q1 - P0)`
- `C2 = P2 + 2/3 * (Q1 - P2)`

処理後は以下を更新する。

- `current_point = end`
- `previous_quadratic_control = control`

また、次コマンドのために「直前が Quadratic 系である」状態を保持する。

---

# 9. A / a: ArcTo

## パラメータ

| 形式 | 意味 |
|---|---|
| `A rx ry x_axis_rotation large_arc_flag sweep_flag x y` | 絶対座標の ArcTo |
| `a rx ry x_axis_rotation large_arc_flag sweep_flag dx dy` | 相対座標の ArcTo |

7個で1組。

| パラメータ | 意味 |
|---|---|
| `rx` | 楕円のX半径 |
| `ry` | 楕円のY半径 |
| `x_axis_rotation` | 楕円のX軸回転角度 |
| `large_arc_flag` | 大きい弧を使うか |
| `sweep_flag` | 弧の向き |
| `x` / `dx` | 終点X |
| `y` / `dy` | 終点Y |

## 絶対座標

`A rx ry rotation large_arc_flag sweep_flag x y` は以下として扱う。

- `end = (x, y)`

## 相対座標

`a rx ry rotation large_arc_flag sweep_flag dx dy` は以下として扱う。

- `end = (current_x + dx, current_y + dy)`

相対指定で変換対象になるのは終点のみ。

以下は `current_point` に加算しない。

- `rx`
- `ry`
- `x_axis_rotation`
- `large_arc_flag`
- `sweep_flag`

## 特殊条件

`current_point` が必要。

`large_arc_flag` と `sweep_flag` は `0` または `1` のみ。

- `large_arc_flag = 0 or 1`
- `sweep_flag = 0 or 1`

Arc は他コマンドより設計判断が重い。

候補は以下。

1. `PathInstructionType::ArcTo` を追加して保持する
2. `CubicBezierTo` に分割変換する
3. MVPでは未対応にする

処理後は以下を更新する。

- `current_point = end`

Arc の描画や CubicBezier 分割は実装コストが高いため、`SvgPathInterpreter` の初期実装から分離する判断もあり。

---

# 10. Z / z: ClosePath

## パラメータ

| 形式 | 意味 |
|---|---|
| `Z` | ClosePath |
| `z` | ClosePath |

パラメータなし。

## 絶対座標 / 相対座標

`Z/z` は座標パラメータを持たないため、大文字・小文字の差による計算差はない。

`ClosePath()` として扱う。

## 計算

現在の subpath の開始点へ戻る。

- `current_point = subpath_start_point`

## 特殊条件

`subpath_start_point` が必要。  
通常は `M/m` によって設定される。

`PathInstruction` としては `ClosePath` を追加する。

現状の `PathInstruction` では以下でよい。

- `type = ClosePath`
- `points = 空`

---

# 初期実装範囲の判断

## 無理なく対応できる範囲

まず無理なく対応できる範囲は以下。

- `M/m`
- `L/l`
- `H/h`
- `V/v`
- `C/c`
- `Z/z`

理由は以下。

- 現状の `PathInstructionType` で表現できる
- 相対座標から絶対座標への変換が単純
- `H/h` と `V/v` は `LineTo` へ正規化できる
- `M/m` の暗黙 `LineTo` も同じ仕組みで処理できる

## 追加検討しやすい範囲

追加検討しやすい範囲は以下。

- `S/s`

理由は以下。

- `CubicBezierTo` に正規化できる
- 追加で必要なのは直前の Cubic 系制御点管理のみ
- `PathInstructionType` の追加なしで対応できる

## 設計判断を分けた方がよい範囲

設計判断を分けた方がよい範囲は以下。

- `Q/q`
- `T/t`
- `A/a`

理由は以下。

- `Q/q` と `T/t` は `QuadraticBezierTo` を追加するか、`CubicBezierTo` に変換するか判断が必要
- `A/a` は `ArcTo` を追加するか、`CubicBezierTo` に分割するか判断が必要
- `A/a` は SVG の中でも計算が重く、初期実装に含めると実装範囲が広がりやすい

---

# Project SVG における暫定方針

`SvgPathInterpreter` の初期方針は以下とする。

| 項目 | 方針 |
|---|---|
| 入力 | `std::vector<parser::PathCommand>` |
| 出力 | `std::optional<std::vector<PathInstruction>>` |
| 座標 | 出力時点ですべて絶対座標へ正規化する |
| `H/h` | `LineTo` に正規化する |
| `V/v` | `LineTo` に正規化する |
| `M/m` 複数ペア | 2組目以降を `LineTo` として扱う |
| 不正値 | `std::nullopt` を返す |
| 未対応コマンド | `std::nullopt` を返す |
| 初期実装対象 | `M/m`, `L/l`, `H/h`, `V/v`, `C/c`, `Z/z` |
| 追加検討 | `S/s` |
| 別途判断 | `Q/q`, `T/t`, `A/a` |

`SvgPathInterpreter` は、path の構文解析ではなく、`PathCommand` を意味のある `PathInstruction` に変換する層とする。

そのため、`SvgPathParser` で抽出されたコマンド列に対して、以下の意味解釈を行う。

- 数値として妥当か
- 現在座標を基準に計算できるか
- 相対座標を絶対座標へ変換できるか
- 上位層がそのまま保持できる命令列にできるか

`SvgElementInterpreter` では、`SvgPathInterpreter` の結果を受け取り、そのまま格納できる粒度にする。
