#pragma once

#include <string>
#include <vector>

// Structure to hold a single preset's data
struct PresetData {
    std::string name;
    std::vector<std::string> modList;  // Ordered list of mod folder names
};

// Handler for managing mod presets for a specific game
class ModPresetsHandler {
public:
    explicit ModPresetsHandler(const std::string& gameName);

    // Load presets from config file
    void readConfigFile();

    // Save presets to config file
    void writeConfigFile();

    // Get all presets
    const std::vector<PresetData>& getPresetList() const { return presetList; }
    std::vector<PresetData>& getPresetList() { return presetList; }

    // Add a new preset
    void addPreset(const PresetData& preset);

    // Remove a preset by name
    void removePreset(const std::string& presetName);

    // Find a preset by name
    const PresetData* findPreset(const std::string& presetName) const;

    // Get config file path
    std::string getConfigFilePath() const;

private:
    std::string gameName;
    std::vector<PresetData> presetList;
};
