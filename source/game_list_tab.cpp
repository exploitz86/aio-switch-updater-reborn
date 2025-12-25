#include "game_list_tab.hpp"
#include "mod_list_tab.hpp"
#include "smd_utils/utils.hpp"
#include "constants.hpp"
#include <switch.h>

namespace i18n = brls::i18n;
using namespace i18n::literals;

GameListTab::GameListTab() : brls::AppletFrame(true, true) {
    this->setTitle("menus/mods/browse_mods_by_game"_i18n);
    
    // Content list inside the frame
    this->list = new brls::List();
    this->setContentView(this->list);
    
    // Get installed games
    games = smd_utils::getInstalledGames();
    
    if (games.empty()) {
        brls::ListItem* emptyItem = new brls::ListItem("menus/mods/no_games_found"_i18n);
        emptyItem->setHeight(LISTITEM_HEIGHT);
        this->list->addView(emptyItem);
        return;
    }

    // Create list items for each game with thumbnails
    for (const auto& game : games) {
        u64 tid_u64 = std::stoull(game.second, nullptr, 16);
        
        // Get control data for icon
        NsApplicationControlData* controlData = (NsApplicationControlData*)malloc(sizeof(NsApplicationControlData));
        size_t controlSize = 0;
        
        brls::ListItem* gameItem = new brls::ListItem(game.first, "", game.second);
        
        if (R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, tid_u64, controlData, sizeof(NsApplicationControlData), &controlSize))) {
            gameItem->setThumbnail(controlData->icon, sizeof(controlData->icon));
        }
        free(controlData);
        
        // Capture game by value for the lambda
        std::string title = game.first;
        std::string tid = game.second;
        
        gameItem->getClickEvent()->subscribe([title, tid](brls::View* view) {
            Game gameObj(title, tid);
            
            // Check if game has mods on GameBanana
            if (gameObj.getGamebananaID() <= 0) {
                brls::Application::notify("No mods found for this game on GameBanana");
                return true;
            }
            
            // Open mod list for this game
            brls::Application::pushView(new ModListTab(gameObj));
            return true;
        });

        this->list->addView(gameItem);
    }

    // Store first focusable item for focus restoration
    // Skip the header (index 0) and get the first game item (index 1)
    if (this->list->getViewsCount() > 1) {
        this->firstFocusableItem = this->list->getChild(1);
    }
}

void GameListTab::willAppear(bool resetState) {
    brls::AppletFrame::willAppear(resetState);

    // Restore focus to first item when returning from mod list
    if (!resetState && this->firstFocusableItem) {
        brls::Application::giveFocus(this->firstFocusableItem);
    }
}
