# libConf 設定ファイル管理ライブラリ（sample）

`key=value` 形式の設定ファイルを読み込み、プログラムを止めずに設定変更を反映するための
ライブラリ。設定項目は `confData.hpp` の1か所で定義し、項目名・既定値・説明を一括管理する。

## 実行環境

- Linux + g++ + make（外部ライブラリには依存しない。C++11 以降）
- `conf.cpp` には Windows（`_WIN32`）向けの分岐もあるが、Makefile は Linux 前提

## ディレクトリ構成

```
libConf/
├── README.md
├── Makefile            # ビルド。実行ファイル confmng と obj/ を libConf/ 直下に出力する
├── header.h            # main.cpp が読み込む共通ヘッダー
├── main.cpp            # デモ。3秒ごとに live.conf の変更を確認して再読込する
├── config/
│   ├── boot.conf       # 起動時に読む設定
│   └── live.conf       # 実行中に変更する設定（更新日時で変更を検出）
└── libConf/
    ├── conf.hpp        # ConfigManager の宣言
    ├── conf.cpp        # ConfigManager の実装
    └── confData.hpp    # 設定項目の定義（X マクロ CONFIG_PARAMS）
```

## 使い方

```bash
cd libConf
make            # ./confmng を作る
./confmng       # 起動して設定値を表示し、live.conf の変更を待つ（Ctrl+C で終了）
make clean      # obj/*.o と confmng を削除
```

実行中に別の端末で `config/live.conf` を書き換えると、次の確認タイミング（3秒ごと）で
`[Config] Detected change. Reloading...` と表示され、新しい値が反映される。

## 動作の流れ

1. `ConfigManager` の生成時、`config/` がなければ作成する。
2. `config/boot.conf` を読む。ファイルがなければ既定値で作成する。
3. `config/live.conf` を読む。ファイルがなければ既定値で作成する。後から読んだ値が優先される。
4. `checkAndReload()` を呼ぶたびに `live.conf` の更新日時を確認し、変わっていれば再読込して
   `true` を返す。

設定ファイルのパスはカレントディレクトリからの相対パス（`config/boot.conf`、`config/live.conf`）。
実行ファイルは `libConf/` で起動する。

## 設定ファイルの書式

- 1行1項目で `項目名=値` と書く。前後の空白は無視する。
- `#` で始まる行と空行は無視する。
- `confData.hpp` に定義されていない項目名、`=` のない行は無視する。
- 値は `int` として読む。数値に変換できない値は無視し、変更前の値を保つ。

```
# [ retry_limit ]
retry_limit=2
```

既定値で作成されたファイルには、項目ごとに用途・未使用可否・既定値のコメントが付く。

## 設定項目を追加する

`libConf/confData.hpp` の `CONFIG_PARAMS` に1行追加するだけでよい。メンバ変数の定義、
読み込み時の項目名の対応付け、既定ファイルへの書き出しは、このマクロから自動で生成される。

```cpp
#define CONFIG_PARAMS(X) \
    X(exec_mode,     1,  "実行モード切替。現在唯一使用中。", "使用中。値変更で動作に影響。") \
    ...
    X(port_id,       1,  "使用ポート識別子。",               "未使用。") \
    X(new_param,     5,  "追加した項目の用途。",             "未使用。")
```

引数は順に、項目名（変数名）、既定値、用途の説明、未使用可否の説明。

既存の `config/*.conf` には新しい項目は自動で追記されない。新しい項目は既定値で動作する。
ファイルにも項目を載せたい場合は、手で追記するか、ファイルを削除して作り直させる。

## プログラムからの使い方

```cpp
#include "libConf/conf.hpp"

ConfigManager conf;                       // boot.conf → live.conf の順に読み込む
int limit = conf.get().retry_limit;       // 値の参照（読み取り専用）

if (conf.checkAndReload()) {              // live.conf が変わっていれば再読込
    limit = conf.get().retry_limit;
}
```

## 制約

- 値の型は `int` のみ。文字列や小数は扱えない。
- 設定ファイルのパスは固定（`conf.hpp` の `BOOT_PATH` / `LIVE_PATH`）。
- 変更の検出はファイルの更新日時（秒単位）による。同じ秒の中で2回書き換えると、2回目を
  検出しない場合がある。
- 再読込は `checkAndReload()` を呼んだときだけ行う。自動では監視しない。
- スレッドセーフではない。
