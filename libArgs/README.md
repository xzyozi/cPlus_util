# libArgs コマンドライン引数解析ライブラリ

C++17 標準ライブラリだけで書いた、小さなコマンドライン引数解析ライブラリ。
サブコマンドの振り分け、長短オプション、フラグ、位置引数、必須・既定値、
取得時の型変換、`--help` の自動生成を扱う。例外は投げず、解析結果に
エラー種別とメッセージを持たせるため、終了コードは呼び出し側が決める。

## 実行環境

- Linux + g++（C++17）+ GNU make + bash
- 外部ライブラリ不要

## ディレクトリ構成

```
libArgs/
├── README.md
├── .gitignore         # generated/ を除外
├── Makefile           # ビルド。出力は generated/
├── main.cpp           # デモ兼テストドライバ（IN_ARGS= / MSG= / RET= / VALUE= を出力）
├── lib/
│   ├── args.hpp       # 公開 API
│   └── args.cpp       # 実装
├── cases/
│   └── sample.tsv     # ケース表（A01〜A14 正常系、E01〜E14 異常系）
├── run_tests.sh       # ケース表を1ケースずつ別プロセスで実行し 良/否 を判定
└── generated/         # 実行ファイル・obj・証跡・結果。管理外（make clean で削除）
```

## 使い方

```bash
make                                   # generated/libArgsSample を作る（obj は generated/obj/）
bash run_tests.sh                      # cases/*.tsv を全件実行
bash run_tests.sh --filter 'E0[1-3]'   # ID が glob に一致するケースのみ
bash run_tests.sh cases/sample.tsv     # 指定ファイルのみ
make clean                             # generated/ を削除（証跡も消える）
```

ドライバを直接動かす例:

```bash
./generated/libArgsSample run --id C01 --count 3 -v
# IN_ARGS=run --id C01 --count 3 -v
# RET=NML
# VALUE=cmd=run id=C01 count=3 ratio=- verbose=1 pos=-

./generated/libArgsSample run --help
```

- `run_tests.sh` はスクリプト自身のディレクトリへ移動して動くので、リポジトリ
  ルートから `bash libArgs/run_tests.sh` としてもよい（CI はこの形で呼ぶ）。
- 判定は `良` / `否` の2種類。`RET=`、`VALUE=`、終了コード（NML=0、ERR=2）を照合する。
- 終了コード: 0=全件 良 / 1=否あり（実行ケース0件も 1）/ 2=準備不備（実行ファイルなし、ケース表なし）。
- `generated/result_<日時>.tsv` に判定の一覧を保存する。
- 証跡は `generated/evidence/<日時>/<id>.log`。ドライバの出力全体と `EXIT=<終了コード>` を記録する。

## API

名前空間は `args`。宣言は `lib/args.hpp`。

| 型・関数                                                | 内容                                                                   |
| ------------------------------------------------------- | ---------------------------------------------------------------------- |
| `Parser(program, description)`                          | 解析器。`program` は使い方に出すプログラム名                           |
| `Parser::addCommand(name, description)`                 | サブコマンドを追加し `Command&` を返す（参照は Parser の寿命の間有効） |
| `Parser::root()`                                        | サブコマンドなしの単一コマンドとして使うときの定義先                   |
| `Parser::parse(argc, argv)`                             | `argv[0]` を除いて解析し `Result` を返す                               |
| `Parser::parse(tokens)`                                 | `std::vector<std::string>`（プログラム名なし）を解析する               |
| `Parser::help()` / `help(cmd)`                          | 使い方の文字列を返す                                                   |
| `Command::required(long, short, help)`                  | 必須の値付きオプション                                                 |
| `Command::value(long, short, help)`                     | 任意の値付きオプション                                                 |
| `Command::withDefault(long, short, default, help)`      | 既定値付きの値付きオプション                                           |
| `Command::flag(long, short, help)`                      | フラグ（値なし）                                                       |
| `Result::ok()`                                          | エラーなしなら true（`--help` 時も true）                              |
| `Result::helpRequested()` / `helpText()`                | `--help` 指定の有無と、生成した使い方                                  |
| `Result::errorKind()` / `error()`                       | エラー種別と日本語メッセージ                                           |
| `Result::command()`                                     | 選ばれたサブコマンド名（単一コマンド・全体 help は空）                 |
| `Result::has(long)`                                     | 値がある（指定済みまたは既定値あり）か                                 |
| `Result::flag(long)`                                    | フラグが指定されたか                                                   |
| `Result::getString/getInt/getDouble/getBool(long, out)` | 値を型変換して取得。値なし・変換失敗は false（`out` は変更しない）     |
| `Result::failInvalidValue(long, raw)`                   | 型変換失敗を `invalid_value` として記録（既存のエラーは上書きしない）  |
| `Result::positionals()`                                 | 位置引数の一覧                                                         |
| `errorName(kind)`                                       | エラー種別の名前（`unknown_option` など）                              |
| `toInt/toDouble/toBool(text, out)`                      | 型変換関数（単体でも使える）                                           |

