#pragma once

#include <borealis.hpp>
#include <vector>
#include <string>

#include "api/game.hpp"
#include "api/mod.hpp"

// Simple mod list page for a selected game, with footer hints
class ModListTab : public brls::AppletFrame {
public:
    ModListTab(const Game& game);
    ~ModListTab();

    void willAppear(bool resetState = false) override;

private:
    Game game;
    std::vector<Mod> mods;
    ModList* modList = nullptr;
    brls::List* list = nullptr;
    std::vector<brls::Event<brls::View*>::Subscription> clickSubscriptions;
    brls::View* firstFocusableItem = nullptr;

    void refresh();
    void nextPage();
    void prevPage();
    void startSearch();
};
