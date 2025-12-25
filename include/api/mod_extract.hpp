#pragma once

#include <string>

namespace mod_extract {
    int getFileCount(const std::string& archivePath);
    bool extractEntry(const std::string& archivePath, const std::string& entryName, const std::string& outputPath);
    bool extractAll(const std::string& archivePath, const std::string& outputDir);
    bool extractRecursive(const std::string& archivePath, const std::string& outputDir);
    void fixModStructure(const std::string& modDir, const std::string& titleId);
}