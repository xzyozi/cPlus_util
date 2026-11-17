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

このsampleは業務ライブラリに依存しません（標準ライブラリのみ）。実ライブラリにリンクするテストは、`sclComBatTest` の Makefile を雛形にします。

## 3. 変更するファイル

このsampleでは、テスト追加時に次のファイルを変更します。構成は `sclComBatTest` / `sclWebApiTest` と同じフラット形式です。
別ツールへ展開する場合は、`lib/gdb_probe.hpp` を新しいツールのディレクトリへコピーして使い、ほかのファイルは各ツールの対象・リンク構成に合わせて用意します。

| ファイル            | 役割                                                                      |
| ------------------- | ------------------------------------------------------------------------- |
| `main.cpp`          | ドライバ。ケース名を受け取り1ケース実行し、`IN_*=` `RET=` `VALUE=` を出力 |
| `<name>Test.hpp`    | テスト用構造体、関数宣言、定数                                            |
| `<name>Test.cpp`    | モック、テストデータ、補助関数                                            |
| `cases/*.tsv`       | ケース表（ID、ケース名、期待する戻り値と出力値）                          |
| `run_tests.sh`      | ケース表を1ケースずつ別プロセスで実行し、良/否を判定する                  |
| `Makefile`          | ビルド定義。出力は `generated/`                                           |
| `lib/gdb_probe.hpp` | 共通のGDB観測点の正本。コピーして使い、通常は変更しない                   |
| `generated/`        | 実行ファイル・証跡・結果。直接編集しない                                  |

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

各テストケースには、成功条件を明記します。GDB出力の目視確認だけにせず、ケース表（`cases/*.tsv`）の期待値で自動判定できるようにしてください。

### 4.2 テスト用の型とAPIを定義する

業務型を直接利用できない場合は、`<name>Test.hpp` にテスト用の型を定義します。

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

### 4.5 `main.cpp`（ドライバ）にケースを追加する

ドライバは、ケース名を受け取って1ケースだけ実行し、結果を標準出力へ出します。
複数ケースの実行は `run_tests.sh` が行い、ケースごとに別プロセスで起動します。

1ケースにつき、次の順序で処理します。

1. ストアをクリアし、ケースに応じたテストデータを登録する
2. 入力キーを作成する
3. 入力を `IN_*=` で標準出力へ出す
4. `gdb_probe` で入力を観測し、テスト対象を呼び出し、`gdb_probe` で出力を観測する
5. `RET=` と `VALUE=` を標準出力へ出す

```cpp
std::cout << "IN_CASE=" << caseName << std::endl;

std::vector<Result> out;
gdb_probe("read_key", key);
const int rc = readResult(key, out);
gdb_probe("out_dump", out);

std::cout << "RET=" << (rc == RETURN_OK ? "NML" : "ERR") << std::endl;
std::cout << "VALUE=count=" << out.size() << std::endl;
return (rc == RETURN_OK) ? 0 : 1;
```

GDBで確認したい値は、テスト対象の呼び出し前後で `gdb_probe` に渡します。

### 4.6 ケース表で自動判定する

合否は `cases/*.tsv` の期待値で判定します。GDB出力は証跡として使います。
`assert` ではなく、ドライバの出力（`RET=` と `VALUE=`）と、ケース表の期待値を `run_tests.sh` が照合します。

```text
# id	case	exp_ret	exp_value
C01	single	NML	count=1
C03	none	NML	count=0
```

- `exp_ret` は `NML` または `ERR`。
- `exp_value` はドライバの `VALUE=` と完全一致で照合する。`-` は照合しない。
- 不一致データを混ぜたケースは、期待件数を「一致したレコードだけ」の件数にする。

## 5. ビルドと実行

`libGdb/` で実行します。

```bash
make
./run_tests.sh            # 全ケースを実行し、良/否を判定する
./run_tests.sh --gdb      # gdb 経由で実行し、gdb_probe の値を証跡に保存する
```

`run_tests.sh` は、次の処理を行います。

1. `cases/*.tsv` を1行ずつ読み、ケースごとに別プロセスでドライバを起動する
2. 出力の `RET=` と `VALUE=` を期待値と照合し、`良` / `否` を判定する
3. `--gdb` 指定時は、`generated/probe.gdb` を生成し、`gdb_probe` の停止点で値を表示する
4. 結果を `generated/result_<日時>.tsv`、証跡を `generated/evidence/<実行日時>/<ID>.log` に保存する

`make` は `generated/` を削除しません。証跡を消す場合は `make clean` を使います。

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
- [ ] テスト用の型とAPIを定義した
- [ ] モックの責務を最小限にした
- [ ] テストケース間のデータをクリアした
- [ ] 一致系と不一致系を用意した
- [ ] ケース表（`cases/*.tsv`）に期待値を書いた
- [ ] ドライバが `IN_*=` `RET=` `VALUE=` を出力する
- [ ] GDBで必要な値を観測できるようにした

### 実行

- [ ] `make` が成功した
- [ ] `./run_tests.sh` が全件 `良` になった
- [ ] `./run_tests.sh --gdb` の証跡に、期待した入力と出力が表示された
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
