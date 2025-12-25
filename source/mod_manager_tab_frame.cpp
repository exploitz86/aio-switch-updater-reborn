#include "mod_manager_tab_frame.hpp"
#include "mod_manager_page.hpp"
#include <borealis.hpp>

namespace i18n = brls::i18n;
using namespace i18n::literals;

ModManagerTabFrame::ModManagerTabFrame() : brls::AppletFrame(true, true) {
    this->setTitle("menus/mods/mod_manager"_i18n);
    this->setContentView(new ModManagerPage());
}
