/*
 * libArgs: コマンドライン引数解析ライブラリ（C++17 標準ライブラリのみ）
 *
 * 機能:
 *   - サブコマンドの振り分け（prog run --id C01 / prog list）
 *     サブコマンドを1つも登録しなければ、root() の定義で単一コマンドとして解析する
 *   - 長いオプション --name value / --name=value、短いオプション -n value
 *   - フラグ（値なし。-v / --verbose）
 *   - -- 以降は位置引数。オプションでない単独トークンも位置引数
 *   - 必須・任意・既定値
 *   - 取得時の型変換 int / double / string / bool（末尾まで変換できた場合のみ成功）
 *   - --help / -h で使い方を自動生成
 *
 * 例外は投げない。解析結果（Result）にエラー種別とメッセージを持たせ、
 * 終了コードは呼び出し側が決める。
 */
#ifndef LIBARGS_ARGS_HPP
#define LIBARGS_ARGS_HPP

#include <cstddef>
#include <deque>
#include <map>
#include <string>
#include <vector>

namespace args {

// エラー種別。errorName() で機械可読な名前（unknown_option 等）に変換できる
enum class ErrorKind {
  None,            // エラーなし
  UnknownOption,   // 未定義のオプション
  MissingValue,    // 値付きオプションの値がない
  UnexpectedValue, // フラグに --flag=値 で値を指定した
  MissingRequired, // 必須オプションがない
  InvalidValue,    // 型変換失敗（Result の取得関数の失敗を呼び出し側が報告する）
  UnknownCommand,  // 未知のサブコマンド
  MissingCommand,  // サブコマンドが指定されていない
};

// エラー種別の名前を返す（None は "none"）
const char *errorName(ErrorKind kind);

// 文字列からの型変換。先頭の空白・末尾の余り・範囲外は失敗（out は変更しない）
bool toInt(const std::string &text, int &out);
bool toDouble(const std::string &text, double &out);
// true/1/yes/on → true、false/0/no/off → false。それ以外は失敗
bool toBool(const std::string &text, bool &out);

// オプション1件の定義
struct OptionDef {
  std::string longName;     // 長い名前（-- を除く。例: "id"）
  char shortName = '\0';    // 短い名前（- を除く1文字。'\0' はなし）
  bool takesValue = true;   // false ならフラグ
  bool required = false;    // 必須か
  bool hasDefault = false;  // 既定値を持つか
  std::string defaultValue; // 既定値（文字列で保持し、取得時に型変換する）
  std::string help;         // 使い方に出す説明
};

// コマンド（サブコマンドまたは単一コマンド）とそのオプション定義
class Command {
public:
  Command(const std::string &name, const std::string &description);

  // 任意の値付きオプションを追加する
  Command &value(const std::string &longName, char shortName,
                 const std::string &help);
  // 必須の値付きオプションを追加する
  Command &required(const std::string &longName, char shortName,
                    const std::string &help);
  // 既定値付きの値付きオプションを追加する
  Command &withDefault(const std::string &longName, char shortName,
                       const std::string &defaultValue,
                       const std::string &help);
  // フラグ（値なし）を追加する
  Command &flag(const std::string &longName, char shortName,
                const std::string &help);

  const std::string &name() const { return name_; }
  const std::string &description() const { return description_; }
  const std::vector<OptionDef> &options() const { return options_; }

  // 名前で定義を探す。見つからなければ nullptr
  const OptionDef *findLong(const std::string &longName) const;
  const OptionDef *findShort(char shortName) const;

private:
  Command &add(const OptionDef &def);

  std::string name_;
  std::string description_;
  std::vector<OptionDef> options_;
};

// 解析結果
class Result {
public:
  // エラーがなければ true（--help 指定時も true）
  bool ok() const { return ErrorKind::None == errorKind_; }
  bool helpRequested() const { return helpRequested_; }
  ErrorKind errorKind() const { return errorKind_; }
  const std::string &error() const { return error_; }       // メッセージ
  const std::string &command() const { return command_; }   // サブコマンド名
  const std::string &helpText() const { return helpText_; } // 使い方
  const std::vector<std::string> &positionals() const { return positionals_; }

  // 指定された（または既定値を持つ）か
  bool has(const std::string &longName) const;
  // フラグが指定されたか（未指定・未定義は false）
  bool flag(const std::string &longName) const;

  // 値を取得する。未指定（既定値もなし）または型変換失敗なら false
  bool getString(const std::string &longName, std::string &out) const;
  bool getInt(const std::string &longName, int &out) const;
  bool getDouble(const std::string &longName, double &out) const;
  bool getBool(const std::string &longName, bool &out) const;

private:
  friend class Parser;
  // 最初のエラーだけを保持する
  void fail(ErrorKind kind, const std::string &message);

  ErrorKind errorKind_ = ErrorKind::None;
  bool helpRequested_ = false;
  std::string error_;
  std::string command_;
  std::string helpText_;
  std::map<std::string, std::string> values_; // longName → 値（フラグは "true"）
  std::vector<std::string> positionals_;
};

// 解析器。サブコマンドを登録しなければ root() を単一コマンドとして使う
class Parser {
public:
  Parser(const std::string &program, const std::string &description);

  // 単一コマンドとして使うときのオプション定義先
  Command &root() { return root_; }
  // サブコマンドを追加する（返した参照は Parser の寿命の間有効）
  Command &addCommand(const std::string &name, const std::string &description);

  // argv[0] を除いて解析する
  Result parse(int argc, const char *const *argv) const;
  // トークン列（プログラム名を含まない）を解析する
  Result parse(const std::vector<std::string> &tokens) const;

  // 全体の使い方（サブコマンド一覧。サブコマンドなしなら root の使い方）
  std::string help() const;
  // コマンド1件の使い方
  std::string help(const Command &cmd) const;

private:
  const Command *findCommand(const std::string &name) const;
  void parseOptions(const Command &cmd, const std::vector<std::string> &tokens,
                    std::size_t start, Result &result) const;
  static bool consumeValue(const OptionDef &def, const std::string &label,
                           const std::vector<std::string> &tokens,
                           std::size_t &index, Result &result);

  std::string program_;
  Command root_;
  std::deque<Command> commands_; // push_back で既存要素の参照が無効にならない
};

} // namespace args

#endif // LIBARGS_ARGS_HPP
