#include "mod_detail_page.hpp"

#include <regex>

#include "confirm_page.hpp"
#include "worker_page.hpp"
#include "download.hpp"
#include "api/mod_extract.hpp"
#include "smd_utils/utils.hpp"
#include "smd_utils/progress_event.hpp"
#include "constants.hpp"
#include "ui/image_viewer_page.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

static std::string sanitize(const std::string& s)
{
    std::regex badChars("[:/\\<>|*]");
    std::regex accent("é");
    return std::regex_replace(std::regex_replace(s, badChars, "-"), accent, "e");
}

ModDetailPage::ModDetailPage(const Game& game, const Mod& mod)
        : brls::AppletFrame(true, true),
            game(game),
      // Recreate the Mod with a local Game reference to avoid dangling refs
      mod(Mod(mod.getName(), mod.getID(), mod.getImagesUrl(), mod.getAuthor(), this->game))
{
    // Build a clean scrollable container instead of a List to avoid row visuals
    this->scroll = new brls::ScrollView();
    this->container = new brls::BoxLayout(brls::BoxLayoutOrientation::VERTICAL);
    this->container->setResize(true);
    this->container->setSpacing(14);
    this->container->setMargins(12, 24, 20, 24);
    this->scroll->setContentView(this->container);
    this->setContentView(this->scroll);
    this->setTitle(fmt::format("{} - {}", this->game.getTitle(), this->mod.getName()));

    buildUI();
}

void ModDetailPage::buildUI()
{
    // Ensure we have description and files
    this->mod.loadMod();

    // Header: Mod title
    // this->container->addView(new brls::Header(this->mod.getName()));

    // Files at the top
    this->container->addView(new brls::Header("menus/mods/download_links"_i18n));

    auto files = this->mod.getFiles();
    if (files.empty())
    {
        auto* none = new brls::ListItem("menus/mods/no_files"_i18n);
        this->container->addView(none);
    }
    else
    {
        for (const auto& f : files)
            addFileItem(f);
    }

    // Description below files
    if (!this->mod.getDescription().empty())
    {
        this->container->addView(new brls::Header("menus/mods/description"_i18n));
        auto* desc = new brls::Label(brls::LabelStyle::DESCRIPTION, this->mod.getDescription(), true);
        this->container->addView(desc);
    }

    // View Images button at the bottom (only if images exist)
    if (!this->mod.getImagesUrl().empty())
    {
        auto* viewImagesItem = new brls::ListItem("menus/mods/view_images"_i18n);
        viewImagesItem->setHeight(LISTITEM_HEIGHT);
        viewImagesItem->getClickEvent()->subscribe([this](brls::View* view) {
            brls::Application::pushView(new ImageViewerPage(this->mod, this->mod.getImagesUrl(), 0, this->mod.getName()));
            return true;
        });
        this->container->addView(viewImagesItem);
    }

    // Store first focusable item for focus restoration
    for (size_t i = 0; i < this->container->getViewsCount(); i++)
    {
        brls::View* child = this->container->getChild(i);
        brls::ListItem* item = dynamic_cast<brls::ListItem*>(child);
        if (item)
        {
            this->firstFocusableItem = item;
            break;
        }
    }
}

void ModDetailPage::addFileItem(const File& file)
{
    auto* item = new brls::ListItem(file.getName());
    item->setValue(fmt::format("{} • {}", smd_utils::file_size_to_string(file.getSize()), smd_utils::timestamp_to_date(static_cast<time_t>(file.getDate()))));
    item->setHeight(LISTITEM_HEIGHT);

    item->getClickEvent()->subscribe([this, file](brls::View* view) {
        // Load file metadata (romfs/exefs detection)
        File f = file; // make a copy we can operate on
        f.loadFile();

        // Build output directory - use POSIX path for Switch file I/O
        std::string base = fmt::format("/{}/{}/{}",
            smd_utils::getModInstallPath(),
            sanitize(f.getGame().getTitle()),
            sanitize(f.getModName()));

        // Stage flow: confirm -> download -> extract -> done
        auto* staged = new brls::StagedAppletFrame();
        staged->setTitle(fmt::format("{}", "menus/common/downloading"_i18n));

        std::string confirmText = fmt::format("{}: {}", "menus/common/download"_i18n, f.getName());
        staged->addStage(new ConfirmPage(staged, confirmText));

        // Download to temporary path managed by File
        staged->addStage(new WorkerPage(staged, "menus/common/downloading"_i18n, [f]() {
            download::downloadFile(f.getUrl(), f.getPath(), OFF);
        }));

        // Extract to target structure
        staged->addStage(new WorkerPage(staged, "menus/common/extracting"_i18n, [f, base]() {
            mod_extract::extractRecursive(f.getPath(), base);
            mod_extract::fixModStructure(base, f.getGame().getTid());
        }));

        staged->addStage(new ConfirmPage_Done(staged, "menus/common/all_done"_i18n));
        brls::Application::pushView(staged);
        return true;
    });

    this->container->addView(item);
}

void ModDetailPage::willAppear(bool resetState)
{
    brls::AppletFrame::willAppear(resetState);

    // Restore focus to first item when returning from image viewer
    if (!resetState && this->firstFocusableItem) {
        brls::Application::giveFocus(this->firstFocusableItem);
    }
}

ModDetailPage::~ModDetailPage() {}