- オプションは長い名前（`--` を除く）で識別する。短い名前は `'\0'` でなし。
- `Command` の各追加関数は `Command&` を返すので連結して書ける。
- 定義の誤りは `assert` で停止する（`NDEBUG` 時は検出しない）。対象は、long 名
  `help` と short 名 `'h'`（`--help` / `-h` の予約名）、同じコマンド内での long 名・
  short 名の重複。

### 使用例（サブコマンドあり）

```cpp
#include <iostream>
#include "args.hpp"

int main(int argc, char **argv) {
  args::Parser parser("prog", "ジョブ管理ツール");
  parser.addCommand("run", "ジョブを実行する")
      .required("id", 'i', "ジョブID")
      .withDefault("count", 'c', "1", "実行回数")
      .flag("verbose", 'v', "詳細を表示する");
  parser.addCommand("list", "ジョブ一覧を表示する").flag("all", 'a', "すべて表示する");

  const args::Result r = parser.parse(argc, argv);
  if (!r.ok()) {
    std::cerr << r.error() << "\n";
    return 2;
  }
  if (r.helpRequested()) {
    std::cout << r.helpText();
    return 0;
  }
  if ("run" == r.command()) {
    std::string id;
    r.getString("id", id);
    int count = 0;
    if (!r.getInt("count", count)) {
      std::cerr << "--count は整数で指定してください\n";
      return 2;
    }
    std::cout << id << " x" << count << (r.flag("verbose") ? " (verbose)" : "") << "\n";
  }
  return 0;
}
```

### 使用例（単一コマンド）

```cpp
args::Parser parser("tool", "ファイルを処理する");
parser.root().value("output", 'o', "出力先").flag("dry-run", 'n', "書き込まない");
const args::Result r = parser.parse(argc, argv);   // 例: tool -o out.txt a.txt b.txt
// r.positionals() == {"a.txt", "b.txt"}
```

### 使用例（libConf と併用し、設定値を引数で上書きする）

libConf（`libConf/libConf/conf.hpp`）の `ConfigManager::get()` は
`const ConfData&` を返すので、コピーを作ってから引数の値で上書きする。
`getInt` は値がないとき・変換に失敗したとき `out` を変更しないため、
引数を指定しなければ設定ファイルの値がそのまま残る。

```cpp
#include <iostream>
#include "args.hpp"
#include "conf.hpp" // libConf。ビルド時は libConf/libConf/conf.cpp もリンクする

int main(int argc, char **argv) {
  args::Parser parser("tool", "設定ファイルの値を引数で上書きする");
  parser.root().value("retry_limit", 'r', "再試行回数の上限（設定値を上書き）");

  args::Result result = parser.parse(argc, argv);
  if (!result.ok()) {
    std::cerr << result.error() << "\n";
    return 2;
  }
  if (result.helpRequested()) {
    std::cout << result.helpText();
    return 0;
  }

  ConfigManager conf;
  ConfData data = conf.get(); // コピー（get() は const ConfData& を返す）
  if (result.has("retry_limit") &&
      !result.getInt("retry_limit", data.retry_limit)) {
    std::string raw;
    result.getString("retry_limit", raw);
    result.failInvalidValue("retry_limit", raw);
    std::cerr << result.error() << "\n"; // errorName(result.errorKind()) は invalid_value
    return 2;
  }
  std::cout << "retry_limit=" << data.retry_limit << "\n";
  return 0;
}
```

