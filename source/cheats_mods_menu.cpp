#include "cheats_mods_menu.hpp"
#include "constants.hpp"
#include "list_download_tab.hpp"
#include "mods_tab.hpp"
#include "mods_frame.hpp"
#include "cheats_page.hpp"
#include "cheat_settings_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

CheatsModsMenu::CheatsModsMenu() : brls::List() {
    // Cheats option - opens in TabFrame with sidebar
    this->addView(new brls::Label(brls::LabelStyle::DESCRIPTION,"menus/cheats/cheat_menu_desc"_i18n,true));
    auto* cheatsItem = new brls::ListItem("menus/cheats/menu"_i18n);
    cheatsItem->setHeight(LISTITEM_HEIGHT);
    cheatsItem->getClickEvent()->subscribe([](brls::View* view) {
        // Create TabFrame for sidebar navigation
        auto* tabFrame = new brls::TabFrame();
        tabFrame->setTitle("menus/main/download_cheats"_i18n);
        tabFrame->addTab("menus/cheats/menu"_i18n, new ListDownloadTab(contentType::cheats));
        tabFrame->addTab("menus/main/tools"_i18n, new CheatsPage());
        tabFrame->addTab("menus/cheats/settings"_i18n, new CheatSettingsPage());
        // Reduce sidebar width
        tabFrame->sidebar->setWidth(310);
        brls::Application::pushView(tabFrame);
        return true;
    });
    this->addView(cheatsItem);

    // Mods option - open in AppletFrame with title and back button
    this->addView(new brls::Label(brls::LabelStyle::DESCRIPTION,"menus/mods/mod_menu_desc"_i18n,true));
    auto* modsItem = new brls::ListItem("menus/mods/menu"_i18n);
    modsItem->setHeight(LISTITEM_HEIGHT);
    modsItem->getClickEvent()->subscribe([](brls::View* view) {
        auto* modsFrame = new ModsFrame();
        brls::Application::pushView(modsFrame);
        return true;
    });
    this->addView(modsItem);
}
