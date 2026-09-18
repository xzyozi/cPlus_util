#include <string>
#include <vector>

#include "header.h"

// ==========================================
// 観測点は libGdb 共通の汎用マーカー gdb_probe(label, value) を使う。
// 対象固有のダミー関数は定義しない（対象非依存化）。
// 見せたい変数を gdb_probe に渡すだけで、生成される .gdb は
//   break gdb_probe / p value
// だけで済む（メンバ名の列挙が不要）。
// ==========================================

namespace {

// テストで一致させる基準の日付キー（テストデータと同じ値）
const char *const KEY_YM = "202407";
const char *const KEY_HDK = "1";
const char *const KEY_YMD = "20240729";

// 読み込み条件（一致データと同じキー）を組み立てる
ChildResultTest::ReadKey makeMatchKey(int expectedPid, int expectedFlg) {
  ChildResultTest::ReadKey key{};
  key.oya_process_id = expectedPid;
  key.cal_result_flg = expectedFlg;
  key.cal_ym = KEY_YM;
  key.cal_heidokyu = KEY_HDK;
  key.cal_ymd = KEY_YMD;
  return key;
}

// 1ケース分の read を実行し、観測点で入力条件と結果を見せる
void runCase(int expectedPid, int expectedFlg) {
  ChildResultTest::ReadKey key = makeMatchKey(expectedPid, expectedFlg);
  std::vector<ChildResultTest::CalResult> out;

  gdb_probe("read_key", key); // 入力条件のエビデンス（value=key）
  (void)ChildResultTest::readCalResult(key, out);
  gdb_probe("out_dump", out); // 読み込み結果のエビデンス（value=out）
}

} // namespace

// ---------------------------------------------------------
// Main
// ---------------------------------------------------------
int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;

  int expectedPid = ChildResultTest::test_get_oya_pid();
  int expectedFlg = ChildResultTest::VALID_FLG;

  // 1. 単一一致
  std::cout << "\n\n========== TEST: 単一一致 ==========" << std::endl;
  ChildResultTest::deleteResults();
  (void)ChildResultTest::setupSingleMatchData();
  runCase(expectedPid, expectedFlg);

  // 2. 複数一致
  std::cout << "\n\n========== TEST: 複数一致 ==========" << std::endl;
  ChildResultTest::deleteResults();
  (void)ChildResultTest::setupMultiMatchData();
  runCase(expectedPid, expectedFlg);

  // 3. 該当なし（ストアを空にして読み込み）
  std::cout << "\n\n========== TEST: 該当なし ==========" << std::endl;
  ChildResultTest::deleteResults();
  runCase(expectedPid, expectedFlg);

  // 4. 親プロセスID不一致
  std::cout << "\n\n========== TEST: 親プロセスID不一致 =========="
            << std::endl;
  ChildResultTest::deleteResults();
  (void)ChildResultTest::setupUnmatchPidData();
  runCase(expectedPid, expectedFlg);

  // 5. 計算結果有効フラグ不一致
  std::cout << "\n\n========== TEST: 計算結果有効フラグ不一致 =========="
            << std::endl;
  ChildResultTest::deleteResults();
  (void)ChildResultTest::setupUnmatchFlgData();
  runCase(expectedPid, expectedFlg);

  // 6. 年月不一致
  std::cout << "\n\n========== TEST: 年月不一致 ==========" << std::endl;
  ChildResultTest::deleteResults();
  (void)ChildResultTest::setupUnmatchYmData();
  runCase(expectedPid, expectedFlg);

  // 7. 平土休不一致
  std::cout << "\n\n========== TEST: 平土休不一致 ==========" << std::endl;
  ChildResultTest::deleteResults();
  (void)ChildResultTest::setupUnmatchHdkData();
  runCase(expectedPid, expectedFlg);

  // 8. 年月日不一致
  std::cout << "\n\n========== TEST: 年月日不一致 ==========" << std::endl;
  ChildResultTest::deleteResults();
  (void)ChildResultTest::setupUnmatchYmdData();
  runCase(expectedPid, expectedFlg);

  return 0;
}
