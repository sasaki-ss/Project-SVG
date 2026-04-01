# コーディング規約（初期版）

## 1. 目的
本規約は、Project SVG における C++ コードの命名・記法・include 順序などの最低限の統一ルールを定義する。  
まずはレビュー観点のぶれを減らし、実装スタイルを揃えることを目的とする。

---

## 2. 命名規則

### 2.1 変数名
- **スネークケース** を使用する

例:
- `file_path`
- `input_stream`
- `error_code`

### 2.2 関数名
- **スネークケース** を使用する

例:
- `load_from_file`
- `parse_path`

### 2.3 型名
- **アッパーキャメルケース** を使用する

例:
- `SvgLoader`
- `XmlParser`
- `FileBuffer`

### 2.4 enum class 名
- **アッパーキャメルケース** を使用する

例:
- `LoadResult`

### 2.5 enum class 列挙子
- **アッパーキャメルケース** を使用する

例:
- `FileNotFound`
- `ReadFailed`

### 2.6 定数名
- **大文字スネークケース** を使用する

例:
- `DEFAULT_WIDTH`
- `MAX_POINT_COUNT`

### 2.7 メンバ変数
- **スネークケース** を使用する
- 接頭辞・接尾辞は付けない

例:
- `file_path`
- `buffer_size`

### 2.8 型エイリアス
- 原則として **`using`** を使用する
- `typedef` は禁止しないが、優先は `using` とする

---

## 3. namespace 規約

### 3.1 基本方針
- namespace は **プロジェクト名 → ディレクトリ名** の順で管理する
- ディレクトリ階層が増える場合は、その分ネストを増やす

例:

```cpp
namespace svg{
namespace loader{

class SvgLoader {
};

}
}
```

### 3.2 namespace 名
- **小文字**
- 基本は **1単語**

例:
- `svg`

### 3.3 インデント
- **namespace 内はネストしても1段下げしない**

例:

```cpp
namespace svg{
namespace loader{

class SvgLoader {
public:
    static std::optional<std::string> load_from_file(const std::filesystem::path& file_path);
};

}
}
```

---

## 4. ファイル・ヘッダ規約

### 4.1 ファイル名
- **アッパーキャメルケース** を使用する

例:
- `SvgLoader.h`
- `SvgLoader.cpp`

### 4.2 インクルードガード
- **`#ifndef` 方式** を使用する
- **`#pragma once` は使用しない**

命名規則:
- `PROJECT_SVG_<DIR>_<FILE>_H_`

例:

```cpp
#ifndef PROJECT_SVG_LOADER_SVG_LOADER_H_
#define PROJECT_SVG_LOADER_SVG_LOADER_H_

// ...

#endif // PROJECT_SVG_LOADER_SVG_LOADER_H_
```

---

## 5. 波括弧・空白・空行

### 5.1 開き波括弧の位置
- **クラス / 関数 / 制御文ともに開き波括弧は同一行**
- **開き波括弧の前に半角スペースを1つ入れる**

例:

```cpp
class SvgLoader {
public:
    static std::optional<std::string> load_from_file(const std::filesystem::path& file_path);
};

if (is_valid) {
    return true;
}
```

### 5.2 空行
- ブロック間には必要に応じて空行を1つ入れる
- 詰め込みすぎないこと

---

## 6. ポインタ・参照・null

### 6.1 ポインタ
- `Type* value`

### 6.2 参照
- `Type& value`

### 6.3 null
- **`nullptr`** を使用する
- **`NULL`** は使用しない

---

## 7. cast

### 7.1 C-style cast
- **可能な限り使用しない**
- 必要な場合は C++ cast を優先する

使用候補:
- `static_cast`
- `const_cast`
- `reinterpret_cast`
- `dynamic_cast`

---

## 8. auto

### 8.1 基本方針
- **型が明確な場合のみ使用可**
- 使用範囲は可能な限り限定する

### 8.2 主な許容例
- `for` 文
- `find` の戻り値
- 右辺から型が明白な場合

例:

```cpp
for (auto it = values.begin(); it != values.end(); ++it) {
    // ...
}

auto it = map.find(key);
```

---

## 9. include 順

以下の順番で記述する。

1. 同ディレクトリの自ヘッダ
2. 標準ライブラリ
3. 外部ライブラリ
4. プロジェクト内他ヘッダ

例:

```cpp
#include "SvgLoader.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>

#include <third_party/lib.hpp>

#include "parser/XmlParser.h"
```

---

## 10. 補足
- 本規約は **初期版** とする
- まずは命名、見た目、include 順の統一を優先する
- 例外設計や詳細なクラス設計規約は、必要に応じて別途定義する