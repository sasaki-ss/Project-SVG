# SVG Path コマンド一覧（Project SVG向け）

## ■ 絶対座標コマンド

| コマンド | 動作概要 | 入力パラメータ | 状態更新 | 実装Tier | 備考 |
|----------|----------|----------------|----------|----------|------|
| `M` | 現在位置を移動し、サブパスを開始する | `x y` | `current_point`, `subpath_start` を更新 | Tier1 | 複数座標指定時、2組目以降は `LineTo` 扱い |
| `L` | 指定座標まで直線を引く | `x y` | `current_point` を更新 | Tier1 | 基本命令 |
| `H` | 水平方向に直線を引く | `x` | `current_point.x` を更新 | Tier1 | `LineTo(x, current.y)` に正規化可能 |
| `V` | 垂直方向に直線を引く | `y` | `current_point.y` を更新 | Tier1 | `LineTo(current.x, y)` に正規化可能 |
| `C` | 3次ベジェ曲線を描く | `x1 y1 x2 y2 x y` | `current_point` を更新 | Tier1 | 制御点2つ + 終点 |
| `Z` | 現在のサブパスを閉じる | なし | `current_point` を `subpath_start` に戻す | Tier1 | `LineTo(subpath_start)` 相当 |
| `S` | スムーズ3次ベジェ曲線を描く | `x2 y2 x y` | `current_point` を更新 | Tier2 | 前回の制御点を反射して使う |
| `Q` | 2次ベジェ曲線を描く | `x1 y1 x y` | `current_point` を更新 | Tier2 | 将来的に Cubic へ変換可能 |
| `T` | スムーズ2次ベジェ曲線を描く | `x y` | `current_point` を更新 | Tier2 | 前回の制御点依存 |
| `A` | 楕円弧を描く | `rx ry x-axis-rotation large-arc-flag sweep-flag x y` | `current_point` を更新 | Tier1(高) | 実装難易度高 |

---

## ■ 相対座標コマンド

| コマンド | 動作概要 | 入力パラメータ | 状態更新 | 実装Tier | 備考 |
|----------|----------|----------------|----------|----------|------|
| `m` | 現在位置からの相対移動でサブパスを開始する | `dx dy` | `current_point`, `subpath_start` を更新 | Tier1 | 複数座標指定時、2組目以降は `LineTo` 扱い |
| `l` | 現在位置からの相対位置へ直線を引く | `dx dy` | `current_point` を更新 | Tier1 | 基本命令 |
| `h` | 現在位置から水平方向へ相対直線を引く | `dx` | `current_point.x` を更新 | Tier1 | `LineTo(current.x + dx, current.y)` に正規化可能 |
| `v` | 現在位置から垂直方向へ相対直線を引く | `dy` | `current_point.y` を更新 | Tier1 | `LineTo(current.x, current.y + dy)` に正規化可能 |
| `c` | 現在位置基準の相対3次ベジェ曲線を描く | `dx1 dy1 dx2 dy2 dx dy` | `current_point` を更新 | Tier1 | 制御点2つ + 終点すべて相対値 |
| `z` | 現在のサブパスを閉じる | なし | `current_point` を `subpath_start` に戻す | Tier1 | `Z` と同義 |
| `s` | 相対スムーズ3次ベジェ曲線を描く | `dx2 dy2 dx dy` | `current_point` を更新 | Tier2 | 前回の制御点を反射して使う |
| `q` | 相対2次ベジェ曲線を描く | `dx1 dy1 dx dy` | `current_point` を更新 | Tier2 | 将来的に Cubic へ変換可能 |
| `t` | 相対スムーズ2次ベジェ曲線を描く | `dx dy` | `current_point` を更新 | Tier2 | 前回の制御点依存 |
| `a` | 相対楕円弧を描く | `rx ry x-axis-rotation large-arc-flag sweep-flag dx dy` | `current_point` を更新 | Tier1(高) | 終点のみ相対位置 |

---

## ■ 実装時に必要な状態

| 要素名 | 型 | 用途 |
|--------|----|------|
| `current_point` | `Point` | 現在位置。相対座標解釈の基準 |
| `subpath_start` | `Point` | `Z` / `z` で戻る開始点 |
| `last_control_point` | `Point` or `std::optional<Point>` | `S` / `s` / `T` / `t` 用の将来拡張 |

---

## ■ 正規化ルール

| 入力 | 正規化後 |
|------|----------|
| `H` | `LineTo(x, current.y)` |
| `h` | `LineTo(current.x + dx, current.y)` |
| `V` | `LineTo(current.x, y)` |
| `v` | `LineTo(current.x, current.y + dy)` |
| 小文字コマンド | 絶対座標へ変換して保持 |
| `M` / `m` の2組目以降 | `LineTo` 系として扱う |

---

## ■ 実装Tier定義

| Tier | 意味 |
|------|------|
| Tier1 | MVP必須 |
| Tier1(高) | MVP候補だが難易度高 |
| Tier2 | 後回しでよい拡張範囲 |

---

## ■ 実装優先順（推奨）

1. `M` / `m`
2. `L` / `l`
3. `H` / `h`
4. `V` / `v`
5. `Z` / `z`
6. `C` / `c`
7. `A` / `a`
8. `S` / `s`, `Q` / `q`, `T` / `t`

---

## ■ 補足

- `Z` と `z` は意味上ほぼ同一
- 実装時は「文字の大文字小文字で相対/絶対を判定」し、内部では絶対座標へ正規化するのが扱いやすい
- renderer には相対/絶対判定を持ち込まず、解釈済みの幾何命令列だけを渡す方が責務分離しやすい