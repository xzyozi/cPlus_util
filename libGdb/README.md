# libGdb エビデンス採取テストフレームワーク

GDB を使った C++ 単体検証で、エビデンス（証跡）を楽に採取するための作業用
ツール。テスト対象を実行し、GDB のネイティブ出力（`p 変数` の型付き表示）を
そのまま証跡として残すことを狙う。

設計の詳細な検討記録は `../docs/features/gdb_issue.md` を参照。

## 実行環境

- Linux + g++ + gdb（**PostgreSQL は不要**）
- 版管理は Git

## ディレクトリ構成

```
libGdb/
├── README.md            # このファイル
├── Makefile             # ビルド（USE_LIBS でスタンドアローン/業務ライブラリ切替）
├── gdb_debug.sh         # build → .gdb 自動生成 → test 実行 を1本で行うランナー
├── main.cpp             # テストシナリオ（ケースを実行順に並べる）
├── childResultTest.hpp  # テストデータ生成・書き込み/読み込み（業務コード非依存）
├── childResultTest.cpp  #   同上（インメモリのデータストアで動作）
├── header.h             # 共通ヘッダ
├── auto_read.gdb        # 旧世代の GDB スクリプト（整理候補）
├── manual_debug.gdb     # 旧世代の GDB スクリプト（整理候補）
└── libGdb/              # サブディレクトリ（ルート側と重複、整理候補）
    ├── auto_read.gdb
    ├── gdb_debug.sh
    └── manual_debug.gdb
```

## 使い方（build → .gdb 自動生成 → test 実行）

`gdb_debug.sh` が次の3ステップを1本で行う。

1. `make` で build（既定は `USE_LIBS=0`＝スタンドアローン）
2. dump 用の `.gdb`（`dump_generated.gdb`）をスクリプトが自動生成
3. 生成した `.gdb` で `gdb` を起動し、test を実行してエビデンスを採取

```bash
# スタンドアローン build → 実行
./gdb_debug.sh

# 業務ライブラリを使う build → 実行（make へ引数を透過）
./gdb_debug.sh USE_LIBS=1
```

### ビルドだけ行う場合

```bash
make            # スタンドアローン（USE_LIBS=0・標準ライブラリのみ）
make USE_LIBS=1 # 業務 INCLUDE/LIBS をリンク
make clean
```

## ビルドモード（USE_LIBS）

| モード | 指定 | 内容 |
| ------ | ---- | ---- |
| スタンドアローン（既定） | `USE_LIBS=0` | 業務 INCLUDE/LIBS を使わず標準ライブラリのみ。観測点を残すため `-O0`。 |
| 業務ライブラリ使用 | `USE_LIBS=1` | 従来どおり業務 INCLUDE/LIBS をリンク。 |

## 現状の制約（重要）

- **`main.cpp` は業務コード（Cgt0927 等）に依存したまま。** そのため
  スタンドアローン（`USE_LIBS=0`）での `make` は現状ビルドが通らない。
  スタンドアローンで通すには `main.cpp` の依存除去が別途必要。
  `gdb_debug.sh` は build → .gdb 生成 → 実行という「流れ」を固定する枠組みで
  あり、`main.cpp` の依存除去が完了すればそのまま一気通貫で動く。
- `childResultTest.hpp` / `childResultTest.cpp` は業務コード・DB 非依存へ
  移行済み（インメモリのデータストアで「登録 → 条件で読み込み → 結果」を
  再現。動作は従来同等）。
- ルート直下と `libGdb/` サブディレクトリに重複する GDB スクリプト・ランナーが
  残っている（整理候補）。

## エビデンスのログ出力

- ログのファイル出力機構はフラグ管理とし、**既定は off**（GDB 出力を見ることが
  主目的で、記録ファイルは副次的）。フラグの外部設定ファイル読み込みは
  ログ機構本体に持たせず、別レイヤーの責務とする方針。
