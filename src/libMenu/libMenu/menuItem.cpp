#include "menuItem.hpp"
namespace cui {

MenuItem MenuItem::actionItem(const std::string& label, std::function<int()> fn) {
    MenuItem it; it.label = label; it.kind = MenuItemKind::Action; it.action = std::move(fn); it.child = nullptr; return it;
}
MenuItem MenuItem::submenuItem(const std::string& label, const Menu& child) {
    MenuItem it; it.label = label; it.kind = MenuItemKind::Submenu; it.child = &child; return it;
}
MenuItem MenuItem::backItem(const std::string& label) {
    MenuItem it; it.label = label; it.kind = MenuItemKind::Back; return it;
}
MenuItem MenuItem::exitItem(const std::string& label) {
    MenuItem it; it.label = label; it.kind = MenuItemKind::Exit; return it;
}

}