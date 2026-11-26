# libMenu CUI メニューライブラリ（sample）

番号入力で階層メニューをたどる、対話式ツールの入口用ライブラリ。メニューの表示、入力の
受け付け、サブメニューへの移動と戻り、入力履歴の記録を `Navigator` が受け持つ。
文字列をトークンに分けて整数・小数・文字列に振り分ける `Tokenizer` も同梱する。

## 実行環境

- Linux + g++ + make（外部ライブラリには依存しない。C++17）
- `menu.cpp` には Windows（`_WIN32`）向けの分岐もあるが、Makefile は Linux 前提

## ディレクトリ構成

```
libMenu/
├── README.md
├── Makefile            # ビルド。実行ファイル menuTest と obj/ を libMenu/ 直下に出力する
├── main.cpp            # デモ。Search / Settings / Tools のメニューを組み立てて実行する
└── libMenu/
    ├── menu.hpp/.cpp       # Menu、Navigator、InputLog（namespace cui）
    ├── menuItem.hpp/.cpp   # MenuItem（namespace cui）
    └── tokenizer.hpp/.cpp  # Tokenizer（namespace util）。デモでは使っていない
```

## 使い方

```bash
cd libMenu
make            # ./menuTest を作る
./menuTest      # メニューを表示して入力を待つ（0 で戻る、ルートで 0 なら終了）
make clean      # obj/ と menuTest を削除
```

表示例:

```text
    === Main ===
    1) Search Menu
    2) Settings Menu
    3) Tools Menu
    4) About
    5) Help
    0) Exit
  番号を入力してください:
```

## 操作の仕様

- `1`〜`N` で項目を選ぶ。`0` はサブメニューでは Back、ルートでは Exit になる。
- 入力は1行ずつ読み、先頭の整数を選択番号とする。整数として読めない入力と範囲外の番号は
  不正として扱い、メッセージを出して同じメニューを再表示する。
- Action 項目は関数を実行した後、同じメニューに戻る。関数の戻り値（`int`）は使わない。
- Action 内で `std::exception` が送出された場合はメッセージを出してメニューに戻る。
  それ以外の例外は捕捉しない。
- 標準入力が EOF になったら終了する。
- 見出しにはパンくず（`Main > Search` など）を表示する。

## プログラムからの使い方

```cpp
#include "libMenu/menu.hpp"
using namespace cui;

int hello() { std::cout << "hello\n"; return 0; }

int main() {
    Menu root("Main");
    Menu sub("Sub");

    root.add(MenuItem::submenuItem("Sub Menu", sub));   // sub は参照で保持される
    root.add(MenuItem::actionItem("Hello", &hello));
    sub.add(MenuItem::actionItem("Hello", [] { std::cout << "in sub\n"; return 0; }));

    Navigator nav;          // 既定は std::cout / std::cin。テストでは任意のストリームを渡せる
    nav.run(root);
    nav.show_history();     // 入力履歴を表示
    return 0;
}
```

### MenuItem の作り方

`MenuItem` はファクトリ関数でだけ作れる。

| 関数 | 内容 |
|---|---|
| `actionItem(label, fn)` | 選ぶと `fn`（`std::function<int()>`）を実行する |
| `submenuItem(label, child)` | 選ぶと `child` のメニューへ移る |
| `backItem(label)` / `exitItem(label)` | メニューには表示されず、番号でも選べない。戻る・終了は常に `0` で行う |

`submenuItem` は子メニューをポインタで持つだけで、所有しない。子の `Menu` は、`Navigator::run`
が終わるまで生存させること。子メニューへの項目追加は、`submenuItem` を作った後でもよい。

### 入力履歴

`Navigator::logs()` で `InputLogEntry` の一覧を取得できる。`show_history()` は表形式で出力する。

| 項目 | 内容 |
|---|---|
| `timestamp` | 記録した時刻 |
| `menu_path` | 記録時点のパンくず。`0` で戻った場合は戻った後のパンくずになる |
| `raw_input` | 入力した行そのまま。Action の例外時は `action-exception` |
| `choice_index` | 選択番号。不正な入力は `Navigator::kInvalidInput`（`-999`） |
| `status` | `ok` / `invalid` / `error` |

## Tokenizer

空白で区切った文字列を、整数・小数・文字列に振り分ける静的クラス（インスタンス化不可）。

```cpp
#include "libMenu/tokenizer.hpp"
using util::Tokenizer;

auto tokens = Tokenizer::filterValidTokens(Tokenizer::splitWords("10 2.5 abc"));
auto pv = Tokenizer::parseTokens(tokens);   // pv.ints={10} pv.doubles={2.5} pv.strings={"abc"}
auto sv = Tokenizer::parseToStrVector(tokens, Tokenizer::TagStyle::Prefix);
// sv = {"INT:10", "FLOAT:2.5", "STR:abc"}
```

- `filterValidTokens` は、UTF-8 として不正なトークンと、タブ・改行以外の制御文字を含むトークンを除く。
- `long long` の範囲を超える整数は `double` として扱う。
- 判定に使う `static std::regex` はマルチスレッドでの利用に注意が必要（`tokenizer.hpp` のコメント参照）。

## 制約

- 入力は番号のみ。キー操作やカーソル移動には対応しない。
- 子メニューは所有されないため、生存期間は呼び出し側で管理する。
- スレッドセーフではない。
