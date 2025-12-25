#include "api/mod.hpp"
#include "api/net.hpp"
#include "smd_utils/utils.hpp"
#include "constants.hpp"

Mod::Mod(const std::string &name, int ID, const std::vector<std::string>& imageUrls, const std::string &author, const Game& game): game(game) {
    this->name = name;
    this->ID = ID;
    this->imageUrls = imageUrls;
    this->author = author;
}

std::vector<unsigned char> Mod::downloadImage(const int& index) {
    std::vector<unsigned char> buffer;

    net::downloadImage(fmt::format("https://images.gamebanana.com/img/ss/mods/{}",imageUrls[index]), buffer);
       
    if(buffer.size() <= 0) {
        brls::Logger::error("Failed to download image: {}", imageUrls[index]);
        return buffer;
    } 
    imageBuffer = buffer;
    return buffer;
}

void Mod::loadImage(const int& index) {
    std::vector<unsigned char> buffer = imageBuffer;

    if(buffer.size() <= 0) {
        brls::Logger::error("Failed to download image: {}", imageUrls[index]);
        return;
    }
    auto image = new brls::Image();
    image->setImage(buffer.data(), buffer.size());
    images.push_back(image);
    imageBuffers.push_back(buffer);
    imageBuffer.clear();
}


brls::Image* Mod::getImage(const int& index) {
    return images[index];
}

void Mod::loadMod() {
    try {
        nlohmann::json mod_json = net::downloadRequest(fmt::format("https://gamebanana.com/apiv11/Mod/{}?_csvProperties=_sText,_aFiles,_aPreviewMedia", std::to_string(this->ID)));

        this->description = mod_json.at("_sText").get<std::string>();
        this->description = smd_utils::removeHtmlTags(this->description);

        for(auto file : mod_json.at("_aFiles")) {
            std::string name = file.at("_sFile");
            std::string url = file.at("_sDownloadUrl");
            int size = file.at("_nFilesize");
            std::string checkSum = file.at("_sMd5Checksum");
            int date = file.at("_tsDateAdded");
            std::string id = std::to_string(file.at("_idRow").get<int>());

            brls::Logger::debug("File details: Name: {}, URL: {}, Size: {}, Checksum: {}, Date: {}, ID: {}", name, url, size, checkSum, date, id);

            files.push_back(File(name, size, url, checkSum, this->getName(), date, id, game));
        }

        for(auto image : mod_json.at("_aPreviewMedia").at("_aImages")) {
            std::string url = image.at("_sFile");

            brls::Logger::debug("Image URL: {}", url);

            imageUrls.push_back(url);
        }
    } catch (const std::exception& e) {
        brls::Logger::error("Error in loadMod: {}", e.what());
    }
}

File::File(const std::string &name, const int &size, const std::string &url, const std::string &checkSum, const std::string& modName,const int& date, const std::string& fileID,const Game& game): game(game) {            
    this->name = name;
    this->size = size;
    this->url = url;
    this->checkSum = checkSum;
    this->date = date;
    this->path = fmt::format("{}{}", CONFIG_PATH, name);
    this->modName = modName;
    this->fileID = fileID;
}

bool File::findRomfsRecursive(const nlohmann::json& obj) {
    for (const auto& item : obj.items()) {
        if (item.key() == "romfs" || item.key() == "exefs" || item.key() == "exefs_patches"){
            brls::Logger::debug("found romfs");
            return true;
        }

        if (item.value().is_object()) {
            if (findRomfsRecursive(item.value())) {
                return true;
            }
        }
    }

    return false;
}

void File::loadFile() {
    auto json = net::downloadRequest(fmt::format("https://gamebanana.com/apiv11/File/{}", fileID));
    nlohmann::json archiveFileTree = json;
    // Check if the correct json object is present. 
    if (json.contains("_aArchiveFileTree") && json["_aArchiveFileTree"].is_object()) {
        archiveFileTree = json["_aArchiveFileTree"];
    } else if (json.contains("_aMetadata") &&
               json["_aMetadata"].contains("_aArchiveFileTree") &&
               json["_aMetadata"]["_aArchiveFileTree"].is_object()) {
        archiveFileTree = json["_aMetadata"]["_aArchiveFileTree"];
    } else {
        brls::Logger::error("Could not find correct JSON object: {}", json.dump(2));
        return;
    }

    for (const auto& item : archiveFileTree.items()) {
        if (item.value().is_object()) {
            if (findRomfsRecursive(item.value())) {
                this->romfs = true;
                break;
            }
        }
    }
}

ModList::ModList(Game m_game): game(m_game) {
    updatePage();
}

bool ModList::updatePage() {
    try {
    auto cached = pageCache.find(currentPage);
    if (cached != pageCache.end()) {
        mods = cached->second;
        return true;
    }

    nlohmann::json mod_json;

    if (currentCategory.getName() != "") {
        mod_json = net::downloadRequest(fmt::format("https://gamebanana.com/apiv11/Mod/Index?_nPerpage={}&_aFilters[Generic_Category]={}&_nPage={}", pageSize, currentCategory.getID(), currentPage));
    }
    else if(currentSearch == "") {
        mod_json = net::downloadRequest(fmt::format("https://gamebanana.com/apiv11/Game/{}/Subfeed?_nPage={}&_nPerpage={}&_csvModelInclusions=Mod", game.getGamebananaID(), currentPage, pageSize));
    } 
    else {
        mod_json = net::downloadRequest(fmt::format("https://gamebanana.com/apiv11/Game/{}/Subfeed?_nPage={}&_nPerpage={}&_sName={}&_csvModelInclusions=Mod", game.getGamebananaID(), currentPage, pageSize, currentSearch));
    }

    if(mod_json.empty() || !mod_json.contains("_aRecords")) {
        brls::Logger::error("ModList updatePage: empty or missing _aRecords (page={})", currentPage);
        return false;
    }

    mods.clear();
    auto records = mod_json.at("_aRecords");
    if (records.empty()) {
        brls::Logger::error("ModList updatePage: records empty (page={})", currentPage);
        return false;
    }

    for(auto mod : records) {
        std::string name = mod.at("_sName");
        int ID = mod.at("_idRow");
        
        std::string author = mod.at("_aSubmitter").at("_sName");

        std::vector<std::string> images;

        Mod newMod(name, ID, images, author, game);

        mods.push_back(newMod);
    }

    // No client-side strict filtering; rely on API results

    pageCache[currentPage] = mods;
    return true;
    } catch (const std::exception& e) {
        brls::Logger::error("updatePage failed: {}", e.what());
        return false;
    }
}

void ModList::nextPage() {
    int target = currentPage + 1;
    int previous = currentPage;
    currentPage = target;
    if (!updatePage()) {
        currentPage = previous;
        updatePage();
    }
}

void ModList::previousPage() {
    if(currentPage > 1) {
        currentPage--;
        updatePage();
    }
}

void ModList::search(const std::string& search) {
    if(search.size() < 3) {
        return;
    }
    this->currentSearch = search;
    pageCache.clear();
    currentPage = 1;
    updatePage();
}

void ModList::setCategory(const Category& category) {
    this->currentCategory = category;
    pageCache.clear();
    currentPage = 1;
    updatePage();
}
