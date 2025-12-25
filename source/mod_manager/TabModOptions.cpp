//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "TabModOptions.h"
#include "FrameModBrowser.h"




#include "Logger.h"
#include "borealis/i18n.hpp"
namespace i18n = brls::i18n;
using namespace i18n::literals;


LoggerInit([]{
  Logger::setUserHeaderStr("[TabModOptions]");
});


TabModOptions::TabModOptions(FrameModBrowser* owner_) : _owner_(owner_) {  }

void TabModOptions::buildFolderInstallPresetItem() {

  _itemConfigPreset_ = new brls::ListItem(
    "menus/mods/config_preset"_i18n,
    "menus/mods/config_preset_desc"_i18n,
    ""
  );

  _itemConfigPreset_->setValue("menus/mods/inherited_from_main_menu"_i18n);
  if( not this->getModManager().getCurrentPresetName().empty() ){
    _itemConfigPreset_->setValue( this->getModManager().getCurrentPresetName() );
  }

  // On click : show scrolling up menu
  _itemConfigPreset_->getClickEvent()->subscribe([this](View* view) {
    LogInfo << "Opening config preset selector..." << std::endl;

    // build the choice list + preselection
    int preSelection{0};
    std::vector<std::string> menuList;
    menuList.reserve( 1 + this->getModManager().getConfig().presetList.size() );
    menuList.emplace_back( "menus/mods/inherited_from_main_menu"_i18n );
    int iPreset{-1};
    for( auto& preset: this->getModManager().getConfig().presetList ){
      iPreset++;
      menuList.emplace_back( preset.name + " \uE090 \"" + preset.installBaseFolder + "\"" );
      if( preset.name == this->getModManager().getCurrentPresetName() ){ preSelection = 1 + iPreset; }
    }

    // function that will set the config preset from the Dropdown menu selection (int result)
    brls::ValueSelectedEvent::Callback valueCallback = [this](int result) {
      if( result == -1 ){
        LogDebug << "Not selected. Return." << std::endl;
        // auto pop view
        return;
      }

      if( result == 0 ){
        LogDebug << "Same as config selected. Deleting file..." << std::endl;
        this->getModManager().setCustomPreset("");
        _itemConfigPreset_->setValue( "menus/mods/inherited_from_main_menu"_i18n );
      }
      else{
        LogDebug << "Selected " << result - 1 << std::endl;
        this->getModManager().setCustomPreset( this->getModManager().getConfig().presetList[result - 1].name );
        _itemConfigPreset_->setValue( this->getModManager().getCurrentPresetName() );
      }

      _owner_->getGuiModManager().checkAllMods( true );
      brls::Application::popView();
      return;
    }; // Callback sequence

    brls::Dropdown::open(
        "menus/mods/select_config_preset_for_folder"_i18n,
        menuList, valueCallback,
        preSelection,
        true
    );

  });

}
void TabModOptions::buildResetModsCacheItem() {

  _itemResetModsCache_ = new brls::ListItem(
    "menus/mods/recheck_all_mods"_i18n,
    "menus/mods/recheck_all_mods_desc"_i18n,
    ""
  );

  _itemResetModsCache_->getClickEvent()->subscribe([this](View* view){

    auto* dialog = new brls::Dialog("menus/mods/recheck_mods_confirm"_i18n);

    dialog->addButton("menus/common/yes"_i18n, [&, dialog](brls::View* view) {
      // first, close the dialog box before the async routine starts
      dialog->close();

      // starts the async routine
      _owner_->getGuiModManager().startCheckAllModsThread();
    });
    dialog->addButton("menus/common/no"_i18n, [dialog](brls::View* view) {
      dialog->close();
    });

    dialog->setCancelable(true);
    dialog->open();

  });

}
void TabModOptions::buildDisableAllMods() {

  _itemDisableAllMods_ = new brls::ListItem(
    "menus/mods/disable_all_mods"_i18n,
    "menus/mods/disable_all_mods_desc"_i18n,
    ""
  );

  _itemDisableAllMods_->getClickEvent()->subscribe([this](View* view){

    auto* dialog = new brls::Dialog("menus/mods/disable_all_mods_confirm"_i18n);

    dialog->addButton("menus/common/yes"_i18n, [&, dialog](brls::View* view) {
      // first, close the dialog box before the async routine starts
      dialog->close();

      // starts the async routine
      _owner_->getGuiModManager().startRemoveAllModsThread();
    });
    dialog->addButton("menus/common/no"_i18n, [dialog](brls::View* view) {
      dialog->close();
    });

    dialog->setCancelable(true);
    dialog->open();

  });

}
void TabModOptions::buildGameIdentificationItem(){

  _itemGameIdentification_ = new brls::ListItem(
    "menus/mods/associated_titleid"_i18n,
    "",
    "menus/mods/current_value"_i18n
  );

  if( _owner_->getIcon() != nullptr ){
    _itemGameIdentification_->setThumbnail( _owner_->getIcon(), 0x20000 );
    _itemGameIdentification_->setSubLabel(
        _itemGameIdentification_->getSubLabel() + _owner_->getTitleId()
    );
  }
  else{
    _itemGameIdentification_->setSubLabel(
        _itemGameIdentification_->getSubLabel() + "menus/mods/not_found"_i18n
    );
  }

}

void TabModOptions::initialize() {

  this->buildFolderInstallPresetItem();
  this->buildResetModsCacheItem();
  this->buildDisableAllMods();
  this->buildGameIdentificationItem();

  // finally add to view
  this->addView(_itemResetModsCache_);
  // this->addView(_itemConfigPreset_);
  this->addView(_itemDisableAllMods_);
  this->addView(_itemGameIdentification_);

}

void TabModOptions::draw(NVGcontext *vg, int x, int y, unsigned int width, unsigned int height, brls::Style *style,
                         brls::FrameContext *ctx) {
  ScrollView::draw(vg, x, y, width, height, style, ctx);
}

const ModManager &TabModOptions::getModManager() const {
  return _owner_->getGameBrowser().getModManager();
}
ModManager &TabModOptions::getModManager() {
  return _owner_->getGameBrowser().getModManager();
}




