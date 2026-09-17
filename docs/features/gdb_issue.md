# libGdb エビデンス採取テストフレームワーク 設計メモ

## 目的

GDB を使った C++ 単体検証で、**エビデンス（証跡）作成を楽にする**こと。
GDB のネイティブ出力（`(gdb) p 変数` → `$1 = ...` の型付き表示）に
できる限り準拠したまま、対象ごとの記述量を減らす共通ライブラリを整える。

対象は現状 `Cgt0927ChildResultKnr::readCalResult()`（子プロセス計算結果の
読み込み）の検証。将来 Cgt0928 など別対象にも使い回せる形にしたい。

## 現状（コミット済み）

配置: `SCRIPT/shell/my_tool/c/libGdb/`（親に `.svn`。SVN 作業コピー。Git 管理外）

```
libGdb/
├── main.cpp              … テストシナリオ + GDBダミー関数(gdb_dump_read_date / gdb_dump_out)
├── header.h              … childResultTest.hpp をインクルード
├── Makefile              … SOURCES = $(wildcard *.cpp)、DEVHOME=../../../.. 他は相対
├── childResultTest.hpp   … テストデータ生成・書き込みロジック（ルート平置き）
├── childResultTest.cpp
├── libGdb/               … 汎用GDBスクリプト群（サブディレクトリ）
│   ├── auto_read.gdb     … 自動実行（silent+continue で止まらず一括ダンプ）
│   ├── manual_debug.gdb  … 手動（各ブレイクで停止、c で進める）
│   └── gdb_debug.sh      … ランナー（現状 testChildPrcKnr 固定）
└── obj/

# ルート直下に旧世代の重複/不整合ファイルが残存（未整理）:
#   auto_read.gdb / manual_debug.gdb / gdb_debug.sh（libGdb/ と重複）
#   dump.gdb / tmp.gdb（現行 main.cpp と不整合な旧世代）
#   test.h / test_pid.h（未参照 / childResultTest.cpp に統合済み）
```

libTree（`trank/tool/libTree`）の構成に倣った。ただし libTree は複数部品
（libTree/libSch/libTkn/libDsp）があるためサブディレクトリ化しているのに対し、
本ツールはテスト対象が1つのため childResultTest.* はルート平置きとした。
「汎用扱いは libGdb/ サブディレクトリのみ」という切り分け。

## テストコードの構造（3層 + GDB2層）

エビデンス採取型テストは常に次の部品で構成される。

| 部品                          | 内容                                                             | 汎用/対象依存          |
| ----------------------------- | ---------------------------------------------------------------- | ---------------------- |
| ① main.cpp シナリオ           | ケースを実行順に並べる。見出し `===== TEST: xxx =====` を出す    | ほぼ定型               |
| ② 対象別ロジック（xxxTest.*） | データ生成・対象への書き込み。対象のAPI/構造体に依存             | 対象依存（必須で書く） |
| ③ GDB観測点（ダミー関数）     | GDBがブレイクを張る着地点。処理はしない（volatile で最適化回避） | 汎用化可能（後述）     |
| ④ dump_xxx.gdb                | ブレイク定義 + `p 式`。GDBネイティブ出力がそのままエビデンス     | 対象依存（p式が本質）  |
| ⑤ gdb_common.gdb              | 共通設定（set pagination off 等）+ ロギング                      | 汎用（固定資産）       |
| ⑥ gdb_run.sh                  | ランナー（引数で実行ファイル・gdbスクリプトを受ける）            | 汎用（固定資産）       |

## 汎用化の到達点

「GDB出力に準拠する」制約下で、汎用化できる/できないの切り分けは明確。

### 汎用化できる（libGdb/ の固定資産にできる）

- ⑤ **共通GDB設定** … `set pagination off` 等の定型。完全に対象非依存。
- ⑥ **ランナー** … 実行ファイルと gdbスクリプトを引数で受ける形にすれば完全汎用。
  ```bash
  # gdb_run.sh <実行ファイル> <gdbスクリプト> [追加引数...]
  TARGET_EXEC="${1:?}"; GDB_SCRIPT="${2:?}"; shift 2
  gdb -q -x "$GDB_SCRIPT" --args "$TARGET_EXEC" "$@"
  ```
- **エビデンスのファイル自動出力** … GDB の `set logging`（`set logging file evidence.txt`
  / `set logging on`）で GDB 画面出力をそのままファイル保存。タイムスタンプ命名を
  ランナーで付ければ実行ごとに証跡が残る。完全汎用。エビデンス作成の主目的に直結。
- ③ **GDB観測点** … 下記「観測点の汎用化」参照。汎用マーカー1個に集約できる。

### 汎用化できない（対象ごとに書くしかない = 書くのが本質）

