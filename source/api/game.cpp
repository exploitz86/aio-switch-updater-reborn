#include "api/game.hpp"
#include "api/net.hpp"
#include <curl/curl.h>

#include <regex>

Game::Game(const std::string& m_title, const std::string& m_tid) {
    title = m_title;
    tid = m_tid;
    searchGame();
    parseJson();
    if (gamebananaID > 0)
        loadCategories();

}

void Game::searchGame() {
    auto curl = curl_easy_init();
    std::string title_url;
    title_url = curl_easy_escape(curl, title.c_str(), title.length());
    curl_easy_cleanup(curl);

    std::string endpoint = "https://gamebanana.com/apiv11/Util/Search/Results?_sModelName=Game&_sOrder=best_match&_sSearchString={}%20%28Switch%29";

    try {
        json = net::downloadRequest(fmt::format(endpoint, title_url));
    } catch (const std::exception& e) {
        brls::Logger::error("Failed to search for game: " + title + " - " + e.what());
        gamebananaID = -1;
        return;
    }
    
    
    if(json.empty()) {
        brls::Logger::error("Failed to search for game: " + title);
    }
}

void Game::parseJson() {
    if(json.empty() || json.at("_aMetadata").at("_nRecordCount").get<int>() == 0) {
        return;
    }

    int pos = 0;

    const auto& record = json.at("_aRecords")[pos];
    title = record.at("_sName").get<std::string>();
    gamebananaID = record.at("_idRow").get<int>();
}

void Game::loadCategories() {
    json = net::downloadRequest(fmt::format("https://gamebanana.com/apiv11/Game/{}/ProfilePage", gamebananaID));

    if(json.empty() || json.find("_aModRootCategories") == json.end()) {
        brls::Logger::error("Failed to load tags for game: {}", title);
        return;
    }

    for (const auto& image : json.at("_aPreviewMedia").at("_aImages")) {
        if (image.at("_sType").get<std::string>() == "banner") {
            bannerURL = image.at("_sUrl").get<std::string>();
            break;
        }
    }

    int index = 0;
    for(auto tag : json.at("_aModRootCategories")) {
        categories.push_back(Category(tag.at("_sName").get<std::string>(), tag.at("_idRow").get<int>(), index));
        index++;
    }
}