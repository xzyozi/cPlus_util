include "menu.hpp"
#include <iomanip>

namespace cui {

// ===== InputLog =====
std::string InputLog::fmt_time(std::chrono::system_clock::time_point tp) {
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm tm{};
    #ifdef _WIN32
        localtime_s(&tm, &tt);
    #else
        localtime_r(&tt, &tm);
    #endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

// 履歴を出力ストリームに表示する
void InputLog::display(std::ostream& out) const {
    out << "\n[History] Records: " << entries_.size() << "\n";
    out << "-------------------------------------------------------------\n";
    // メンバ変数 entries_ が std::vector<InputLogEntry> であると仮定しています
    for (const auto& entry : entries_) {
        out << fmt_time(entry.timestamp) << " | "
            << "menu_path: " << std::left << std::setw(15) << entry.menu_path.substr(0, 15) << " | "
            << "In: " << std::left << std::setw(4) << entry.raw_input << " | "
            << "St: " << entry.status << "\n";
    }
    out << "-------------------------------------------------------------\n";
}


// ===== Menu =====
Menu::Menu(std::string title) : title_(std::move(title)) {}
void Menu::add(const MenuItem& item) { items_.push_back(item); }

// ===== Navigator =====
Navigator::Navigator(std::ostream& out, std::istream& in) : out_(out), in_(in) {}

void Navigator::run(const Menu& root) {
    stack_.clear();
    stack_.push_back(&root);
    bool running = true;

    while (running && !stack_.empty()) {
        render();
        int choice = -999;
        if (!read_choice(choice)) {
            out_ << "\n[!] 入力が得られませんでした（EOF）。終了します。\n";
            break;
        }
        int n = choice; // 0 は Back/Exit, 1..N は項目
        const Menu& cur = *stack_.back();

        // 0: Back/Exit
        if (n == 0) {
            if (stack_.size() == 1) { running = false; out_ << "Bye.\n"; }
            else { stack_.pop_back(); }
            log_.record(make_log(cur, last_raw_input_, 0, "ok"));
            continue;
        }

        // 可視項目数（Back/Exitを除く）
        int visibleCount = 0;
        for (const auto& it : cur.items()) {
            if (it.kind != MenuItemKind::Back && it.kind != MenuItemKind::Exit) ++visibleCount;
        }
        if (n < 0 || n > visibleCount) {
            log_.record(make_log(cur, last_raw_input_, -1, "invalid"));
            out_ << "[!] 番号が不正です。0〜" << visibleCount << " で選択してください。\n\n";
            continue;
        }

        // n番目の可視項目を取得
        int idx = -1;
        for (size_t i = 0; i < cur.items().size(); ++i) {
            if (cur.items()[i].kind == MenuItemKind::Back || cur.items()[i].kind == MenuItemKind::Exit) continue;
            ++idx;
            if (idx == n - 1) {
                const MenuItem& item = cur.items()[i];
                log_.record(make_log(cur, last_raw_input_, n, "ok"));

                switch (item.kind) {
                    case MenuItemKind::Action:
                        try { if (item.action) (void)item.action(); }
                        catch (const std::exception& e) {
                            out_ << "[!] 例外: " << e.what() << "\n";
                            log_.record(make_log(cur, "action-exception", n, "error"));
                        }
                        break; // 実行後は同じメニューへ戻る
                    case MenuItemKind::Submenu:
                        if (item.child) stack_.push_back(item.child);
                        else out_ << "[!] 子メニューが設定されていません。\n";
                        break;
                    case MenuItemKind::Back:
                    case MenuItemKind::Exit:
                        // ここには来ない（0で処理）
                        break;
                }
                break;
            }
        }
    }
}

void Navigator::render() const {
    const Menu& m = *stack_.back();
    out_ << "\n    === " << breadcrumb() << " ===\n";
    int visibleIndex = 0;
    for (const auto& it : m.items()) {
        if (it.kind == MenuItemKind::Back || it.kind == MenuItemKind::Exit) continue; // 0 枠で表示するのでスキップ
        ++visibleIndex;
        out_ << "    " << visibleIndex << ") " << it.label << "\n";
    }
    out_ << "    " << "0) " << (stack_.size() == 1 ? "Exit" : "Back") << "\n";
    out_ << "  番号を入力してください: ";
}

std::string Navigator::breadcrumb() const {
    std::ostringstream oss;
    for (size_t i = 0; i < stack_.size(); ++i) {
        if (i) oss << " > ";
        oss << stack_[i]->title();
    }
    return oss.str();
}

bool Navigator::read_choice(int& out) {
    std::string line;
    if (!std::getline(in_, line)) return false; // EOF
    last_raw_input_ = line;
    std::istringstream iss(line);
    int n;
    if (iss >> n) { out = n; return true; }
    // 先頭が数字でない場合は不正入力として -999 を返す（呼び出し側で弾く）
    out = -999;
    return true;
}

InputLogEntry Navigator::make_log(const Menu& m, const std::string& raw, int choice, const std::string& status) const {
    return InputLogEntry{std::chrono::system_clock::now(), breadcrumb(), raw, choice, status};
}

// InputLogの表示機能を呼び出すラッパー
void Navigator::show_history() const {
    log_.display(out_);
}

} // namespace cui
