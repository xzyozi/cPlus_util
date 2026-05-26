#ifndef CUI_MENUITEM_HPP
#define CUI_MENUITEM_HPP

#include <string>
#include <functional>

namespace cui {

class Menu; // 前方宣言

enum class MenuItemKind { Action, Submenu, Back, Exit };

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
};

} // namespace cui
#endif // CUI_MENUITEM_H