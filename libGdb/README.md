# libGdb エビデンス採取テストフレームワーク（sample）

GDB を使った C++ 単体検証で、エビデンス（証跡）を楽に採取するための作業用
ツール。テスト対象を実行し、GDB のネイティブ出力（`p 変数` の型付き表示）を
そのまま証跡として残すことを狙う。現状は **sample（お手本）** として扱う。

設計の詳細な検討記録は `../docs/features/gdb_issue.md` を参照。

## 実行環境

- Linux + g++ + gdb（**PostgreSQL は不要**）
- 版管理は Git

## ディレクトリ構成（役割で3分割）

```
libGdb/
├── README.md
├── .gitignore          # generated/ を除外
├── lib/                # 【ライブラリ本体】固定資産。基本は触らない
│   ├── Makefile        #   ビルド定義（USE_LIBS 切替・generated を作り直す）
│   ├── gdb_debug.sh    #   build → .gdb 自動生成 → test 実行 のランナー
│   └── header.h        #   共通ヘッダ
├── src/                # 【可変ファイル】sample: 自分のテスト対象を書く場所
│   ├── main.cpp        #   テストシナリオ（ケースを実行順に並べる）
│   ├── childResultTest.hpp  #  テストデータ生成・書き込み/読み込み（業務非依存）
│   └── childResultTest.cpp  #    同上（インメモリのデータストアで動作）
└── generated/          # 【自動生成物】make で毎回作り直す（Git 管理外）
    ├── obj/            #   オブジェクトファイル
    ├── testChildPrcKnr #   実行ファイル
    └── dump_generated.gdb  # 自動生成される GDB スクリプト
```

- **lib/（本体）**: ビルドと実行の仕組み。原則ユーザーは編集しない。
- **src/（可変）**: 検証したい関数・データをユーザーが書く場所。sample として
  `childResultTest.*` と `main.cpp` を同梱している。
- **generated/（生成物）**: `make` 実行時に **毎回削除して作り直す**。
  `.gitignore` で除外しているのでコミット対象にならない。

## 使い方（build → .gdb 自動生成 → test 実行）

`lib/gdb_debug.sh` が次の3ステップを1本で行う。**`lib/` で実行する。**

1. `make` で build（既定は `USE_LIBS=0`＝スタンドアローン）。
   make が `generated/` を作り直し、実行ファイルを `generated/` に出力する。
2. dump 用の `.gdb`（`generated/dump_generated.gdb`）を自動生成
3. 生成した `.gdb` で `gdb` を起動し、test を実行してエビデンスを採取

```bash
cd lib

# スタンドアローン build → 実行
./gdb_debug.sh

# 業務ライブラリを使う build → 実行（make へ引数を透過）
./gdb_debug.sh USE_LIBS=1
```

### ビルドだけ行う場合（lib/ で実行）

```bash
cd lib
make            # スタンドアローン（USE_LIBS=0・標準ライブラリのみ）
make USE_LIBS=1 # 業務 INCLUDE/LIBS をリンク
make clean      # generated/ を削除
```

`make` は実行のたびに `generated/` を丸ごと削除してから作り直す。

## ビルドモード（USE_LIBS）

| モード                   | 指定         | 内容                                                                   |
| ------------------------ | ------------ | ---------------------------------------------------------------------- |
| スタンドアローン（既定） | `USE_LIBS=0` | 業務 INCLUDE/LIBS を使わず標準ライブラリのみ。観測点を残すため `-O0`。 |
| 業務ライブラリ使用       | `USE_LIBS=1` | 業務 INCLUDE/LIBS をリンク（パスは実環境に合わせて調整）。             |

## 自分のテスト対象を追加するとき

`src/` を編集する（`lib/` と `generated/` は触らない）。

1. `src/` にテストデータ生成・読み込みロジックを書く（`childResultTest.*` が雛形）
2. `src/main.cpp` のシナリオに、観測点ダミー関数（`gdb_dump_read_date` /
   `gdb_dump_out`）を要所に置いて呼ぶ
3. `lib/` で `./gdb_debug.sh` を実行すると、build → .gdb 生成 → 実行まで走る

観測点で見せたいメンバを変えたい場合は、`lib/gdb_debug.sh` が生成する
`.gdb` のヒアドキュメント部分（`p` 出力）を調整する。

## エビデンスのログ出力

- ログのファイル出力機構はフラグ管理とし、**既定は off**（GDB 出力を見ることが
  主目的で、記録ファイルは副次的）。フラグの外部設定ファイル読み込みは
  ログ機構本体に持たせず、別レイヤーの責務とする方針。

## 補足

- `src/` の `main.cpp` / `childResultTest.*` は業務コード・DB 非依存へ移行済み。
  `USE_LIBS=0`（スタンドアローン）でビルドできる構成。
- ビルド・実行は Linux 実機（g++ + gdb）に依存する。
