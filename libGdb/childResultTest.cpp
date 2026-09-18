#include "childResultTest.hpp"

#include <cstdlib>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>

// ============================================================
// 子プロセス計算結果 読み込み検証用テストデータ生成・書き込み／読み込み
//
// 業務コード（Cgt0927ChildResultKnr 等）・DB には依存しない。
// 書き込み先は業務 DB ではなくインメモリの vector（データストア）とし、
// 読み込みも自前の条件フィルタで行う。動き（テストケースの観点・
// データ内容・読み込み条件）は従来と同じ。
// ============================================================
namespace ChildResultTest {
namespace {
// インメモリのデータストア（業務の DB テーブル相当）
std::vector<CalResult> g_store;

// 固定長 char 配列へ安全にコピーする（末尾 NUL 保証）
template <std::size_t N> void setField(char (&dst)[N], const std::string &src) {
  std::memset(dst, 0, N);
  std::size_t n = src.size() < (N - 1) ? src.size() : (N - 1);
  std::memcpy(dst, src.c_str(), n);
}

// 固定長 char 配列と std::string の一致判定
template <std::size_t N>
bool fieldEquals(const char (&field)[N], const std::string &expected) {
  return expected.size() < N && std::strncmp(field, expected.c_str(), N) == 0;
}
} // namespace

int test_get_oya_pid() {
  const char *s = std::getenv("MEL_TEST_OYA_PID");
  if (s && *s) {
    char *end = nullptr;
    long v = std::strtol(s, &end, 10);
    if (end && *end == '\0' && v > 0)
      return static_cast<int>(v);
  }
  return static_cast<int>(getpid());
}

static void fillDate(CalResult &r, const std::string &ym,
                     const std::string &hdk, const std::string &ymd) {
  setField(r.cal_ym, ym);
  setField(r.cal_heidokyu, hdk);
  setField(r.cal_ymd, ymd);
}

// subfileNo を追加し、複数一致時のデータに差異を持たせる
CalResult makeRec(const std::string &ym, const std::string &hdk,
                  const std::string &ymd, bool isValidFlg, int customOyaPid,
                  int subfileNo) {
  CalResult r{};
  if (isValidFlg) {
    r.cal_result_flg = VALID_FLG;
  } else {
    r.cal_result_flg = 0; // 無効フラグ
  }
  fillDate(r, ym, hdk, ymd);
  r.oya_process_id = customOyaPid;
  r.subfile_no = subfileNo; // 区別用の非キー項目
  return r;
}

void deleteResults() { g_store.clear(); }

int writeResults_TEST(std::vector<CalResult> &vData) {
  int oyaProcessId = test_get_oya_pid();
  std::string prgName = "testChildPrcKnr"; // 旧 pmf_getprgname 相当（固定）
  int koPidx = 0;                          // 旧 pmf_getpidx 相当（固定）

  for (auto &data : vData) {
    if (data.oya_process_id == 0) {
      data.oya_process_id = oyaProcessId;
    }
    setField(data.ko_proc_name, prgName);
    data.ko_pidx = koPidx;
    g_store.push_back(data);
  }
  return RETURN_OK;
}

int readCalResult(const ReadKey &key, std::vector<CalResult> &out) {
  out.clear();
  for (const auto &rec : g_store) {
    // 全キー一致のみ抽出（業務 readCalResult の条件相当）
    if (rec.oya_process_id != key.oya_process_id)
      continue;
    if (rec.cal_result_flg != key.cal_result_flg)
      continue;
    if (!fieldEquals(rec.cal_ym, key.cal_ym))
      continue;
    if (!fieldEquals(rec.cal_heidokyu, key.cal_heidokyu))
      continue;
    if (!fieldEquals(rec.cal_ymd, key.cal_ymd))
      continue;
    out.push_back(rec);
  }
  return RETURN_OK;
}

// ---------------------------------------------------------
// 各テストケース用データセットアップ関数
// ---------------------------------------------------------
int setupSingleMatchData() {
  std::vector<CalResult> v{makeRec("202407", "1", "20240729")};
  return writeResults_TEST(v);
}

int setupMultiMatchData() {
  // 複数一致: キーは同じだが、subfile_no を 1, 2, 3 と変えて差異を持たせる
  std::vector<CalResult> v{makeRec("202407", "1", "20240729", true, 0, 1),
                           makeRec("202407", "1", "20240729", true, 0, 2),
                           makeRec("202407", "1", "20240729", true, 0, 3)};
  return writeResults_TEST(v);
}

int setupUnmatchPidData() {
  std::vector<CalResult> v{
      makeRec("202407", "1", "20240729"),             // 一致
      makeRec("202407", "1", "20240729", true, 99999) // PID不一致
  };
  return writeResults_TEST(v);
}

int setupUnmatchFlgData() {
  std::vector<CalResult> v{
      makeRec("202407", "1", "20240729"),       // 一致
      makeRec("202407", "1", "20240729", false) // フラグ不一致
  };
  return writeResults_TEST(v);
}

int setupUnmatchYmData() {
  std::vector<CalResult> v{
      makeRec("202407", "1", "20240729"), // 一致
      makeRec("202408", "1", "20240729")  // 年月不一致
  };
  return writeResults_TEST(v);
}

int setupUnmatchHdkData() {
  std::vector<CalResult> v{
      makeRec("202407", "1", "20240729"), // 一致
      makeRec("202407", "2", "20240729")  // 平土休不一致
  };
  return writeResults_TEST(v);
}

int setupUnmatchYmdData() {
  std::vector<CalResult> v{
      makeRec("202407", "1", "20240729"), // 一致
      makeRec("202407", "1", "20240730")  // 年月日不一致
  };
  return writeResults_TEST(v);
}

} // namespace ChildResultTest