## 仕様

### 解析の流れ

1. 走査を始める前に、`--` より前のトークンに `--help` / `-h` があるかを調べる。
   あれば、他のエラー（未定義オプション、値なし、必須なしなど）より優先して help を返す。
   - サブコマンドあり: 先頭トークンが登録済みサブコマンドなら、そのサブコマンドの使い方
     （`command()` はその名前。例: `run --bogus --help` → run の使い方）。
     先頭が `--help` / `-h` や `-` 始まり、未登録の名前（例: `stop --help`）なら全体の使い方
     （`command()` は空）。
   - サブコマンドなし: `root()` の使い方（`command()` は空）。
   - `--` より後の `--help` / `-h` は位置引数。
2. サブコマンドが1つ以上登録されていれば、先頭トークンで振り分ける。
   - トークンなし、または先頭が `-` で始まる → `missing_command`
   - 登録されていない名前 → `unknown_command`
3. サブコマンドがなければ、`root()` の定義で全トークンを解析する。
4. 既定値を入れてから、オプションを左から順に読む。同じオプションを2回指定したら後勝ち
   （例: `--count 2 --count 3` は `3`）。
5. 最初のエラーで解析を止める（`Result` は最初のエラーだけを持つ）。
6. 最後まで読めたら必須チェックを行う（定義順で最初の欠落を報告）。

### オプションの書式

| 書式                             | 扱い                                                                                 |
| -------------------------------- | ------------------------------------------------------------------------------------ |
| `--name value`                   | 値付きオプション                                                                     |
| `--name=value`                   | 値付きオプション（値に `=` や `-` 始まりを含めてよい。`--name=` は `missing_value`） |
| `-n value`                       | 短い名前の値付きオプション                                                           |
| `--flag` / `-f`                  | フラグ。値は `"true"` として保持                                                     |
| `--flag=値`                      | `unexpected_value`                                                                   |
| `--help` / `-h`                  | 使い方を生成して返す（`--` より前のどこにあってもよい。他のエラーより優先）          |
| `--`                             | 以降はすべて位置引数（`-` 始まりも含む）                                             |
| `-` 単独、`-` で始まらない語     | 位置引数                                                                             |
| `-abc`（まとめ書き）、`-n=value` | 非対応。`unknown_option`                                                             |

- 値付きオプションの値は次のトークン。次がない場合、次が空文字の場合、または次が `-` で始まる
  2文字以上のトークン（`-5` や `-.5` のように2文字目が数字か `.` のものを除く）
  の場合は `missing_value`。`-` 始まりの文字列を値にしたいときは `--name=value` を使う。
  `--help` / `-h` は値として受け付けない（help として扱う）。
- 空の値は、値付きオプションすべてで `missing_value`（`--name=` と `--name ""` の両方）。
- オプションの値としての `-5` は受け付ける（例: `--count -2`）。位置引数の `-5` は短い
  オプション扱いで `unknown_option` になる。負数を位置引数にしたいときは `--` の後に置く。

### 型変換規則（取得時）

| 関数        | 成功する値                                       | 失敗例                                          |
| ----------- | ------------------------------------------------ | ----------------------------------------------- |
| `getString` | 何でも                                           | （値がないときだけ失敗）                        |
| `getInt`    | `strtol`（10進）で末尾まで変換でき、`int` 範囲内 | `abc`、`3x`、`1.5`、空文字、先頭空白、範囲外    |
| `getDouble` | `strtod` で末尾まで変換でき、有限値              | `1.2.3`、`inf`、`nan`、空文字、先頭空白、範囲外 |
| `getBool`   | `true` `1` `yes` `on` / `false` `0` `no` `off`   | `TRUE`、`2`                                     |

`getBool` は大文字小文字を区別する（`TRUE`、`Yes` は失敗）。

変換失敗は解析時にはエラーにしない。取得関数が false を返したら、呼び出し側が
`Result::failInvalidValue(long, raw)` で記録する。`errorKind()` が `InvalidValue`、
`error()` が `オプションの値を変換できません: --<long>=<raw>` になる。既にエラーが
あれば上書きしない（最初の1件を保持する）。ドライバは `--count` / `--ratio` /
`--dry-run` の変換失敗をこの関数で記録し、`error=invalid_value` で返す。

