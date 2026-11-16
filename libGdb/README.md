# libGdb エビデンス採取テストフレームワーク（sample）

GDB を使った C++ 単体検証で、エビデンス（証跡）を楽に採取するための作業用
ツール。テスト対象を実行し、GDB のネイティブ出力（`p 変数` の型付き表示）を
そのまま証跡として残すことを狙う。現状は **sample（お手本）** として扱う。

構成は `sclComBatTest` / `sclWebApiTest` と同じフラット形式に揃えている。
`libGdb` は、これらのテストを作るときの雛形であり、`gdb_probe.hpp` の正本を持つ。

## 実行環境

- Linux + g++ + gdb（**PostgreSQL は不要**、業務ライブラリにも依存しない）

## ディレクトリ構成

```
libGdb/
├── README.md
├── main.cpp              # ドライバ（ケース名を受け取り1ケース実行。IN_* / RET= / VALUE= を出力）
├── childResultTest.hpp   # 試験対象（sample: インメモリのデータストアとモック）
├── childResultTest.cpp
├── Makefile              # ビルド。出力は generated/
├── run_tests.sh          # ケース表を1ケースずつ別プロセスで実行し 良/否 を判定
├── cases/
│   └── childResult.tsv   # ケース表（C01〜C08）
├── lib/
│   └── gdb_probe.hpp     # 共有資産の正本（version: 1）。各テストへコピーして使う
├── docs/
│   └── TEST_CREATION_GUIDE.md
├── generated/            # 実行ファイル・証跡・結果。管理外（make clean で削除）
└── _old_structure/       # 旧構成（lib/ と src/ の分割）の退避。不要なら削除してよい
```

- **`gdb_probe.hpp`（正本）**: 持ってきて基本変更しないものとして扱う。変更が必要な
  ときは、`lib/` の正本を直して `version` を上げ、各テストへコピーし直す。
  各テストは `libGdb` に依存せず、コピーしたファイルだけで独立して扱う。
- **ドライバ（`main.cpp`）**: 標準出力に `IN_*=`（入力）、`RET=`、`VALUE=` を出す。
  `run_tests.sh` はこの出力でケースの合否を判定し、証跡にも使う。
- **`generated/`**: `make` では消さない。`make clean` で削除する。証跡は
  `generated/evidence/<実行日時>/` に実行ごとに分かれて残る。

## 使い方

```bash
make                                    # generated/libGdbSample を作る
./run_tests.sh                          # cases/*.tsv を全件実行
./run_tests.sh --filter 'C0[1-3]'       # ID が glob に一致するケースのみ
./run_tests.sh --gdb                    # gdb 経由で実行し、gdb_probe の入出力を証跡に保存
make clean                              # generated/ を削除（証跡も消える）
```

- 判定は `良` / `否` の2種類。
- 終了コード: 0=全件 良 / 1=否あり / 2=準備不備（実行ファイルなし、`--gdb` で gdb なし 等）。
- `generated/result_<日時>.tsv` に判定の一覧を保存する。
- 証跡は `generated/evidence/<実行日時>/<ID>.log`。先頭付近に入力（`IN_*=`）、続いて
  `RET=`・`VALUE=` を記録する。`--gdb` 時は観測点ごとに GDB のネイティブ表示が入る。

### `--gdb` の出力例

```text
[GDB] === probe: read_key ===
(gdb) p value
$1 = {oya_process_id = 12345, cal_result_flg = 1, cal_ym = "202407", ...}

[GDB] === probe: out_dump ===
(gdb) p value
$2 = std::vector of length 1, capacity 1 = {{...}}
```

出力は `p value` の素の表示で、加工しない。

## ケース表の書き方（`cases/childResult.tsv`）

タブ区切り・UTF-8・LF。`#` で始まる行はコメント。4列。

| 列          | 内容                                                              |
| ----------- | ----------------------------------------------------------------- |
| `id`        | ケース ID（`C01` など）                                           |
| `case`      | `main.cpp` のケース名（`single` `multi` `none` `unmatch_pid` 他） |
| `exp_ret`   | `NML` / `ERR`                                                     |
| `exp_value` | ドライバの `VALUE=` の期待値（`count=<件数>`）。`-` は照合しない  |

## 観測点の仕組み（汎用マーカー `gdb_probe`）

観測点は libGdb 共通の**汎用テンプレートマーカー** `gdb_probe(label, value)`
に統一している（`lib/gdb_probe.hpp`）。見せたい変数を値として渡すだけで、
型は自動推論される。対象が変わってもマーカーは書き直さない。

```cpp
std::vector<Foo> out;
readSomething(key, out);
gdb_probe("out_dump", out);   // ここで停止。value=out を GDB が丸ごと表示
```

`run_tests.sh --gdb` が生成する `.gdb`（`generated/probe.gdb`）は**対象非依存の汎用形**で、
メンバ名を列挙しない。

```gdb
rbreak ^gdb_probe
commands
  silent
  printf "\n[GDB] === probe: %s ===\n", label
  printf "(gdb) p value\n"
  p value
  continue
end
run
```

`gdb_probe` はテンプレートなので、実体はマングル名
`gdb_probe<T>(char const*, T const&)` になる。プレーン名の
`break gdb_probe` では張れないため、正規表現で全インスタンスに張れる
`rbreak ^gdb_probe` を使う。`__attribute__((noinline))` と `-O0` により、
観測点が最適化で消えないようにしている。

## 新しいテストを作るとき

詳細は [テストコード作成手順](docs/TEST_CREATION_GUIDE.md) を参照。

1. 新しいディレクトリ（例: `sclXxxTest/`）を `libGdb` と同じ階層に作る。
2. `lib/gdb_probe.hpp` をコピーする。
3. `main.cpp`（ドライバ）、`Makefile`、`run_tests.sh`、`cases/*.tsv` を、この sample
   または `sclComBatTest` を雛形に用意する。実ライブラリにリンクするテストは、
   `sclComBatTest/Makefile` を雛形にする。

## 補足

- 本 sample は業務ライブラリに依存せず、`USE_LIBS` による切替は廃止した。旧版にあった
  業務ライブラリのリンク定義は、`_old_structure/lib/Makefile` に残してある。
- `generated/` と `_old_structure/` は、版管理の対象にするか、除外するかを運用に合わせて決める。
- ビルド・実行（実 GDB）は Linux 実機（g++ + gdb）に依存する。
