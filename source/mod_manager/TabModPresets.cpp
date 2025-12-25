//
// Created by Adrien BLANCHET on 22/06/2020.
//

#include "TabModPresets.h"

#include "FrameModBrowser.h"

#include <ThumbnailPresetEditor.h>

#include <borealis.hpp>
#include "Logger.h"
#include "borealis/i18n.hpp"
namespace i18n = brls::i18n;
using namespace i18n::literals;

LoggerInit([]{
  Logger::setUserHeaderStr("[TabModPresets]");
});


void TabModPresets::draw(NVGcontext *vg, int x, int y, unsigned width, unsigned height, brls::Style *style, brls::FrameContext *ctx) {

  if( _triggerUpdateItem_ ){
    this->updatePresetItems();
  }

  this->brls::List::draw( vg,x,y,width,height,style,ctx );
}

void TabModPresets::assignButtons(brls::ListItem *item, bool isPreset_) {

  if( item == nullptr ){
    LogError << "Can't assign buttons to nullptr item" << std::endl;
    return;
  }

  // reset
  item->clearActions();
//  item->getClickEvent()->subscribe([](brls::View* view){});
//  item->registerAction("", brls::Key::A, []{ return true; });
//  item->registerAction("", brls::Key::X, []{ return true; });
//  item->registerAction("", brls::Key::Y, []{ return true; });
//  item->updateActionHint(brls::Key::A, "");
//  item->updateActionHint(brls::Key::X, "");
//  item->updateActionHint(brls::Key::Y, "");
//  item->setActionAvailable(brls::Key::A, false);
//  item->setActionAvailable(brls::Key::X, false);
//  item->setActionAvailable(brls::Key::Y, false);

  auto* ownerPtr{_owner_};

  if( isPreset_ ){
    item->registerAction("menus/mods/apply"_i18n, brls::Key::A, [item, ownerPtr]{
      auto* dialog = new brls::Dialog(fmt::format("menus/mods/apply_preset_confirm"_i18n, item->getLabel()));

      dialog->addButton("menus/common/yes"_i18n, [&, dialog, item](brls::View* view) {
        // first, close the dialog box before the apply mod thread starts
        dialog->close();

        // starts the async routine
        ownerPtr->getGuiModManager().startApplyModPresetThread(item->getLabel());
      });
      dialog->addButton("menus/common/no"_i18n, [dialog](brls::View* view) {
        dialog->close();
      });

      dialog->setCancelable(true);
      dialog->open();
      return true;
    });
    item->registerAction("menus/mods/remove"_i18n, brls::Key::X, [this, ownerPtr, item]{

      auto* dialog = new brls::Dialog(fmt::format("menus/mods/delete_preset_confirm"_i18n, item->getLabel()));

      dialog->addButton("menus/common/yes"_i18n, [this, ownerPtr, item, dialog](brls::View* view) {
        ownerPtr->getGameBrowser().getModPresetHandler().deletePreset( item->getLabel() );
        dialog->close();
        this->setTriggerUpdateItem( true );
      });
      dialog->addButton("menus/common/no"_i18n, [dialog](brls::View* view) { dialog->close(); });

      dialog->setCancelable(true);
      dialog->open();
      return true;

    });
    item->registerAction("menus/mods/edit"_i18n, brls::Key::Y, [ownerPtr, item]{
      // open editor
      auto* editor = new ThumbnailPresetEditor( ownerPtr, item->getLabel() );

      auto* icon = ownerPtr->getIcon();
      if(icon != nullptr){
        brls::PopupFrame::open(
            "menus/mods/edit_preset"_i18n, icon, 0x20000, editor,
            "menus/mods/select_mods_to_install"_i18n,
            "menus/mods/mods_applied_in_order"_i18n
        );
      }
      else{
        brls::PopupFrame::open(
            "menus/mods/edit_preset"_i18n, editor,
            "menus/mods/select_mods_to_install"_i18n,
            "menus/mods/mods_applied_in_order"_i18n
        );
      }

      return true;
    });

    item->updateActionHint(brls::Key::A, "menus/mods/apply"_i18n);
    item->updateActionHint(brls::Key::X, "menus/mods/remove"_i18n);
    item->updateActionHint(brls::Key::Y, "menus/mods/edit"_i18n);

    item->setActionAvailable(brls::Key::A, true);
    item->setActionAvailable(brls::Key::X, true);
    item->setActionAvailable(brls::Key::Y, true);
  }
  else {
    // new preset
    item->registerAction("menus/mods/create"_i18n, brls::Key::A, [ownerPtr]{
      // create new preset
      auto* editor = new ThumbnailPresetEditor( ownerPtr );

      auto* icon = ownerPtr->getIcon();
      if(icon != nullptr){
        brls::PopupFrame::open(
            "menus/mods/new_preset"_i18n, icon, 0x20000, editor,
            "menus/mods/select_mods_to_install"_i18n,
            "menus/mods/mods_applied_in_order"_i18n
        );
      }
      else{
        brls::PopupFrame::open(
            "menus/mods/new_preset"_i18n, editor,
            "menus/mods/select_mods_to_install"_i18n,
            "menus/mods/mods_applied_in_order"_i18n
        );
      }

      return true;
    });
    item->updateActionHint(brls::Key::A, "menus/mods/create"_i18n);

    item->setActionAvailable(brls::Key::A, true);
  }

}

