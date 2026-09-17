#ifndef CHILD_RESULT_TEST_HPP
#define CHILD_RESULT_TEST_HPP

#include <string>
#include <vector>

#include <Cgt0927ChildResultKnr.h>

// ============================================================
// 子プロセス計算結果（CGT_CAL_RESULT_St）読み込み検証用の
// テストデータ生成・書き込みロジック。
// 既存の main.cpp からロジックをそのまま流用して切り出したもの。
// ============================================================
namespace ChildResultTest
{
    // 親プロセスID取得（環境変数 MEL_TEST_OYA_PID で差し替え可能。既定は自PID）
    int test_get_oya_pid();

    // 1レコード生成
    CGT_CAL_RESULT_St makeRec(const std::string &ym,
                              const std::string &hdk,
                              const std::string &ymd,
                              bool               isValidFlg   = true,
                              int                customOyaPid = 0,
                              int                subfileNo    = 0);

    // 結果レコードの書き込み
    int writeResults_TEST(std::vector<CGT_CAL_RESULT_St> &vData);

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
