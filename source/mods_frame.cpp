#include "mods_frame.hpp"
#include <borealis.hpp>
#include "mods_tab.hpp"

ModsFrame::ModsFrame() : brls::AppletFrame(true, true) {
    this->setTitle("Mods");
    auto* modsTab = new ModsTab();
    this->setContentView(modsTab);
    this->registerAction("Back", brls::Key::B, []() {
        brls::Application::popView();
        return true;
    });
}
