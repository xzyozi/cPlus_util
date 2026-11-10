# libGdb テストコード作成手順

## 1. この手順書の対象

この手順書は、`libGdb` を使って新しいC++テストシナリオを追加するための手順です。

`libGdb` のテストは、次の2つを目的にします。

- テスト対象の入力・出力をGDBで確認し、証跡を採取する
- DBや業務ライブラリに依存しないモックで、条件分岐やデータ変換を再現する

このテストだけでは、実DBのSQL、接続、トランザクション、ロック、NULLの扱いは検証できません。実DBの動作確認が必要な場合は、別途統合テストを作成してください。

## 2. 事前条件

実行環境はLinuxです。次のコマンドが利用できることを確認します。

```bash
g++ --version
gdb --version
make --version
```

PostgreSQLや業務ライブラリを使わないテストは、`USE_LIBS=0` のスタンドアローンモードで実行できます。

## 3. 変更するファイル

テスト追加時は、原則として `libGdb/src/` だけを変更します。

| ファイル | 役割 |
| --- | --- |
| `src/main.cpp` | テストシナリオ、期待値検証、GDB観測点 |
| `src/<name>Test.hpp` | テスト用構造体、関数宣言、定数 |
| `src/<name>Test.cpp` | モック、テストデータ、補助関数 |
| `lib/gdb_probe.hpp` | 共通のGDB観測点。原則変更しない |
| `lib/Makefile` | ビルド定義。原則変更しない |
| `lib/gdb_debug.sh` | ビルドとGDB実行のランナー。原則変更しない |
| `generated/` | ビルド時の生成物。直接編集しない |

既存の `childResultTest.hpp` と `childResultTest.cpp` は、テスト用構造体とインメモリモックのサンプルです。

## 4. テストコードの作成手順

### 4.1 テスト対象と確認項目を決める

最初に、何を確認するテストなのかを決めます。

例:

- 入力キーの全項目が一致したレコードだけ返す
- 一致するレコードが複数ある場合に全件返す
- PIDや日付が不一致のレコードを除外する
- 固定長文字列へ正しく格納する
- エラー時に期待した戻り値を返す

各テストケースには、成功条件を明記します。GDB出力の目視確認だけにせず、可能な範囲で `assert` による自動判定も追加してください。

### 4.2 テスト用の型とAPIを定義する

業務型を直接利用できない場合は、`src/<name>Test.hpp` にテスト用の型を定義します。

```cpp
struct ReadKey {
  int id;
  std::string code;
};

struct Result {
  int id;
  char code[16];
};

int readResult(const ReadKey &key, std::vector<Result> &out);
```

実業務型の代替であることが分かるようにコメントを書きます。フィールド長、キー項目、非キー項目も明記してください。

### 4.3 モックを実装する

DBや外部サービスの代わりに、テスト対象の動作に必要な最小限のインメモリモックを実装します。

```cpp
namespace {
std::vector<Result> g_store;
}

void clearResults() {
  g_store.clear();
}

int readResult(const ReadKey &key, std::vector<Result> &out) {
  out.clear();
  for (const auto &record : g_store) {
    if (record.id == key.id &&
        std::string(record.code) == key.code) {
      out.push_back(record);
    }
  }
  return RETURN_OK;
}
```

モックには、テスト対象の確認に不要な機能を追加しません。モックの戻り値、入力値の副作用、データの初期化方法は、コメントまたはAPIの説明で明確にしてください。

### 4.4 テストデータを準備する

テストケースごとに、期待するレコードと除外するレコードを用意します。

```cpp
int setupUnmatchedIdData() {
  std::vector<Result> data{
      makeResult(100, "A"), // 一致
      makeResult(200, "A")  // ID不一致
  };
  return writeResults_TEST(data);
}
```

不一致テストでは、次のように「一致するデータ」と「1条件だけ異なるデータ」を混在させると、条件が正しく適用されたことを確認できます。

