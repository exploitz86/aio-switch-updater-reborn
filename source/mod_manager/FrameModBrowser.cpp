//
// Created by Adrien BLANCHET on 21/06/2020.
//

#include "FrameModBrowser.h"

#include <TabModBrowser.h>
#include <TabModPresets.h>
#include <TabModOptions.h>

#include "GenericToolbox.Switch.h"
#include "Logger.h"
#include "borealis/i18n.hpp"
namespace i18n = brls::i18n;
using namespace i18n::literals;


LoggerInit([]{
  Logger::setUserHeaderStr("[FrameModBrowser]");
});


FrameModBrowser::FrameModBrowser(GuiModManager* guiModManagerPtr_) : _guiModManagerPtr_(guiModManagerPtr_) {

  // fetch game title
  this->setTitle( getGameBrowser().getModManager().getGameName() );

  std::string gamePath = getGameBrowser().getModManager().getGameFolderPath();

  _titleId_ = GenericToolbox::Switch::Utils::lookForTidInSubFolders( gamePath, 5);
  uint8_t* rawIcon = GenericToolbox::Switch::Utils::getIconFromTitleId( _titleId_ );
  if(rawIcon != nullptr){
    _icon_.reset(rawIcon);  // Transfer ownership to unique_ptr
    this->setIcon(_icon_.get(), 0x20000);
  }
  else{ this->setIcon("romfs:/images/icon_corner.png"); }

  // Set the Title ID in ModManager so it knows which folder to use
  getGameBrowser().getModManager().setTitleId(_titleId_);

  // this->setFooterText("AIO Switch Updater Reborn");


  if( not getGameBrowser().getModManager().getModList().empty() ){

    _tabModBrowser_ = new TabModBrowser( this );
    _tabModPresets_ = new TabModPresets( this );
    _tabModOptions_ = new TabModOptions( this );

    _tabModOptions_->initialize();

    this->addTab("menus/mods/tab_mods"_i18n, _tabModBrowser_);
    this->addSeparator();
    this->addTab("menus/mods/tab_presets"_i18n, _tabModPresets_);
    this->addTab("menus/mods/tab_options"_i18n, _tabModOptions_);

  }
  else{
    auto* list = new brls::List();
    LogError("Can't open: %s", gamePath.c_str());
    auto* item = new brls::ListItem("menus/mods/error_cant_open"_i18n + gamePath , "", "");
    list->addView(item);
    this->addTab("menus/mods/tab_mod_browser"_i18n, list);
  }

}

FrameModBrowser::~FrameModBrowser() {
  // Icon memory is automatically freed by unique_ptr
}

bool FrameModBrowser::onCancel() {

  // Go back to sidebar
  auto* lastFocus = brls::Application::getCurrentFocus();
  brls::Application::onGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT, false);

  // If the sidebar was already there, the focus has not changed
  if(lastFocus == brls::Application::getCurrentFocus()){
    LogInfo("Back on games screen...");
    brls::Application::popView(brls::ViewAnimation::SLIDE_RIGHT);
  }
  return true;

}

uint8_t *FrameModBrowser::getIcon() {
  return _icon_.get();
}
std::string FrameModBrowser::getTitleId() {
  return _titleId_;
}
