#pragma once

#include <borealis.hpp>
#include <string>
#include <vector>

#include "api/game.hpp"
#include "api/mod.hpp"

// Detailed view for a single mod: shows metadata, files, and allows download
class ModDetailPage : public brls::AppletFrame {
public:
    ModDetailPage(const Game& game, const Mod& mod);
    ~ModDetailPage();

    void willAppear(bool resetState = false) override;

private:
    Game game;
    Mod mod;

    // Scrollable container with vertical box layout for clean visuals
    brls::ScrollView* scroll = nullptr;
    brls::BoxLayout* container = nullptr;
    brls::View* firstFocusableItem = nullptr;

    void buildUI();
    void addFileItem(const File& file);
};
