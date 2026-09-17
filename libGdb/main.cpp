#include <vector>

#include "header.h"

#include <Cgt0920AbsDate.h>
#include <Cgt0927ChildResultKnr.h>
#include <Cmn0023UtilLog.h>
#include <mpf_mfs.h>

// ==========================================
// GDB用ダミー関数
// ==========================================
void gdb_dump_read_date(int pid, int flg, const char *ym, const char *hdk,
                        const char *ymd) {
  volatile int p = pid;
  volatile int f = flg;
  volatile const char *d1 = ym;
  volatile const char *d2 = hdk;
  volatile const char *d3 = ymd;
  (void)p;
  (void)f;
  (void)d1;
  (void)d2;
  (void)d3;
}

void gdb_dump_out(const std::vector<CGT_CAL_RESULT_St> &out) { (void)out; }

// ---------------------------------------------------------
// Main
// ---------------------------------------------------------
int main(int argc, char const *argv[]) {
  (void)argc;
  (void)argv;

  pmf_startproca(&argc, const_cast<char **>(argv), nullptr);
  Cgt0927ChildResultKnr::deleteResults();

  int expectedPid = ChildResultTest::test_get_oya_pid();
  int expectedFlg = CGT0927_VALID_FLG::VALID_FIELD;

  // 1. 単一一致
  std::cout << "\n\n========== TEST: 単一一致 ==========" << std::endl;
  (void)ChildResultTest::setupSingleMatchData();
  {
    std::vector<CGT_CAL_RESULT_St> out;
    Cgt0920AbsDate date("202407", "1", "20240729", std::vector<std::string>{});
    gdb_dump_read_date(expectedPid, expectedFlg, date.getCalYm().c_str(),
                       date.getHeiDoKyu().c_str(), date.getCalYmd().c_str());
    (void)Cgt0927ChildResultKnr::readCalResult(date, out);
    gdb_dump_out(out);
  }
  Cgt0927ChildResultKnr::deleteResults();

  // 2. 複数一致
  std::cout << "\n\n========== TEST: 複数一致 ==========" << std::endl;
  (void)ChildResultTest::setupMultiMatchData();
  {
    std::vector<CGT_CAL_RESULT_St> out;
    Cgt0920AbsDate date("202407", "1", "20240729", std::vector<std::string>{});
    gdb_dump_read_date(expectedPid, expectedFlg, date.getCalYm().c_str(),
                       date.getHeiDoKyu().c_str(), date.getCalYmd().c_str());
    (void)Cgt0927ChildResultKnr::readCalResult(date, out);
    gdb_dump_out(out);
  }
  // 該当なしの試験のためあえて削除しない
  // Cgt0927ChildResultKnr::deleteResults();

  // 3. 該当なし
  std::cout << "\n\n========== TEST: 該当なし ==========" << std::endl;
  {
    std::vector<CGT_CAL_RESULT_St> out;
    Cgt0920AbsDate date("202407", "1", "20240729", std::vector<std::string>{});
    gdb_dump_read_date(expectedPid, expectedFlg, date.getCalYm().c_str(),
                       date.getHeiDoKyu().c_str(), date.getCalYmd().c_str());
    (void)Cgt0927ChildResultKnr::readCalResult(date, out);
    gdb_dump_out(out);
  }
  // 削除済みなので deleteResults() は不要

  // 4. 親プロセスID不一致
  std::cout << "\n\n========== TEST: 親プロセスID不一致 =========="
            << std::endl;
  (void)ChildResultTest::setupUnmatchPidData();
  {
    std::vector<CGT_CAL_RESULT_St> out;
    Cgt0920AbsDate date("202407", "1", "20240729", std::vector<std::string>{});
    gdb_dump_read_date(expectedPid, expectedFlg, date.getCalYm().c_str(),
                       date.getHeiDoKyu().c_str(), date.getCalYmd().c_str());
    (void)Cgt0927ChildResultKnr::readCalResult(date, out);
    gdb_dump_out(out);
  }
  Cgt0927ChildResultKnr::deleteResults();

  // 5. 計算結果有効フラグ不一致
  std::cout << "\n\n========== TEST: 計算結果有効フラグ不一致 =========="
            << std::endl;
  (void)ChildResultTest::setupUnmatchFlgData();
  {
    std::vector<CGT_CAL_RESULT_St> out;
    Cgt0920AbsDate date("202407", "1", "20240729", std::vector<std::string>{});
    gdb_dump_read_date(expectedPid, expectedFlg, date.getCalYm().c_str(),
                       date.getHeiDoKyu().c_str(), date.getCalYmd().c_str());
    (void)Cgt0927ChildResultKnr::readCalResult(date, out);
    gdb_dump_out(out);
  }
  Cgt0927ChildResultKnr::deleteResults();

  // 6. 年月不一致
  std::cout << "\n\n========== TEST: 年月不一致 ==========" << std::endl;
  (void)ChildResultTest::setupUnmatchYmData();
  {
    std::vector<CGT_CAL_RESULT_St> out;
    Cgt0920AbsDate date("202407", "1", "20240729", std::vector<std::string>{});
    gdb_dump_read_date(expectedPid, expectedFlg, date.getCalYm().c_str(),
                       date.getHeiDoKyu().c_str(), date.getCalYmd().c_str());
    (void)Cgt0927ChildResultKnr::readCalResult(date, out);
    gdb_dump_out(out);
  }
  Cgt0927ChildResultKnr::deleteResults();

  // 7. 平土休不一致
  std::cout << "\n\n========== TEST: 平土休不一致 ==========" << std::endl;
  (void)ChildResultTest::setupUnmatchHdkData();
  {
    std::vector<CGT_CAL_RESULT_St> out;
    Cgt0920AbsDate date("202407", "1", "20240729", std::vector<std::string>{});
    gdb_dump_read_date(expectedPid, expectedFlg, date.getCalYm().c_str(),
                       date.getHeiDoKyu().c_str(), date.getCalYmd().c_str());
    (void)Cgt0927ChildResultKnr::readCalResult(date, out);
    gdb_dump_out(out);
  }
  Cgt0927ChildResultKnr::deleteResults();

  // 8. 年月日不一致
  std::cout << "\n\n========== TEST: 年月日不一致 ==========" << std::endl;
  (void)ChildResultTest::setupUnmatchYmdData();
  {
    std::vector<CGT_CAL_RESULT_St> out;
    Cgt0920AbsDate date("202407", "1", "20240729", std::vector<std::string>{});
    gdb_dump_read_date(expectedPid, expectedFlg, date.getCalYm().c_str(),
                       date.getHeiDoKyu().c_str(), date.getCalYmd().c_str());
    (void)Cgt0927ChildResultKnr::readCalResult(date, out);
    gdb_dump_out(out);
  }
  Cgt0927ChildResultKnr::deleteResults();

  return 0;
}
