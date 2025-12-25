#pragma once

#include <borealis.hpp>

class CheatsPage : public brls::List
{
private:
    brls::ListItem* item;
    brls::StagedAppletFrame* stagedFrame;

public:
    CheatsPage();
};