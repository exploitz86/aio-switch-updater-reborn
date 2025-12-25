#include "ui/image_viewer_page.hpp"

ImageViewerPage::ImageViewerPage(Mod& mod, const std::vector<std::string>& urls, size_t startIndex, const std::string& title)
    : brls::AppletFrame(true, true), mod(mod), urls(urls), index(startIndex) {
    this->buildUI(title);
}

void ImageViewerPage::buildUI(const std::string& title) {
    this->scroll = new brls::ScrollView();
    this->container = new brls::BoxLayout(brls::BoxLayoutOrientation::VERTICAL);
    this->container->setResize(true);
    this->container->setSpacing(0);

    this->image = new FocusableImage(true);
    this->image->setScaleType(brls::ImageScaleType::FIT);
    this->image->setHeight(555);
    this->container->addView(this->image);

    // Register L/R navigation on the image itself
    this->image->registerAction("⇐", brls::Key::L, [this]() {
        if (this->index > 0) {
            this->index--;
            this->loadCurrent();
            this->updateFooter();
        }
        return true;
    });
    this->image->registerAction("⇒", brls::Key::R, [this]() {
        if (this->index + 1 < this->urls.size()) {
            this->index++;
            this->loadCurrent();
            this->updateFooter();
        }
        return true;
    });

    this->scroll->setContentView(this->container);
    this->setContentView(this->scroll);

    this->setTitle(title);
    this->updateFooter();
    this->loadCurrent();

    // Give focus to the image
    brls::Application::giveFocus(this->image);
}

void ImageViewerPage::updateFooter() {
    // Show "Picture x/y" in footer
    this->setFooterText("Picture " + std::to_string(this->index + 1) + "/" + std::to_string(this->urls.size()));
}

void ImageViewerPage::loadImage(size_t imageIndex) {
    // Check cache first
    if (imageCache.find(imageIndex) != imageCache.end()) {
        return; // Already loaded
    }

    // Download and cache the image
    brls::Logger::debug("Loading image {}", imageIndex);
    std::vector<unsigned char> buffer = this->mod.downloadImage(static_cast<int>(imageIndex));
    if (!buffer.empty()) {
        imageCache[imageIndex] = buffer;
    }
}

void ImageViewerPage::loadCurrent() {
    // Load the current image (lazy load if not in cache)
    loadImage(this->index);

    // Display from cache - use iterator to avoid reference invalidation
    auto it = imageCache.find(this->index);
    if (it != imageCache.end() && !it->second.empty()) {
        this->image->setImage(it->second.data(), it->second.size());
    }
}


ImageViewerPage::~ImageViewerPage() {
    // Clear the image before destroying to avoid dangling pointer issues
    brls::Logger::debug("ImageViewerPage: Destructor called, clearing {} cached images", imageCache.size());

    // Set image to empty to clear any references to cache data
    if (this->image) {
        // Clear the image data before cache is destroyed
        std::vector<unsigned char> empty;
        this->image->setImage(empty.data(), 0);
    }

    // imageCache will be automatically cleaned up after this
}
