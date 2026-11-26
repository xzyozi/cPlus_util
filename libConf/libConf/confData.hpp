#pragma once

//////////////////////////////////////////////////////////////////////////////////
// 設定定義リスト デフォルト値はここで管理します
//////////////////////////////////////////////////////////////////////////////////
#define CONFIG_PARAMS(X)                                                       \
  X(exec_mode, 1, "実行モード切替。現在唯一使用中。",                          \
    "使用中。値変更で動作に影響。")                                            \
  X(log_level, 0, "ログ出力レベル制御。詳細度設定。", "未使用。")              \
  X(retry_limit, 2, "再試行回数の上限。通信制御用。", "未使用。")              \
  X(check_limit, 2, "検査ループ回数の上限。", "未使用。")                      \
  X(threshold_val, 10, "閾値判定用。入力値比較に利用。", "未使用。")           \
  X(timeout_ms, 20, "タイムアウト時間(ms)。", "未使用。")                      \
  X(interval_step, 3, "ポーリング間隔や再試行間隔。", "未使用。")              \
  X(do_skip, 1, "処理スキップ可否フラグ。", "未使用。")                        \
  X(enable_dev_a, 1, "デバイスA有効化フラグ。", "未使用。")                    \
  X(enable_dev_b, 0, "デバイスB有効化フラグ。", "未使用。")                    \
  X(port_id, 1, "使用ポート識別子。", "未使用。")

struct ConfData {
// マクロを展開してメンバ変数を定義
#define DEFINE_MEMBER(name, val, desc, usage) int name = val;
  CONFIG_PARAMS(DEFINE_MEMBER)
#undef DEFINE_MEMBER
};