```cpp
args::Result r = parser.parse(argc, argv); // failInvalidValue を呼ぶので const にしない
int count = 0;
if (!r.getInt("count", count)) {
  std::string raw;
  r.getString("count", raw);
  r.failInvalidValue("count", raw);
}
```

### エラー種別

| `ErrorKind`       | `errorName()`      | 発生条件                                |
| ----------------- | ------------------ | --------------------------------------- |
| `UnknownOption`   | `unknown_option`   | 未定義のオプション、まとめ書き          |
| `MissingValue`    | `missing_value`    | 値付きオプションの値がない              |
| `UnexpectedValue` | `unexpected_value` | フラグに `=値` を付けた                 |
| `MissingRequired` | `missing_required` | 必須オプションがない                    |
| `InvalidValue`    | `invalid_value`    | 型変換失敗（`failInvalidValue` で記録） |
| `UnknownCommand`  | `unknown_command`  | 未知のサブコマンド                      |
| `MissingCommand`  | `missing_command`  | サブコマンドがない                      |

### help の形式

全体（サブコマンドあり）:

```text
使い方: libArgsSample <サブコマンド> [オプション]
libArgs のデモ兼テストドライバ

サブコマンド:
  run   ジョブを実行する
  list  ジョブ一覧を表示する
  conf  設定値（bool）を表示する

各サブコマンドの詳細: libArgsSample <サブコマンド> --help
```

サブコマンド（または単一コマンド）:

```text
使い方: libArgsSample run [オプション] [--] [位置引数...]
ジョブを実行する

オプション:
  -i, --id <VALUE>     ジョブID（必須）
  -c, --count <VALUE>  実行回数（整数）（既定: 1）
  -r, --ratio <VALUE>  比率（小数）
  -v, --verbose        詳細を表示する
  -h, --help           この使い方を表示する
```

- 左列はオプション定義から作り、最長の左列に揃える（左列は ASCII のみ）。
- 説明の後ろに、必須なら `（必須）`、既定値があれば `（既定: 値）` を付ける。

## ドライバ（main.cpp）の出力

| 行         | 内容                                                                           |
| ---------- | ------------------------------------------------------------------------------ |
| `IN_ARGS=` | 受け取った引数を空白で連結                                                     |
| `MSG=`     | エラーメッセージ（エラー時のみ）                                               |
| `RET=`     | `NML` / `ERR`                                                                  |
| `VALUE=`   | run: `cmd=run id=<ID> count=<N> ratio=<R or -> verbose=<0/1> pos=<a,b or ->`   |
|            | list: `cmd=list all=<0/1> pos=<a,b or ->`                                      |
|            | conf: `cmd=conf dry_run=<0/1>`（`--dry-run` は既定 `false`、`getBool` で取得） |
|            | help: `help=<run / list / conf / global>`（使い方本文は `RET=` の前に出す）    |
|            | エラー: `error=<errorName()>`                                                  |

終了コード: 0=成功（`--help` を含む）、2=引数エラー。

## ケース表の書き方（`cases/sample.tsv`）

タブ区切り・UTF-8・LF（CR は読み込み時に除去）。`#` で始まる行はコメント。4列。
空の列は使えない（bash の `read` がタブの連続を1つにまとめるため）。

| 列          | 内容                                                                       |
| ----------- | -------------------------------------------------------------------------- |
| `id`        | ケース ID（`A01` 正常系、`E01` 異常系）                                    |
| `args`      | ドライバへ渡す引数。空白で単語分割する（クォート・エスケープは解釈しない） |
| `exp_ret`   | `NML` / `ERR`。終了コードも NML=0、ERR=2 で照合する                        |
| `exp_value` | `VALUE=` の期待値。`-` は照合しない                                        |

ケースを追加したら、期待値は `main.cpp` の出力形式（上表）から導く。`ratio` は
`std::ostream` の既定書式（例: `0.25`、`1.5`）で出る。

ケース表で書けないもの:

