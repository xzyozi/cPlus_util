/*
 * libArgs: コマンドライン引数解析ライブラリの実装
 * 仕様は args.hpp と README.md を参照。
 */
#include "args.hpp"

#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <sstream>

namespace args {

namespace {

const char *const HELP_LONG = "--help";
const char *const HELP_SHORT = "-h";
const char *const SEPARATOR = "--";

bool startsWithSpace(const std::string &text) {
  return !text.empty() &&
         0 != std::isspace(static_cast<unsigned char>(text[0]));
}

// 値付きオプションの次トークンを値として使えるか。
// 空文字は値とみなさない。"-" で始まる2文字以上のトークンは、
// 負の数（-5, -.5）だけを値とみなす（-h / --help は値にならない）。
bool looksLikeValue(const std::string &token) {
  if (token.empty()) {
    return false;
  }
  if (2 > token.size() || '-' != token[0]) {
    return true;
  }
  const unsigned char second = static_cast<unsigned char>(token[1]);
  return 0 != std::isdigit(second) || '.' == token[1];
}

// -- より前に --help / -h があるか
bool hasHelpBeforeSeparator(const std::vector<std::string> &tokens) {
  for (const std::string &token : tokens) {
    if (SEPARATOR == token) {
      return false;
    }
    if (HELP_LONG == token || HELP_SHORT == token) {
      return true;
    }
  }
  return false;
}

std::string padRight(const std::string &text, std::size_t width) {
  if (width <= text.size()) {
    return text;
  }
  return text + std::string(width - text.size(), ' ');
}

// 使い方の左列（例: "-c, --count <VALUE>"、短い名前なしは空白で揃える）
std::string optionLabel(const OptionDef &def) {
  std::string label = ('\0' != def.shortName)
                          ? std::string("-") + def.shortName + ", "
                          : std::string("    ");
  label += "--" + def.longName;
  if (def.takesValue) {
    label += " <VALUE>";
  }
  return label;
}

} // namespace

const char *errorName(ErrorKind kind) {
  switch (kind) {
  case ErrorKind::None:
    return "none";
  case ErrorKind::UnknownOption:
    return "unknown_option";
  case ErrorKind::MissingValue:
    return "missing_value";
  case ErrorKind::UnexpectedValue:
    return "unexpected_value";
  case ErrorKind::MissingRequired:
    return "missing_required";
  case ErrorKind::InvalidValue:
    return "invalid_value";
  case ErrorKind::UnknownCommand:
    return "unknown_command";
  case ErrorKind::MissingCommand:
    return "missing_command";
  }
  return "unknown";
}

bool toInt(const std::string &text, int &out) {
  if (text.empty() || startsWithSpace(text)) {
    return false;
  }
  errno = 0;
  char *end = nullptr;
  const long parsed = std::strtol(text.c_str(), &end, 10);
  if (0 != errno || nullptr == end || '\0' != *end) {
    return false;
  }
  if (INT_MIN > parsed || INT_MAX < parsed) {
    return false;
  }
  out = static_cast<int>(parsed);
  return true;
}

bool toDouble(const std::string &text, double &out) {
  if (text.empty() || startsWithSpace(text)) {
    return false;
  }
  errno = 0;
  char *end = nullptr;
  const double parsed = std::strtod(text.c_str(), &end);
  if (0 != errno || nullptr == end || '\0' != *end || !std::isfinite(parsed)) {
    return false;
  }
  out = parsed;
  return true;
}

bool toBool(const std::string &text, bool &out) {
  if ("true" == text || "1" == text || "yes" == text || "on" == text) {
    out = true;
    return true;
  }
  if ("false" == text || "0" == text || "no" == text || "off" == text) {
    out = false;
    return true;
  }
  return false;
}

// ---------------------------------------------------------------- Command

Command::Command(const std::string &name, const std::string &description)
    : name_(name), description_(description) {}

Command &Command::add(const OptionDef &def) {
  // 定義の誤りはプログラムの誤りなので assert で止める（NDEBUG 時は検出しない）
  assert("help" != def.longName && "--help は予約名です");
  assert('h' != def.shortName && "-h は予約名です");
  assert(nullptr == findLong(def.longName) && "long 名が重複しています");
  assert(nullptr == findShort(def.shortName) && "short 名が重複しています");
  options_.push_back(def);
  return *this;
}

Command &Command::value(const std::string &longName, char shortName,
                        const std::string &help) {
  OptionDef def;
  def.longName = longName;
  def.shortName = shortName;
  def.help = help;
  return add(def);
}

Command &Command::required(const std::string &longName, char shortName,
                           const std::string &help) {
  OptionDef def;
  def.longName = longName;
  def.shortName = shortName;
  def.required = true;
  def.help = help;
  return add(def);
}

Command &Command::withDefault(const std::string &longName, char shortName,
                              const std::string &defaultValue,
                              const std::string &help) {
  OptionDef def;
  def.longName = longName;
  def.shortName = shortName;
  def.hasDefault = true;
  def.defaultValue = defaultValue;
  def.help = help;
  return add(def);
}

Command &Command::flag(const std::string &longName, char shortName,
                       const std::string &help) {
  OptionDef def;
  def.longName = longName;
  def.shortName = shortName;
  def.takesValue = false;
  def.help = help;
  return add(def);
}

const OptionDef *Command::findLong(const std::string &longName) const {
  for (const OptionDef &def : options_) {
    if (longName == def.longName) {
      return &def;
    }
  }
  return nullptr;
}

const OptionDef *Command::findShort(char shortName) const {
  if ('\0' == shortName) {
    return nullptr;
  }
  for (const OptionDef &def : options_) {
    if (shortName == def.shortName) {
      return &def;
    }
  }
  return nullptr;
}

// ----------------------------------------------------------------- Result

void Result::fail(ErrorKind kind, const std::string &message) {
  if (ErrorKind::None != errorKind_) {
    return;
  }
  errorKind_ = kind;
  error_ = message;
}

void Result::failInvalidValue(const std::string &longName,
                              const std::string &raw) {
  fail(ErrorKind::InvalidValue,
       "オプションの値を変換できません: --" + longName + "=" + raw);
}

bool Result::has(const std::string &longName) const {
  return values_.end() != values_.find(longName);
}

bool Result::flag(const std::string &longName) const {
  bool on = false;
  return getBool(longName, on) && on;
}

bool Result::getString(const std::string &longName, std::string &out) const {
  const auto it = values_.find(longName);
  if (values_.end() == it) {
    return false;
  }
  out = it->second;
  return true;
}

bool Result::getInt(const std::string &longName, int &out) const {
  std::string text;
  return getString(longName, text) && toInt(text, out);
}

bool Result::getDouble(const std::string &longName, double &out) const {
  std::string text;
  return getString(longName, text) && toDouble(text, out);
}

bool Result::getBool(const std::string &longName, bool &out) const {
  std::string text;
  return getString(longName, text) && toBool(text, out);
}

// ----------------------------------------------------------------- Parser

Parser::Parser(const std::string &program, const std::string &description)
    : program_(program), root_(std::string(), description) {}

Command &Parser::addCommand(const std::string &name,
                            const std::string &description) {
  commands_.emplace_back(name, description);
  return commands_.back();
}

const Command *Parser::findCommand(const std::string &name) const {
  for (const Command &cmd : commands_) {
    if (name == cmd.name()) {
      return &cmd;
    }
  }
  return nullptr;
}

Result Parser::parse(int argc, const char *const *argv) const {
  std::vector<std::string> tokens;
  if (nullptr != argv) {
    for (int i = 1; i < argc; ++i) {
      tokens.emplace_back(nullptr != argv[i] ? argv[i] : "");
    }
  }
  return parse(tokens);
}

Result Parser::parse(const std::vector<std::string> &tokens) const {
  Result result;
  // --help / -h は -- より前のどこにあってもよく、解析エラーより優先する
  const bool helpWanted = hasHelpBeforeSeparator(tokens);
  if (commands_.empty()) {
    if (helpWanted) {
      result.helpRequested_ = true;
      result.helpText_ = help(root_);
      return result;
    }
    parseOptions(root_, tokens, 0, result);
    return result;
  }

  if (helpWanted) {
    // 先頭が登録済みサブコマンドならその使い方、それ以外は全体の使い方
    const Command *helpCmd = findCommand(tokens[0]);
    result.helpRequested_ = true;
    if (nullptr == helpCmd) {
      result.helpText_ = help();
    } else {
      result.command_ = helpCmd->name();
      result.helpText_ = help(*helpCmd);
    }
    return result;
  }
  if (tokens.empty()) {
    result.fail(ErrorKind::MissingCommand, "サブコマンドを指定してください（" +
                                               program_ +
                                               " --help で一覧を表示）");
    return result;
  }
  const std::string &first = tokens[0];
  if (!first.empty() && '-' == first[0]) {
    result.fail(ErrorKind::MissingCommand,
                "オプションの前にサブコマンドを指定してください: " + first);
    return result;
  }
  const Command *cmd = findCommand(first);
  if (nullptr == cmd) {
    result.fail(ErrorKind::UnknownCommand, "未知のサブコマンドです: " + first);
    return result;
  }
  result.command_ = cmd->name();
  parseOptions(*cmd, tokens, 1, result);
  return result;
}

bool Parser::consumeValue(const OptionDef &def, const std::string &label,
                          const std::vector<std::string> &tokens,
                          std::size_t &index, Result &result) {
  if (!def.takesValue) {
    result.values_[def.longName] = "true";
    return true;
  }
  const std::size_t next = index + 1;
  if (tokens.size() <= next || !looksLikeValue(tokens[next])) {
    result.fail(ErrorKind::MissingValue,
                "オプションの値がありません: " + label);
    return false;
  }
  result.values_[def.longName] = tokens[next];
  index = next;
  return true;
}

void Parser::parseOptions(const Command &cmd,
                          const std::vector<std::string> &tokens,
                          std::size_t start, Result &result) const {
  // 既定値を先に入れ、指定があれば上書きする
  for (const OptionDef &def : cmd.options()) {
    if (def.hasDefault) {
      result.values_[def.longName] = def.defaultValue;
    }
  }

  bool afterSeparator = false;
  for (std::size_t i = start; i < tokens.size(); ++i) {
    const std::string &token = tokens[i];
    if (afterSeparator) {
      result.positionals_.push_back(token);
      continue;
    }
    if (SEPARATOR == token) {
      afterSeparator = true;
      continue;
    }
    // --help / -h は parse() で先に処理済みのため、ここには来ない

    // 長いオプション（--name / --name=value）
    if (2 < token.size() && 0 == token.compare(0, 2, SEPARATOR)) {
      const std::size_t eq = token.find('=');
      const std::string name =
          (std::string::npos == eq) ? token.substr(2) : token.substr(2, eq - 2);
      const OptionDef *def = cmd.findLong(name);
      if (nullptr == def) {
        result.fail(ErrorKind::UnknownOption,
                    "未定義のオプションです: --" + name);
        return;
      }
      if (std::string::npos != eq) {
        if (!def->takesValue) {
          result.fail(ErrorKind::UnexpectedValue,
                      "フラグに値は指定できません: --" + name);
          return;
        }
        const std::string text = token.substr(eq + 1);
        if (text.empty()) {
          result.fail(ErrorKind::MissingValue,
                      "オプションの値がありません: --" + name);
          return;
        }
        result.values_[def->longName] = text;
        continue;
      }
      if (!consumeValue(*def, "--" + name, tokens, i, result)) {
        return;
      }
      continue;
    }

    // 短いオプション（-n。まとめ書き -abc や -n=value は扱わない）
    if (2 == token.size() && '-' == token[0]) {
      const OptionDef *def = cmd.findShort(token[1]);
      if (nullptr == def) {
        result.fail(ErrorKind::UnknownOption,
                    "未定義のオプションです: " + token);
        return;
      }
      if (!consumeValue(*def, token, tokens, i, result)) {
        return;
      }
      continue;
    }
    if (2 < token.size() && '-' == token[0]) {
      result.fail(ErrorKind::UnknownOption, "未定義のオプションです: " + token);
      return;
    }

    // それ以外（"-" 単独を含む）は位置引数
    result.positionals_.push_back(token);
  }

  // 必須チェック（定義順で最初の欠落を報告する）
  for (const OptionDef &def : cmd.options()) {
    if (def.required && !result.has(def.longName)) {
      result.fail(ErrorKind::MissingRequired,
                  "必須オプションがありません: --" + def.longName);
      return;
    }
  }
}

std::string Parser::help() const {
  if (commands_.empty()) {
    return help(root_);
  }
  std::ostringstream os;
  os << "使い方: " << program_ << " <サブコマンド> [オプション]\n";
  if (!root_.description().empty()) {
    os << root_.description() << "\n";
  }
  std::size_t width = 0;
  for (const Command &cmd : commands_) {
    if (width < cmd.name().size()) {
      width = cmd.name().size();
    }
  }
  os << "\nサブコマンド:\n";
  for (const Command &cmd : commands_) {
    os << "  " << padRight(cmd.name(), width) << "  " << cmd.description()
       << "\n";
  }
  os << "\n各サブコマンドの詳細: " << program_ << " <サブコマンド> --help\n";
  return os.str();
}

std::string Parser::help(const Command &cmd) const {
  const std::string usageName =
      cmd.name().empty() ? program_ : program_ + " " + cmd.name();
  std::ostringstream os;
  os << "使い方: " << usageName << " [オプション] [--] [位置引数...]\n";
  if (!cmd.description().empty()) {
    os << cmd.description() << "\n";
  }

  const std::string helpLabel = "-h, --help";
  std::size_t width = helpLabel.size();
  for (const OptionDef &def : cmd.options()) {
    const std::string label = optionLabel(def);
    if (width < label.size()) {
      width = label.size();
    }
  }

  os << "\nオプション:\n";
  for (const OptionDef &def : cmd.options()) {
    os << "  " << padRight(optionLabel(def), width) << "  " << def.help;
    if (def.required) {
      os << "（必須）";
    } else if (def.hasDefault) {
      os << "（既定: " << def.defaultValue << "）";
    }
    os << "\n";
  }
  os << "  " << padRight(helpLabel, width) << "  この使い方を表示する\n";
  return os.str();
}

} // namespace args
