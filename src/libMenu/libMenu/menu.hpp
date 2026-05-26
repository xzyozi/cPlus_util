#ifndef CUI_MENU_HPP
#define CUI_MENU_HPP

#include <string>
#include <vector>
#include <chrono>
#include <iostream>
#include <sstream>
#include <functional>

#include "menuItem.hpp"

namespace cui {

// ====== ログ定義 ======
struct InputLogEntry {
    std::chrono::system_clock::time_point timestamp;
    std::string menu_path;   // "Main > Search" など
    std::string raw_input;   // ユーザ生入力
    int choice_index;        // 選択番号（-1: 無効）
    std::string status;      // "ok", "invalid", "error" など
};

class InputLog {
public:
    void record(const InputLogEntry& e) { entries_.push_back(e); }
    const std::vector<InputLogEntry>& entries() const { return entries_; }
    static std::string fmt_time(std::chrono::system_clock::time_point tp);
    void display(std::ostream& out) const;
private:
    std::vector<InputLogEntry> entries_;
};

// ====== メニュー ======
class Menu {
public:
    explicit Menu(std::string title);
    void add(const MenuItem& item);
    const std::string& title() const { return title_; }
    const std::vector<MenuItem>& items() const { return items_; }
    std::vector<MenuItem>& items() { return items_; }
private:
    std::string title_;
    std::vector<MenuItem> items_;
};

// ====== ナビゲータ（遷移・表示・入力） ======
class Navigator {
public:
    explicit Navigator(std::ostream& out = std::cout, std::istream& in = std::cin);
    void run(const Menu& root);
    const std::vector<InputLogEntry>& logs() const { return log_.entries(); }
    void show_history() const;
private:
    // log 関連
    std::ostream& out_;
    std::istream& in_;
    std::vector<const Menu*> stack_;
    InputLog log_;
    std::string last_raw_input_;

    void render() const;                    // メニュー描画
    std::string breadcrumb() const;         // パンくず
    bool read_choice(int& out);             // 入力読み取り（optional不使用）
    InputLogEntry make_log(const Menu& m, const std::string& raw, int choice, const std::string& status) const;
};

} // namespace cui

#endif // CUI_MENU_HPP