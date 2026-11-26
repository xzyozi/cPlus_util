#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <iomanip>

#include "libMenu/menu.hpp"

using namespace cui;

// ====== グローバル：Navigator（ログ参照で使用） ======
Navigator g_nav;

// ====== 共有状態（設定） ======
struct AppSettings {
    bool darkTheme;
    std::string language;
    AppSettings() : darkTheme(false), language("ja") {}
} g_settings;

// ====== ユーティリティ ======
static void print_settings() {
    std::cout << "=== Current Settings ===\n";
    std::cout << "Theme      : " << (g_settings.darkTheme ? "Dark" : "Light") << "\n";
    std::cout << "Language   : " << g_settings.language << "\n";
    std::cout << "========================\n";
}

// ====== 参考の処理関数（検索系） ======
int sample_function() {
    std::cout << "search branch_id >> ";
    std::string line;
    std::getline(std::cin, line);
    std::cout << "[Search] You entered branch_id: " << line << "\n";
    return 0;
}

int search_by_date() {
    std::cout << "search date (YYYY-MM-DD) >> ";
    std::string line;
    std::getline(std::cin, line);
    if (line.size() == 10 && line[4] == '-' && line[7] == '-') {
        std::cout << "[Search] Date: " << line << " accepted.\n";
    } else {
        std::cout << "[Search] Invalid date format.\n";
    }
    return 0;
}

// ====== 疑似 長時間処理（進捗表示のみ） ======
int run_long_task() {
    std::cout << "Running long task...\n";
    for (int i = 0; i <= 10; ++i) {
        std::cout << "  progress: " << (i * 10) << "%\r" << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "\n[Task] Completed.\n";
    return 0;
}

// ====== 設定操作 ======
int toggle_theme() {
    g_settings.darkTheme = !g_settings.darkTheme;
    std::cout << "[Settings] Theme toggled to " << (g_settings.darkTheme ? "Dark" : "Light") << ".\n";
    print_settings();
    return 0;
}

int change_language() {
    std::cout << "Language code (ja/en/etc.) >> ";
    std::string line;
    std::getline(std::cin, line);
    if (!line.empty()) {
        g_settings.language = line;
        std::cout << "[Settings] Language set to " << g_settings.language << ".\n";
    } else {
        std::cout << "[Settings] Language unchanged.\n";
    }
    print_settings();
    return 0;
}

int show_current_settings() {
    print_settings();
    return 0;
}

// ====== Branches 固定アクション ======
int open_branch_Tokyo()   { std::cout << "[Branches] Selected branch: Tokyo\n";   return 0; }
int open_branch_Osaka()   { std::cout << "[Branches] Selected branch: Osaka\n";   return 0; }
int open_branch_Nagoya()  { std::cout << "[Branches] Selected branch: Nagoya\n";  return 0; }
int open_branch_Fukuoka() { std::cout << "[Branches] Selected branch: Fukuoka\n"; return 0; }

// ====== 動的サブメニュー生成 ======
void build_branches_menu(Menu& branches) {
    branches.add(MenuItem::actionItem("Open Tokyo",   &open_branch_Tokyo));
    branches.add(MenuItem::actionItem("Open Osaka",   &open_branch_Osaka));
    branches.add(MenuItem::actionItem("Open Nagoya",  &open_branch_Nagoya));
    branches.add(MenuItem::actionItem("Open Fukuoka", &open_branch_Fukuoka));
}

// ====== ログ表示（グローバル g_nav を参照） ======
int show_logs() {
    const std::vector<InputLogEntry>& logs = g_nav.logs();
    if (logs.empty()) {
        std::cout << "[Logs] No entries.\n";
        return 0;
    }
    std::cout << "=== Operation Logs ===\n";
    for (size_t i = 0; i < logs.size(); ++i) {
        const InputLogEntry& e = logs[i];
        std::cout << std::setw(3) << (i + 1) << " : "
                  << InputLog::fmt_time(e.timestamp) << " | "
                  << e.menu_path << " | "
                  << "raw=\"" << e.raw_input << "\" | "
                  << "choice=" << e.choice_index << " | "
                  << "status=" << e.status << "\n";
    }
    std::cout << "======================\n";
    return 0;
}

// ====== ヘルプ表示 ======
int show_help() {
    std::cout << "=== Help ===\n";
    std::cout << "1..N - メニュー項目を選択\n";
    std::cout << "0    - Back（サブメニュー）/ Exit（ルート）\n";
    std::cout << "数字以外の入力は不正として扱われ、メニューが再表示されます。\n";
    std::cout << "Action の実行後は自動的に同じメニューへ戻ります。\n";
    std::cout << "============\n";
    return 0;
}

// ====== About ======
int show_about() {
    std::cout << "CUI Menu System - Display-focused navigator\n";
    return 0;
}

int main() {
    // --- メニュー定義 ---
    Menu root("Main");
    Menu search("Search");
    Menu settings("Settings");
    Menu tools("Tools");
    Menu branches("Branches");

    // --- メインメニュー ---
    root.add(MenuItem::submenuItem("Search Menu",   search));
    root.add(MenuItem::submenuItem("Settings Menu", settings));
    root.add(MenuItem::submenuItem("Tools Menu",    tools));
    root.add(MenuItem::actionItem("About", &show_about));
    root.add(MenuItem::actionItem("Help",  &show_help));

    // --- Sub メニュー ---
    search.add(MenuItem::actionItem("Search by branch_id", &sample_function));
    search.add(MenuItem::actionItem("Search by date",      &search_by_date));

    // Branches
    build_branches_menu(branches);
    search.add(MenuItem::submenuItem("Branches", branches));

    // --- Settings メニュー ---
    settings.add(MenuItem::actionItem("Toggle Theme",          &toggle_theme));
    settings.add(MenuItem::actionItem("Change Language",       &change_language));
    settings.add(MenuItem::actionItem("Show Current Settings", &show_current_settings));

    // --- Tools メニュー ---
    tools.add(MenuItem::actionItem("Run long task", &run_long_task));
    tools.add(MenuItem::actionItem("Show logs",     &show_logs));

    // --- 実行 ---
    g_nav.run(root);
    return 0;
}
