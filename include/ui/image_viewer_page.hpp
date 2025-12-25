#pragma once

#include <borealis.hpp>
#include <string>
#include <vector>
#include <map>

#include "api/mod.hpp"
#include "ui/focusable_image.hpp"

// Fullscreen-ish image viewer with left/right navigation
class ImageViewerPage : public brls::AppletFrame {
public:
    ImageViewerPage(Mod& mod, const std::vector<std::string>& urls, size_t startIndex, const std::string& title);
    ~ImageViewerPage();

private:
    Mod& mod;
    std::vector<std::string> urls;
    size_t index = 0;

    brls::ScrollView* scroll = nullptr;
    brls::BoxLayout* container = nullptr;
    FocusableImage* image = nullptr;

    // Cache for loaded images (lazy loading)
    std::map<size_t, std::vector<unsigned char>> imageCache;

    void buildUI(const std::string& title);
    void loadCurrent();
    void loadImage(size_t imageIndex);
    void updateFooter();
};