- ID不一致
- フラグ不一致
- 年月不一致
- 日付不一致
- 文字列不一致
- 該当データなし
- 複数件一致

テストケース間でデータが残らないよう、各ケースの開始時にストアをクリアします。

### 4.5 `main.cpp` にシナリオを追加する

1ケースにつき、次の順序で処理します。

1. ストアをクリアする
2. テストデータを登録する
3. 入力キーを作成する
4. テスト対象を呼び出す
5. 戻り値と結果を検証する
6. `gdb_probe` で入力と出力を観測する

```cpp
void runCase(const ReadKey &key, std::size_t expectedCount) {
  std::vector<Result> out;

  gdb_probe("read_key", key);
  int rc = readResult(key, out);
  gdb_probe("out_dump", out);

  assert(rc == RETURN_OK);
  assert(out.size() == expectedCount);
}
```

GDBで確認したい値は、テスト対象の呼び出し前後で `gdb_probe` に渡します。

```cpp
gdb_probe("input", key);
readResult(key, out);
gdb_probe("output", out);
```

### 4.6 自動判定を追加する

GDB出力は証跡として利用し、テストの合否はプログラムでも判定します。

```cpp
#include <cassert>

assert(rc == RETURN_OK);
assert(out.size() == 1);
assert(out[0].id == expectedId);
```

エラー系テストでは、期待するエラーコードを明示します。

```cpp
assert(rc == RETURN_NG);
assert(out.empty());
```

`assert` が無効になるビルド設定を使う場合は、終了コードを返す独自の検証関数など、別の自動判定方法を使用してください。

## 5. ビルドと実行

`libGdb/lib/` に移動して、スタンドアローンモードで実行します。

```bash
cd lib
make
./gdb_debug.sh
```

`gdb_debug.sh` は、次の処理を順番に実行します。

1. `generated/` を作り直してビルドする
2. `generated/dump_generated.gdb` を生成する
3. GDBで実行ファイルを起動する
4. `gdb_probe` の停止点で値を表示する

ビルドだけを確認する場合は、次を実行します。

```bash
cd lib
make
```

業務ライブラリを使うテストでは、環境を確認したうえで次を実行します。

```bash
cd lib
make USE_LIBS=1
./gdb_debug.sh USE_LIBS=1
```

## 6. GDB出力の確認

`gdb_probe` のラベルを使って、入力と出力を対応付けます。

```text
[GDB] === probe: read_key ===
...
[GDB] === probe: out_dump ===
...
```

確認する内容は次のとおりです。

- 入力キーが期待値になっている
- 結果件数が期待値になっている
- 結果の各フィールドが期待値になっている
- 不一致データが結果に含まれていない
- 複数一致時に全件が返っている

`gdb_probe` はテンプレート関数です。新しい型を観測する場合は、その型を実際に `gdb_probe` に渡してください。

## 7. 完了チェックリスト

### 実装

- [ ] テストの目的と成功条件を決めた
- [ ] 変更を `src/` に限定した
- [ ] テスト用の型とAPIを定義した
- [ ] モックの責務を最小限にした
- [ ] テストケース間のデータをクリアした
- [ ] 一致系と不一致系を用意した
- [ ] 戻り値と結果を自動判定した
- [ ] GDBで必要な値を観測できるようにした

### 実行

- [ ] `make` が成功した
- [ ] `./gdb_debug.sh` が成功した
- [ ] プログラムの終了コードが成功を示した
- [ ] GDB出力に期待した入力と出力が表示された
- [ ] `generated/` の生成物をコミット対象にしていない

## 8. モックで検証できないもの

次の項目は、インメモリモックだけでは検証できません。

- SQLの構文とWHERE条件
- DBのNULL、CHAR、VARCHARの実際の比較
- 文字コードや照合順序
- トランザクションとロールバック
- 接続失敗、タイムアウト、SQLエラー
- ロック、並行実行、永続化
- 実際の業務ライブラリの初期化とABI

これらを確認する場合は、実DBまたは実業務ライブラリを使う統合テストを別に作成してください。
