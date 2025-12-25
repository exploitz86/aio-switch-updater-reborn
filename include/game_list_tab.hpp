#pragma once

#include <borealis.hpp>
#include <vector>
#include <string>

#include "api/game.hpp"

// Simple game list view showing installed games that have mods on GameBanana
class GameListTab : public brls::AppletFrame {
public:
    GameListTab();

    void willAppear(bool resetState = false) override;

    brls::List* getList() { return list; }

private:
    std::vector<std::pair<std::string, std::string>> games; // title, tid pairs
    brls::List* list = nullptr;
    brls::View* firstFocusableItem = nullptr;
};
