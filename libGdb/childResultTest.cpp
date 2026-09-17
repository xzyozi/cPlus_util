#include "childResultTest.hpp"

#include <cstdlib>
#include <string>
#include <unistd.h>
#include <vector>

#include <mpf_mfs.h>

// ============================================================
// 子プロセス計算結果 読み込み検証用テストデータ生成・書き込み
// （既存 main.cpp からロジックをそのまま流用）
// ============================================================
namespace ChildResultTest
{
    int test_get_oya_pid()
    {
        const char *s = std::getenv("MEL_TEST_OYA_PID");
        if (s && *s)
        {
            char *end = nullptr;
            long  v   = std::strtol(s, &end, 10);
            if (end && *end == '\0' && v > 0)
                return static_cast<int>(v);
        }
        return static_cast<int>(getpid());
    }

    static void fillDate(CGT_CAL_RESULT_St &r,
                         const std::string &ym,
                         const std::string &hdk,
                         const std::string &ymd)
    {
        CMN0012_STRCPY(r.cal_ym, ym.c_str());
        CMN0012_STRCPY(r.cal_heidokyu, hdk.c_str());
        CMN0012_STRCPY(r.cal_ymd, ymd.c_str());
    }

    // subfileNo を追加し、複数一致時のデータに差異を持たせる
    CGT_CAL_RESULT_St makeRec(const std::string &ym,
                              const std::string &hdk,
                              const std::string &ymd,
                              bool               isValidFlg,
                              int                customOyaPid,
                              int                subfileNo)
    {
        CGT_CAL_RESULT_St r{};
        if (isValidFlg)
        {
            r.cal_result_flg = CGT0927_VALID_FLG::VALID_FIELD;
        }
        else
        {
            r.cal_result_flg = 0; // 無効フラグ
        }
        fillDate(r, ym, hdk, ymd);
        r.oya_process_id = customOyaPid;
        r.subfile_no     = subfileNo; // 区別用の非キー項目
        return r;
    }

    int writeResults_TEST(std::vector<CGT_CAL_RESULT_St> &vData)
    {
        int              ret = 0;
        MPF_MFS_FCB      fcb;
        std::vector<int> vRecNo;

        int         oyaProcessId = test_get_oya_pid();
        std::string prgName      = pmf_getprgname();
        int         koPidx       = pmf_getpidx();

        for (auto &data : vData)
        {
            if (data.oya_process_id == 0)
            {
                data.oya_process_id = oyaProcessId;
            }
            CMN0012_STRCPY(data.ko_proc_name, prgName.c_str());
            data.ko_pidx = koPidx;
        }

        (void)mpf_mfs_openm(&fcb, NULL, FNO_CGT_CAL_RESULT, 0, MPF_MFS_WRITELOCK);
        ret = SdmAddRec(FNO_CGT_CAL_RESULT, 0, &fcb, vData, vRecNo, true);
        mpf_mfs_close(&fcb);

        if (SDM_NOMAL != ret)
            return CGT0927_RETURN::CGT0927_RETURN_NG;
        return CGT0927_RETURN::CGT0927_RETURN_OK;
    }

    // ---------------------------------------------------------
    // 各テストケース用データセットアップ関数
    // ---------------------------------------------------------
    int setupSingleMatchData()
    {
        std::vector<CGT_CAL_RESULT_St> v{makeRec("202407", "1", "20240729")};
        return writeResults_TEST(v);
    }

    int setupMultiMatchData()
    {
        // 複数一致: キーは同じだが、subfile_no を 1, 2, 3 と変えて差異を持たせる
        std::vector<CGT_CAL_RESULT_St> v{
            makeRec("202407", "1", "20240729", true, 0, 1),
            makeRec("202407", "1", "20240729", true, 0, 2),
            makeRec("202407", "1", "20240729", true, 0, 3)};
        return writeResults_TEST(v);
    }

    int setupUnmatchPidData()
    {
        std::vector<CGT_CAL_RESULT_St> v{
            makeRec("202407", "1", "20240729"),             // 一致
            makeRec("202407", "1", "20240729", true, 99999) // PID不一致
        };
        return writeResults_TEST(v);
    }

    int setupUnmatchFlgData()
    {
        std::vector<CGT_CAL_RESULT_St> v{
            makeRec("202407", "1", "20240729"),       // 一致
            makeRec("202407", "1", "20240729", false) // フラグ不一致
        };
        return writeResults_TEST(v);
    }

    int setupUnmatchYmData()
    {
        std::vector<CGT_CAL_RESULT_St> v{
            makeRec("202407", "1", "20240729"), // 一致
            makeRec("202408", "1", "20240729")  // 年月不一致
        };
        return writeResults_TEST(v);
    }

    int setupUnmatchHdkData()
    {
        std::vector<CGT_CAL_RESULT_St> v{
            makeRec("202407", "1", "20240729"), // 一致
            makeRec("202407", "2", "20240729")  // 平土休不一致
        };
        return writeResults_TEST(v);
    }

    int setupUnmatchYmdData()
    {
        std::vector<CGT_CAL_RESULT_St> v{
            makeRec("202407", "1", "20240729"), // 一致
            makeRec("202407", "1", "20240730")  // 年月日不一致
        };
        return writeResults_TEST(v);
    }

} // namespace ChildResultTest