- ② **データ生成ロジック** … 対象の構造体・書き込みAPIに依存。
- ④ **`p 式`** … `p out[$i].oya_process_id` 等。「エビデンスとして何を見せるか」
  そのものなので、汎用化するとエビデンスの意味が失われる。ここは残すのが正解。

## 観測点（③）の汎用化案

観測点は「ブレイクの着地点」でしかなく、引数の中身は本質的でない。GDBは
観測点で止まった後、**呼び出し元フレーム（対象の実変数）を見に行ける**。

### 推奨: マーカー関数1個 + up 方式

```cpp
// libGdb 側に置ける汎用マーカー（対象非依存）。インライン抑止が重要。
__attribute__((noinline)) void gdb_probe(const char* label) {
    volatile const char* l = label; (void)l;
}
```

テスト側は見せたい変数がスコープにある場所で呼ぶだけ:

```cpp
{
    std::vector<XXX_St> out;
    Cgt0928Foo::readXxx(key, out);
    gdb_probe("out_dump");   // ここで停止。out はこのスコープに存在
}
```

GDBスクリプトは呼び出し元へ `up` して `p`:

```gdb
break gdb_probe
commands
  silent
  up                 # 呼び出し元フレーム = out が見えるスコープ
  printf "(gdb) p out\n"
  p out              # GDBネイティブ出力 "$1 = ..." がそのまま
end
```

- 利点: ③が完全に汎用化（`gdb_probe` 1個を使い回す）。GDB出力に完全準拠。
- ラベルで複数観測箇所を区別可能（読込前 / 結果 など）。

### 注意点（up の脆さ）

- `up` は「呼び出し元が1つ上のフレーム」前提。インライン展開/最適化で段数が
  ずれる。→ 観測点を `__attribute__((noinline))` にして抑止する。
  （現 Makefile は `-finline-functions` 付きのため特に注意）
- 代替: ポインタ渡し方式 `gdb_probe(label, &out)` もあるが、GDB側で型キャストが
  必要になり出力がきれいでない。→ up 方式（noinline）を推奨。

### 非推奨: C++側で文字列整形して渡す方式

GDBの型付き出力が失われ、「GDBがこう出した」というエビデンスの説得力が
落ちるため、今回の目的（GDB出力準拠）には不採用。

## 新しい対象を検証するとき書くもの（量産手順）

| 部品                   | 書くか                                   | 雛形                |
| ---------------------- | ---------------------------------------- | ------------------- |
| ② xxxTest.hpp/.cpp     | 書く（対象のデータ生成・書込）           | childResultTest.*   |
| ④ dump_xxx.gdb の p 式 | 書く（見たいメンバ）                     | 現 manual_debug.gdb |
| ① main シナリオ        | 書く（gdb_probe を要所に置く、ほぼ定型） | 現 main.cpp         |
| ③ gdb_probe            | 書かない（流用）                         | libGdb/             |
| ⑤ gdb_common.gdb       | 書かない（流用）                         | libGdb/             |
| ⑥ gdb_run.sh           | 書かない（流用）                         | libGdb/             |

Cgt0927 の現物がリファレンス実装になる。次からはコピーして中身差し替えで量産。

## 未決事項（着手前に決める）

1. **進め方**
   - A: フルリファクタ … libGdb/ を 共通設定+ランナー+ログ に整理し、Cgt0927 側も
     `dump_cgt0927.gdb` が `gdb_common.gdb` を source する形に作り替える。
   - B: 段階移行 … Cgt0927 の既存スクリプトは残し、`gdb_common.gdb` と `gdb_run.sh`
     を新設するだけ。既存を壊さない。
2. **観測点方式** … up 方式（noinline マーカー、推奨）/ ポインタ渡し方式。
3. **エビデンスのファイル出力** … `set logging` を共通機能として入れるか。
   （エビデンス作成が主目的なので入れる効果大）
4. **gdb_probe の配置** … `libGdb/gdb_probe.hpp` + `.cpp` として追加するか、
   ヘッダのみにするか。
5. **ルート直下の重複/旧世代ファイルの整理** … auto_read.gdb 等の重複、
   dump.gdb/tmp.gdb（旧世代）、test.h/test_pid.h（未参照/統合済み）を削除するか。

## 設計判断の要点（記録）

- GDB出力準拠とエビデンス採取が最優先要件。この2点が「C++整形方式」を却下し、
  「マーカー + up + GDBネイティブ p」方式を選ぶ根拠。
- 汎用化の限界は「p 式（何を見せるか）」であり、これは本質的に対象依存。
  ここを無理に汎用化しない判断が、ツールの実用性を保つ。
- libGdb/ = 「エビデンス採取の型」を固定する層。対象ごとには「データ生成・
  観測点配置・p式」の3種だけを埋める運用。
