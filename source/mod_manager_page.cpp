#include "mod_manager_page.hpp"
#include <borealis.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <switch.h>

// SimpleModManager integration
#include "FrameModBrowser.h"
#include "GuiModManager.h"
#include "GameBrowser.h"
#include "GenericToolbox.Switch.h"

namespace {

std::string modsRoot() { return std::string("sdmc:/mods"); }

std::vector<std::string> listDirectories(const std::string& root) {
    std::vector<std::string> out;
    std::error_code ec;
    if (!std::filesystem::exists(root)) return out;
    for (const auto& entry : std::filesystem::directory_iterator(root, ec)) {
        if (entry.is_directory()) out.emplace_back(entry.path().filename().string());
    }
    std::sort(out.begin(), out.end());
    return out;
}

} // namespace

// Global GuiModManager instance - one per app session
// This manages all game mod operations and persists across game switches
static GuiModManager* g_guiModManager = nullptr;

// Cleanup function registered with atexit
static void cleanupGuiModManager() {
    if (g_guiModManager != nullptr) {
        delete g_guiModManager;
        g_guiModManager = nullptr;
    }
}

// Constructor implementation
ModManagerPage::ModManagerPage() {
    this->addView(new brls::Header("Games"));

    // Initialize global GuiModManager if not already created
    if (g_guiModManager == nullptr) {
        g_guiModManager = new GuiModManager();
        // Register cleanup to run when app exits
        std::atexit(cleanupGuiModManager);
    }

    auto games = listDirectories(modsRoot());
    if (games.empty()) {
        auto* empty = new brls::ListItem("No games found under /mods");
        this->addView(empty);
    } else {
        for (const auto& g : games) {
            // Count how many mods this game has
            std::string gamePath = modsRoot() + "/" + g;
            int nMods = static_cast<int>(listDirectories(gamePath).size());

            // Create ListItem with game name and mod count
            auto* item = new brls::ListItem(g, "", std::to_string(nMods) + " mod(s) available.");

            // Try to get the game icon using GameBrowser's getFolderIcon method
            uint8_t* icon = g_guiModManager->getGameBrowser().getFolderIcon(g);
            if (icon != nullptr) {
                // The icon is raw JPEG data, size is 0x20000 (128KB)
                item->setThumbnail(icon, 0x20000);
                // Free the allocated memory after setThumbnail copies it
                delete[] icon;
            }

            item->getClickEvent()->subscribe([g](brls::View*) {
                // Null check on global instance
                if (g_guiModManager == nullptr) {
                    brls::Logger::error("GuiModManager instance is null");
                    brls::Application::notify("Error: Mod manager not initialized");
                    return false;
                }

                // Select the game in the GameBrowser
                g_guiModManager->getGameBrowser().selectGame(g);

                // Create and push the FrameModBrowser
                auto* modBrowser = new FrameModBrowser(g_guiModManager);
                brls::Application::pushView(modBrowser, brls::ViewAnimation::SLIDE_LEFT);

                // Reset the game selection flag when returning
                g_guiModManager->getGameBrowser().setIsGameSelected(false);

                return true;
            });
            this->addView(item);
        }
    }
}

void ModManagerPage::willAppear(bool resetState) {
    brls::List::willAppear(resetState);
    brls::Logger::debug("ModManagerPage: Appearing (wake from sleep or navigation)");

    // Check if GuiModManager is still valid after sleep/wake
    if (g_guiModManager == nullptr) {
        brls::Logger::warning("ModManagerPage: GuiModManager was destroyed, reinitializing");
        g_guiModManager = new GuiModManager();
        std::atexit(cleanupGuiModManager);
    }
}

void ModManagerPage::willDisappear(bool resetState) {
    brls::List::willDisappear(resetState);
    brls::Logger::debug("ModManagerPage: Disappearing (sleep or navigation)");

    // Note: We don't delete g_guiModManager here because it's shared across the app
    // It will be cleaned up by atexit when the app exits
}