void TabModPresets::updatePresetItems() {
  LogInfo << "Updating displayed preset items" << std::endl;

  _triggerUpdateItem_ = false;

  // adding presets to the list
  auto presetList = _owner_->getGameBrowser().getModPresetHandler().getPresetList();

  // make sure it is the right size
  _itemList_.reserve( presetList.size() + 1 );

  // keep the index out of the for loop
  size_t iPreset = 0;

  LogInfo << "Adding " << presetList.size() << " presets..." << std::endl;
  for( ; iPreset < presetList.size() ; iPreset++ ){
    LogScopeIndent;
    LogInfo << "Adding mod preset: " << presetList[iPreset].name << std::endl;
    if( iPreset+1 > _itemList_.size() ){
      // missing slot
      _itemList_.emplace_back( new brls::ListItem( presetList[iPreset].name ) );

      // add it once
      this->addView( _itemList_[ iPreset ] );
    }
    this->assignButtons( _itemList_[ iPreset ], true );
    _itemList_[ iPreset ]->setLabel( presetList[iPreset].name );
    _itemList_[ iPreset ]->setValue( fmt::format("menus/mods/x_mods_in_set"_i18n, presetList[iPreset].modList.size()) );
    if( _itemList_[iPreset]->isCollapsed() ) _itemList_[iPreset]->expand();
  }

  // need one more slot for create new preset
  if( iPreset+1 > _itemList_.size() ){
    LogInfo << "(re)-Creating add button" << std::endl;
    _itemList_.emplace_back( new brls::ListItem( "menus/mods/create_new_preset"_i18n ) );

    this->addView( _itemList_[ iPreset ] );
  }
  this->assignButtons( _itemList_[ iPreset ], false );
  if( _itemList_[iPreset]->isCollapsed() ) _itemList_[iPreset]->expand();
  _itemList_[ iPreset ]->setLabel( "menus/mods/create_new_preset"_i18n );
  _itemList_[ iPreset ]->setValue( "" );
  iPreset++;

  // hide the slots that were already created
  for( ; iPreset < _itemList_.size() ; iPreset++ ){
    _itemList_[iPreset]->collapse();
  }

  LogTrace << GET_VAR_NAME_VALUE(getViewsCount()) << std::endl;
  LogTrace << GET_VAR_NAME_VALUE(_itemList_.size()) << std::endl;

  // focus is lost as the list has been cleared
//  brls::Application::giveFocus( this->getDefaultFocus() );

  // does not work, focus is lost as
//  if(brls::Application::getCurrentFocus() != nullptr and brls::Application::getCurrentFocus()->isCollapsed()){
//    brls::Application::getCurrentFocus()->onFocusLost();
//    brls::Application::giveFocus(_itemNewCreatePreset_->getDefaultFocus());
//  }

  brls::Application::unblockInputs();
  LogInfo << "Leaving update..." << std::endl;
}




