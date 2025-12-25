#pragma once

#include <vector>
#include <string>
#include <ctime>

namespace smd_utils {
    std::vector<std::pair<std::string, std::string>> getInstalledGames();
    // getIconFromTitleId removed - use GenericToolbox::Switch::Utils::getIconFromTitleId instead
    std::string removeHtmlTags(const std::string& str);
    std::string getModInstallPath();
    std::string timestamp_to_date(time_t timestamp);
    std::string file_size_to_string(int file_size);
}