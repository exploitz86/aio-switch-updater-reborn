#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <borealis.hpp>

#include "api/game.hpp"

class File {
    public:
        File(const std::string &name, const int &size, const std::string &url, const std::string &checkSum, const std::string& modName, const int& date, const std::string& fileID,const Game& game);
        void loadFile();
        std::string getPath() const { return path; }
        std::string getName() const { return name; }
        std::string getUrl() const { return url; }
        std::string getCheckSum() const { return checkSum; }
        int getDate() const { return date; }
        int getSize() const { return size; }
        Game getGame() const { return game; }
        std::string getModName() const { return modName; }
        bool getRomfs() const { return romfs; }

    private:
        bool findRomfsRecursive(const nlohmann::json& obj);

        std::string name;
        int size;
        std::string url;
        std::string checkSum;
        int date;
        std::string path;
        Game game;
        std::string modName;
        std::string fileID;
        bool romfs = false;
};

class Mod {
    public:
        Mod(const std::string &name, int ID, const std::vector<std::string>& imageUrls, const std::string &author, const Game& game);

        brls::Image* getImage(const int& index);
        std::vector<unsigned char> getImageBuffer(const int& index) const { return imageBuffers[index]; }

        //Separate the download and load image functions because borealis isn't thread safe, loadImage must be called from the main thread
        std::vector<unsigned char> downloadImage(const int& index);
        void loadImage(const int& index);

        std::string getName() const { return name; }
        int getID() const { return ID; }
        std::string getDescription() const { return description; }
        std::vector<File> getFiles() const { return files; }
        std::vector<std::string> getImagesUrl() const { return imageUrls; }
        std::string getAuthor() const { return author; }

        void loadMod();
    private:
        std::string name;
        int ID = 0;
        std::string description;
        std::vector<File> files;

        std::vector<std::string> imageUrls;

        std::vector<brls::Image*> images;
        std::vector<std::vector<unsigned char>> imageBuffers;

        std::vector<unsigned char> imageBuffer;
        std::string author;

        Game game;
};  

class ModList {
    public:
        ModList(Game game);
        //ModList();
        std::vector<Mod> getMods() { return mods; }

        void nextPage();
        void previousPage();
        void search(const std::string& search);
        void setCategory(const Category& category);

        Category getCategory() {return currentCategory;}
    private:
        bool updatePage();
        
        std::vector<Mod> mods;
        int currentPage = 1;
        std::string currentSearch = "";
        Game game;
        Category currentCategory;
        const size_t pageSize = 15;
        std::unordered_map<int, std::vector<Mod>> pageCache;
};