#pragma once
#include <borealis.hpp>

class ModManagerPage : public brls::List {
  public:
    ModManagerPage();

    // Override lifecycle methods for sleep/wake handling
    void willAppear(bool resetState = false) override;
    void willDisappear(bool resetState = false) override;
};

