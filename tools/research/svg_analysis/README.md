# SVG Analysis (Lucide)

`icons` 配下の `.svg` をXMLとしてパースし、各SVGごとの要素タグ一覧と属性一覧をJSON出力する調査用ツールです。

## 使い方

```bash
python3 tools/research/svg_analysis/analyze_svg.py --icons-dir icons
```

JSONファイルに保存する場合:

```bash
python3 tools/research/svg_analysis/analyze_svg.py \
  --icons-dir icons \
  --output tools/research/svg_analysis/output/analysis.json
```

## 出力フォーマット（抜粋）

```json
{
  "icons_dir": "icons",
  "svg_count": 2,
  "results": [
    {
      "file": "activity.svg",
      "elements": ["svg", "path"],
      "attribute_list": [
        {"tag": "svg", "attributes": {"width": "24"}},
        {"tag": "path", "attributes": {"d": "..."}}
      ],
      "error": null
    }
  ]
}
```

## 例外対応

- SVG単位でパース例外を捕捉します。
- 失敗したSVGは `error` に内容を格納し、他ファイルの処理は継続します。
- `icons` ディレクトリが見つからない場合も異常終了せず、警告付きJSONを出力します。
