#include "mod_presets_handler.hpp"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>

namespace {
    std::string modsRoot() { return std::string("sdmc:/mods"); }

    // Trim whitespace from both ends of a string
    void trimString(std::string& str) {
        // Trim from start
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        // Trim from end
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), str.end());
    }
}

ModPresetsHandler::ModPresetsHandler(const std::string& gameName)
    : gameName(gameName) {
    readConfigFile();
}

std::string ModPresetsHandler::getConfigFilePath() const {
    return modsRoot() + "/" + gameName + "/mod_presets.conf";
}

void ModPresetsHandler::readConfigFile() {
    presetList.clear();

    std::string configPath = getConfigFilePath();
    std::ifstream file(configPath);
    if (!file.good()) {
        return;  // File doesn't exist yet, that's okay
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Parse "key = value"
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);
        trimString(key);
        trimString(value);

        if (key == "preset") {
            // Start a new preset
            presetList.emplace_back();
            presetList.back().name = value;
        } else if (key.find("mod") == 0) {
            // Add mod to current preset
            if (!presetList.empty()) {
                presetList.back().modList.push_back(value);
            }
        }
    }
}

void ModPresetsHandler::writeConfigFile() {
    std::string configPath = getConfigFilePath();

    // Ensure parent directory exists
    std::filesystem::path filePath(configPath);
    std::filesystem::create_directories(filePath.parent_path());

    std::ofstream file(configPath, std::ios::trunc);
    if (!file.good()) {
        return;  // Failed to open file for writing
    }

    file << "# This is a config file\n\n";

    for (const auto& preset : presetList) {
        file << "########################################\n";
        file << "# mods preset name\n";
        file << "preset = " << preset.name << "\n\n";
        file << "# mods list\n";

        int modIndex = 0;
        for (const auto& mod : preset.modList) {
            file << "mod" << modIndex++ << " = " << mod << "\n";
        }
        file << "########################################\n\n";
    }
}

void ModPresetsHandler::addPreset(const PresetData& preset) {
    // Check if preset with same name exists
    auto it = std::find_if(presetList.begin(), presetList.end(),
        [&preset](const PresetData& p) { return p.name == preset.name; });

    if (it != presetList.end()) {
        // Replace existing preset
        *it = preset;
    } else {
        // Add new preset
        presetList.push_back(preset);
    }

    writeConfigFile();
}

void ModPresetsHandler::removePreset(const std::string& presetName) {
    auto it = std::find_if(presetList.begin(), presetList.end(),
        [&presetName](const PresetData& p) { return p.name == presetName; });

    if (it != presetList.end()) {
        presetList.erase(it);
        writeConfigFile();
    }
}

const PresetData* ModPresetsHandler::findPreset(const std::string& presetName) const {
    auto it = std::find_if(presetList.begin(), presetList.end(),
        [&presetName](const PresetData& p) { return p.name == presetName; });

    return (it != presetList.end()) ? &(*it) : nullptr;
}
