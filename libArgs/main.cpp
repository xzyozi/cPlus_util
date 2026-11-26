/*
 * libArgs sample ドライバ（デモ兼テストドライバ）
 * 受け取った引数を libArgs で解析し、結果を標準出力へ出す。
 * run_tests.sh がケース表（cases/*.tsv）から1ケースずつ別プロセスで起動する。
 *
 *   libArgsSample run --id <ID> [--count <N>] [--ratio <R>] [-v] [--] [位置引数...]
 *   libArgsSample list [--all] [位置引数...]
 *   libArgsSample --help | <サブコマンド> --help
 *
 * 標準出力（run_tests.sh が合否判定と証跡に使う）:
 *   IN_ARGS=<受け取った引数を空白で連結>
 *   MSG=<エラーメッセージ>          … エラー時のみ
 *   RET=NML|ERR
 *   VALUE=<結果>
 *     run  : cmd=run id=<ID> count=<N> ratio=<R|-> verbose=<0|1> pos=<a,b|->
 *     list : cmd=list all=<0|1> pos=<a,b|->
 *     help : help=<サブコマンド名|global>（使い方本文は RET= の前に出す）
 *     エラー: error=<種別>（args::errorName() の名前）
 *
 * 終了コード: 0=成功（--help を含む）、2=引数エラー
 */
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "args.hpp"

namespace {

const int EXIT_OK = 0;
const int EXIT_ARG_ERROR = 2;

// 位置引数を "," で連結する。なければ "-"
std::string joinPositionals(const std::vector<std::string> &items) {
  if (items.empty()) {
    return "-";
  }
  std::string joined;
  for (std::size_t i = 0; i < items.size(); ++i) {
    if (0 != i) {
      joined += ",";
    }
    joined += items[i];
  }
  return joined;
}

int reportError(args::ErrorKind kind, const std::string &message) {
  std::cout << "MSG=" << message << "\n";
  std::cout << "RET=ERR\n";
  std::cout << "VALUE=error=" << args::errorName(kind) << "\n";
  return EXIT_ARG_ERROR;
}

int handleRun(const args::Result &result) {
  std::string id;
  result.getString("id", id); // 必須なので解析成功時は必ずある

  int count = 0;
  if (!result.getInt("count", count)) {
    return reportError(args::ErrorKind::InvalidValue,
                       "--count は整数で指定してください");
  }

  std::string ratioText = "-";
  if (result.has("ratio")) {
    double ratio = 0.0;
    if (!result.getDouble("ratio", ratio)) {
      return reportError(args::ErrorKind::InvalidValue,
                         "--ratio は数値で指定してください");
    }
    std::ostringstream os;
    os << ratio;
    ratioText = os.str();
  }

  const bool verbose = result.flag("verbose");
  std::cout << "RET=NML\n";
  std::cout << "VALUE=cmd=run id=" << id << " count=" << count
            << " ratio=" << ratioText << " verbose=" << (verbose ? 1 : 0)
            << " pos=" << joinPositionals(result.positionals()) << "\n";
  return EXIT_OK;
}

int handleList(const args::Result &result) {
  const bool all = result.flag("all");
  std::cout << "RET=NML\n";
  std::cout << "VALUE=cmd=list all=" << (all ? 1 : 0)
            << " pos=" << joinPositionals(result.positionals()) << "\n";
  return EXIT_OK;
}

} // namespace

int main(int argc, char **argv) {
  args::Parser parser("libArgsSample", "libArgs のデモ兼テストドライバ");

  args::Command &run = parser.addCommand("run", "ジョブを実行する");
  run.required("id", 'i', "ジョブID")
      .withDefault("count", 'c', "1", "実行回数（整数）")
      .value("ratio", 'r', "比率（小数）")
      .flag("verbose", 'v', "詳細を表示する");

  args::Command &list = parser.addCommand("list", "ジョブ一覧を表示する");
  list.flag("all", 'a', "すべて表示する");

  /* 入力の証跡 */
  std::cout << "IN_ARGS=";
  for (int i = 1; i < argc; ++i) {
    std::cout << (1 == i ? "" : " ") << argv[i];
  }
  std::cout << "\n";

  const args::Result result = parser.parse(argc, argv);
  if (!result.ok()) {
    return reportError(result.errorKind(), result.error());
  }
  if (result.helpRequested()) {
    std::cout << result.helpText();
    std::cout << "RET=NML\n";
    std::cout << "VALUE=help="
              << (result.command().empty() ? "global" : result.command())
              << "\n";
    return EXIT_OK;
  }

  if ("run" == result.command()) {
    return handleRun(result);
  }
  if ("list" == result.command()) {
    return handleList(result);
  }
  // 登録済みサブコマンドはすべて上で処理するため、ここには来ない
  return reportError(args::ErrorKind::UnknownCommand,
                     "未対応のサブコマンドです: " + result.command());
}