- 引数なしのケース（`args` 列を空にできないため。`-` も照合しない印ではなく引数 `-` として渡る）。
- 空文字のトークン（`--id ""`）。`args` は `read -r -a` で空白分割し、クォートを
  解釈しないため、空トークンを渡せない。`--id ""` が `missing_value` になることは
  実装（空文字を値とみなさない）で担保し、ケース表では `--id=`（E13）で代替する。

### ケース一覧

| id  | args                                  | 期待 RET | 期待 VALUE                                                  |
| --- | ------------------------------------- | -------- | ----------------------------------------------------------- |
| A01 | `run --id C01 --count 3 -v`           | NML      | `cmd=run id=C01 count=3 ratio=- verbose=1 pos=-`            |
| A02 | `run --id=C02 --count=5 --ratio=0.25` | NML      | `cmd=run id=C02 count=5 ratio=0.25 verbose=0 pos=-`         |
| A03 | `run -i C03 -c 2 -r 1.5 --verbose`    | NML      | `cmd=run id=C03 count=2 ratio=1.5 verbose=1 pos=-`          |
| A04 | `run --id C04`                        | NML      | `cmd=run id=C04 count=1 ratio=- verbose=0 pos=-`            |
| A05 | `run --id C05 --count -2`             | NML      | `cmd=run id=C05 count=-2 ratio=- verbose=0 pos=-`           |
| A06 | `list --all`                          | NML      | `cmd=list all=1 pos=-`                                      |
| A07 | `list`                                | NML      | `cmd=list all=0 pos=-`                                      |
| A08 | `run --id C08 -- --count x -v`        | NML      | `cmd=run id=C08 count=1 ratio=- verbose=0 pos=--count,x,-v` |
| A09 | `run --help`                          | NML      | `help=run`                                                  |
| A10 | `--help`                              | NML      | `help=global`                                               |
| A11 | `run --id --help`                     | NML      | `help=run`（`--help` は値にならず help が優先）             |
| A12 | `run --bogus --help`                  | NML      | `help=run`（未定義オプションより help が優先）              |
| A13 | `conf --dry-run yes`                  | NML      | `cmd=conf dry_run=1`                                        |
| A14 | `conf --dry-run off`                  | NML      | `cmd=conf dry_run=0`                                        |
| E01 | `run --count 3`                       | ERR      | `error=missing_required`                                    |
| E02 | `run --id C01 --count abc`            | ERR      | `error=invalid_value`                                       |
| E03 | `run --id C01 --count 3x`             | ERR      | `error=invalid_value`                                       |
| E04 | `run --id C01 --unknown`              | ERR      | `error=unknown_option`                                      |
| E05 | `run --id`                            | ERR      | `error=missing_value`                                       |
| E06 | `stop --id C01`                       | ERR      | `error=unknown_command`                                     |
| E07 | `list --all=1`                        | ERR      | `error=unexpected_value`                                    |
| E08 | `-v`                                  | ERR      | `error=missing_command`                                     |
| E09 | `conf --dry-run TRUE`                 | ERR      | `error=invalid_value`（`toBool` は大文字小文字を区別する）  |
| E10 | `conf --dry-run maybe`                | ERR      | `error=invalid_value`                                       |
| E11 | `run --id C01 --ratio abc`            | ERR      | `error=invalid_value`                                       |
| E12 | `run --id C01 --count 99999999999`    | ERR      | `error=invalid_value`（`int` 範囲外）                       |
| E13 | `run --id=`                           | ERR      | `error=missing_value`（空の値）                             |
| E14 | `run -i`                              | ERR      | `error=missing_value`                                       |

## 制約

- 短いオプションのまとめ書き（`-abc`）、`-nVALUE`、`-n=VALUE` は非対応。
- 長いオプションの省略形（`--verb` → `--verbose`）は非対応。
- サブコマンドの入れ子、サブコマンド前の共通オプションは非対応。
- 同じオプションの複数値（配列）は保持しない（後勝ち）。
- help の左列幅はバイト数で揃えるため、オプション名は ASCII にする。
- 解析・取得とも例外を投げない（`std::bad_alloc` などメモリ不足は除く）。
