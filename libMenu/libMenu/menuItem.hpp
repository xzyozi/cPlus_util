#ifndef CUI_MENUITEM_HPP
#define CUI_MENUITEM_HPP

#include <string>
#include <functional>

namespace cui {

class Menu; // 前方宣言

enum class MenuItemKind { Action, Submenu, Back, Exit };

/// メニュー項目。ファクトリメソッド経由でのみ生成すること。
/// child は非所有ポインタ。参照先の Menu は MenuItem より長く生存させる必要がある。
class MenuItem {
public:
    std::string label;
    MenuItemKind kind;
    std::function<int()> action;
    const Menu* child = nullptr;

    static MenuItem actionItem(const std::string& label, std::function<int()> fn);
    static MenuItem submenuItem(const std::string& label, const Menu& child);
    static MenuItem backItem(const std::string& label = "Back");
    static MenuItem exitItem(const std::string& label = "Exit");

private:
    MenuItem() : kind(MenuItemKind::Action), child(nullptr) {}
    // ファクトリメソッド (static メンバ) からのみ構築可能。
    // コピー/ムーブは暗黙生成のまま public（vector 操作に必要）。
};

} // namespace cui
#endif // CUI_MENUITEM_HPP
