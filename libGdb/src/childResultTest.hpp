#ifndef CHILD_RESULT_TEST_HPP
#define CHILD_RESULT_TEST_HPP

#include <string>
#include <vector>

// ============================================================
// 子プロセス計算結果 読み込み検証用の
// テストデータ生成・書き込み／読み込みロジック。
//
// 業務コード（Cgt0927ChildResultKnr 等）・DB には依存しない。
// 業務構造体 CGT_CAL_RESULT_St の代わりに、同じフィールドを持つ
// 自前 POD 構造体を使い、書き込み先はインメモリのデータストアとする。
// 動き（テストケースの観点・データ内容・読み込み条件）は従来と同じ。
// ============================================================
namespace ChildResultTest {
// 有効フラグの既定値（業務の CGT0927_VALID_FLG::VALID_FIELD 相当）
static const int VALID_FLG = 1;

// 戻り値（業務の CGT0927_RETURN 相当）
enum { RETURN_OK = 0, RETURN_NG = 1 };

// 業務構造体 CGT_CAL_RESULT_St の代わりの自前 POD。
// 固定長 char 配列は、GDB の p 表示（(char*)&cal_ym[0] 等）を
// 従来と揃えるため。
struct CalResult {
  int oya_process_id;    // 親プロセスID（キー）
  int cal_result_flg;    // 計算結果有効フラグ（キー）
  char cal_ym[8];        // 年月     "YYYYMM"（キー）
  char cal_heidokyu[4];  // 平土休（キー）
  char cal_ymd[10];      // 年月日   "YYYYMMDD"（キー）
  int subfile_no;        // 区別用の非キー項目
  char ko_proc_name[64]; // 子プロセス名（非キー）
  int ko_pidx;           // 子プロセスインデックス（非キー）
};

// 読み込み条件（業務の日付キー相当）
struct ReadKey {
  int oya_process_id; // 期待する親プロセスID
  int cal_result_flg; // 期待する有効フラグ
  std::string cal_ym;
  std::string cal_heidokyu;
  std::string cal_ymd;
};

// 親プロセスID取得（環境変数 MEL_TEST_OYA_PID で差し替え可能。既定は自PID）
int test_get_oya_pid();

// 1レコード生成
CalResult makeRec(const std::string &ym, const std::string &hdk,
                  const std::string &ymd, bool isValidFlg = true,
                  int customOyaPid = 0, int subfileNo = 0);

// データストアをクリアする（業務の deleteResults 相当）
void deleteResults();

// 結果レコードの書き込み（インメモリのデータストアへ登録）
int writeResults_TEST(std::vector<CalResult> &vData);

// 条件一致レコードの読み込み（業務の readCalResult 相当）。
// 全キー（PID / flg / ym / hdk / ymd）が一致するものだけ out へ返す。
int readCalResult(const ReadKey &key, std::vector<CalResult> &out);

// ------------------------------------------------------------
// 各テストケース用データセットアップ関数
// ------------------------------------------------------------
int setupSingleMatchData();
int setupMultiMatchData();
int setupUnmatchPidData();
int setupUnmatchFlgData();
int setupUnmatchYmData();
int setupUnmatchHdkData();
int setupUnmatchYmdData();

} // namespace ChildResultTest

#endif // CHILD_RESULT_TEST_HPP
