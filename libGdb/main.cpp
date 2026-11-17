/*
 * libGdb sample ドライバ
 * sampleTarget の findItems() を1ケースずつ実行し、結果を標準出力へ出す。
 * ケースは run_tests.sh
 * がケース表（cases/*.tsv）から1ケースずつ別プロセスで起動する。
 *
 *   libGdbSample sample <case>
 *
 *   case: one | many | mixed | none | invalid
 *
 * 標準出力（run_tests.sh が合否判定と証跡に使う）:
 *   IN_CASE=<ケース名>
 *   IN_QUERY=<検索条件>
 *   RET=NML|ERR
 *   VALUE=count=<返った件数>
 *
 * 終了コード: 0=NML、1=ERR、2=引数不正
 *
 * 観測点は libGdb 共通の汎用マーカー gdb_probe(label, value) を使う。
 * 対象固有のダミー関数は定義しない。見せたい変数を gdb_probe に渡すだけで、
 * 生成される .gdb は  rbreak ^gdb_probe / p value  だけで済む。
 */
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "gdb_probe.hpp" // lib/ の正本。実テストではコピーして使う
#include "sampleTarget.hpp"

namespace {

void printUsage(const char *prog) {
  std::cerr << "使い方:\n"
            << "  " << prog << " sample <case>\n"
            << "  case: one many mixed none invalid\n"
            << "出力: RET=NML|ERR / VALUE=count=<件数>\n"
            << "終了コード: 0=NML 1=ERR 2=引数不正\n";
}

// ケースに対応する検索条件とデータを用意する。未知のケースは false。
bool setupCase(const std::string &name, SampleTarget::Query &query) {
  SampleTarget::clearStore();
  query.grp = 1;
  query.code = "A01";

  if (name == "one") {
    SampleTarget::setupOne();
  } else if (name == "many") {
    SampleTarget::setupMany();
  } else if (name == "mixed") {
    SampleTarget::setupMixed();
  } else if (name == "none") {
    /* ストアを空にして検索する */
  } else if (name == "invalid") {
    SampleTarget::setupOne();
    query.code = ""; /* 不正な条件（code が空） */
  } else {
    return false;
  }
  return true;
}

} // namespace

int main(int argc, char **argv) {
  if (argc == 2 && std::strcmp(argv[1], "-h") == 0) {
    printUsage(argv[0]);
    return 0;
  }
  if (argc != 3 || std::strcmp(argv[1], "sample") != 0) {
    printUsage(argv[0]);
    return 2;
  }

  const std::string caseName = argv[2];
  SampleTarget::Query query;
  if (!setupCase(caseName, query)) {
    printUsage(argv[0]);
    return 2;
  }

  /* 入力の証跡（gdb 経由でない通常実行でも evidence に残す） */
  std::cout << "IN_CASE=" << caseName << std::endl;
  std::cout << "IN_QUERY=grp=" << query.grp << ",code=" << query.code
            << std::endl;

  std::vector<SampleTarget::Item> out;
  gdb_probe("query", query); // 入力条件のエビデンス（value=query）
  const int rc = SampleTarget::findItems(query, out);
  gdb_probe("out_dump", out); // 結果のエビデンス（value=out）

  const bool ok = (rc == SampleTarget::RETURN_OK);
  std::cout << "RET=" << (ok ? "NML" : "ERR") << std::endl;
  std::cout << "VALUE=count=" << out.size() << std::endl;
  return ok ? 0 : 1;
}
