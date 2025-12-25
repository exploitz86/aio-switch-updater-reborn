#include "mod_list_tab.hpp"
#include "confirm_page.hpp"
#include "worker_page.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "constants.hpp"
#include "mod_detail_page.hpp"
#include <borealis/swkbd.hpp>

namespace i18n = brls::i18n;
using namespace i18n::literals;

ModListTab::ModListTab(const Game& game) : brls::AppletFrame(true, true), game(game) {
    this->setTitle(fmt::format("{}", game.getTitle()));

    // Content list inside the frame
    this->list = new brls::List();
    this->setContentView(this->list);

    // Always create new ModList for this view
    this->modList = new ModList(game);
    
    this->mods = this->modList->getMods();

    refresh();

    // L < : previous page
    this->registerAction("⇐", brls::Key::L, [this]() {
        this->prevPage();
        return true;
    });
    // R > : next page
    this->registerAction("⇒", brls::Key::R, [this]() {
        this->nextPage();
        return true;
    });
    // Y Search
    this->registerAction(("menus/mods/search"_i18n), brls::Key::Y, [this]() {
        this->startSearch();
        return true;
    });
}

void ModListTab::willAppear(bool resetState) {
    brls::AppletFrame::willAppear(resetState);

    // Restore focus to first item when returning from mod detail page
    if (!resetState && this->firstFocusableItem) {
        brls::Application::giveFocus(this->firstFocusableItem);
    }
}

void ModListTab::refresh() {
    // Reload mods from modList current state
    this->mods = this->modList->getMods();

    // Get current item count
    size_t currentCount = this->list->getViewsCount();
    size_t newCount = mods.size();
    // Keep subscriptions array in sync with list items
    if (clickSubscriptions.size() < currentCount)
        clickSubscriptions.resize(currentCount);

    // If we have items, update them in-place instead of removing/adding
    if (newCount > 0) {
        // Update existing items
        for (size_t i = 0; i < std::min(currentCount, newCount); i++) {
            brls::View* view = this->list->getChild(i);
            brls::ListItem* item = dynamic_cast<brls::ListItem*>(view);
            if (item && i < mods.size()) {
                const auto& mod = mods[i];
                item->setLabel(mod.getName());
                item->setSubLabel(mod.getAuthor());
                // Rebind click handler to current mod (unsubscribe old to avoid stale page selection)
                if (i < clickSubscriptions.size()) {
                    item->getClickEvent()->unsubscribe(clickSubscriptions[i]);
                }
                clickSubscriptions[i] = item->getClickEvent()->subscribe([this, i](brls::View* view) {
                    if (i < mods.size())
                        brls::Application::pushView(new ModDetailPage(this->game, mods[i]));
                    return true;
                });
            }
        }
        
        // Remove extra old items if new count is smaller
        if (newCount < currentCount) {
            for (size_t i = currentCount; i-- > newCount;) {
                if (i < clickSubscriptions.size())
                    clickSubscriptions.erase(clickSubscriptions.begin() + i);
                this->list->removeView(i, true);
            }
        }
        
        // Add new items if new count is larger
        if (newCount > currentCount) {
            for (size_t i = currentCount; i < newCount; i++) {
                const auto& mod = mods[i];
                auto* item = new brls::ListItem(mod.getName(), "", mod.getAuthor());
                auto sub = item->getClickEvent()->subscribe([this, i](brls::View* view) {
                    if (i < mods.size())
                        brls::Application::pushView(new ModDetailPage(this->game, mods[i]));
                    return true;
                });
                if (i >= clickSubscriptions.size())
                    clickSubscriptions.push_back(sub);
                else
                    clickSubscriptions[i] = sub;
                this->list->addView(item);
            }
        }
    } else {
        // No mods, clear and show empty message
        this->list->clear(true);
        clickSubscriptions.clear();
        auto* emptyItem = new brls::ListItem("menus/mods/no_mods_found"_i18n);
        emptyItem->setHeight(LISTITEM_HEIGHT);
        this->list->addView(emptyItem);
    }

    // Store first focusable item for focus restoration
    if (this->list->getViewsCount() > 0) {
        this->firstFocusableItem = this->list->getChild(0);
    }
}

ModListTab::~ModListTab() {
    delete this->modList;
}

void ModListTab::nextPage() {
    this->modList->nextPage();
    refresh();
}

void ModListTab::prevPage() {
    this->modList->previousPage();
    refresh();
}

void ModListTab::startSearch() {
    std::string initial = "";
    brls::Swkbd::openForText([this](std::string text) {
        this->modList->search(text);
        refresh();
    }, "Search mods", "", 128, initial, brls::KeyboardKeyDisableBitmask::KEYBOARD_DISABLE_NONE, "Search");
}
