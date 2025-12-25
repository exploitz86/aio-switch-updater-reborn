#include "mods_tab.hpp"
#include "mod_manager_page.hpp"
#include "mod_manager_tab_frame.hpp"
#include "game_list_tab.hpp"
#include "constants.hpp"
#include "confirm_page.hpp"
#include "worker_page.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "utils.hpp"
#include "fs.hpp"
#include <borealis.hpp>

namespace i18n = brls::i18n;
using namespace i18n::literals;

ModsTab::ModsTab() : brls::List() {
    // Check if running in applet mode
    if (util::isApplet()) {
        // Show warning that mod features require full application mode
        this->addView(new brls::Header("menus/mods/mod_features"_i18n));
        auto* warningLabel = new brls::Label(
            brls::LabelStyle::SMALL,
            "menus/common/applet_mode_not_supported"_i18n,
            true
        );
        this->addView(warningLabel);

        auto* infoLabel = new brls::Label(
            brls::LabelStyle::DESCRIPTION,
            "menus/mods/applet_mode_explanation"_i18n,
            true
        );
        this->addView(infoLabel);
        return;
    }

    // Header
    this->addView(new brls::Label(brls::LabelStyle::DESCRIPTION,"menus/mods/mod_downloader_desc"_i18n,true));

    // List item: Browse and download mods by game
    auto* browseItem = new brls::ListItem("menus/mods/browse_mods_by_game"_i18n);
    browseItem->setHeight(LISTITEM_HEIGHT);
    browseItem->getClickEvent()->subscribe([](brls::View* view) {
        brls::Application::pushView(new GameListTab());
        return true;
    });
    this->addView(browseItem);

    // List item: Download Mod from URL
    // auto* downloaderItem = new brls::ListItem("menus/mods/download_mod_from_url"_i18n);
    // downloaderItem->setHeight(LISTITEM_HEIGHT);
    // downloaderItem->getClickEvent()->subscribe([](brls::View* view) {
    //     std::string url;
    //     if (!brls::Swkbd::openForText([&url](std::string text) { url = text; }, "Mod URL", "", 512, "https://example.com/mod.zip", 0, "Download", "https://example.com/mod.zip")) {
    //         return true;
    //     }

    //     std::string archivePath = std::string(APP_PATH) + "mod_download.zip";

    //     auto* stagedFrame = new brls::StagedAppletFrame();
    //     stagedFrame->setTitle("menus/mods/downloading_extracting_mod"_i18n);

    //     stagedFrame->addStage(new ConfirmPage(stagedFrame, fmt::format("menus/mods/downloading_from"_i18n, url)));
    //     stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/downloading"_i18n, [url, archivePath]() {
    //         download::downloadFile(url, archivePath);
    //     }));
    //     stagedFrame->addStage(new WorkerPage(stagedFrame, "menus/common/extracting"_i18n, [archivePath]() {
    //         extract::extract(archivePath, ROOT_PATH);
    //     }));
    //     stagedFrame->addStage(new ConfirmPage(stagedFrame, "menus/common/all_done"_i18n));

    //     brls::Application::pushView(stagedFrame);
    //     return true;
    // });
    // this->addView(downloaderItem);

    // List item: Mod Manager

    this->addView(new brls::Label(brls::LabelStyle::DESCRIPTION,"menus/mods/mod_manager_desc"_i18n,true));

    auto* managerItem = new brls::ListItem("menus/mods/mod_manager"_i18n);
    managerItem->setHeight(LISTITEM_HEIGHT);
    managerItem->getClickEvent()->subscribe([](brls::View* view) {
        brls::Application::pushView(new ModManagerTabFrame());
        return true;
    });
    this->addView(managerItem);

    // Mods settings removed
}
