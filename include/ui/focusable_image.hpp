#pragma once

#include <borealis/image.hpp>

// A simple image view that can optionally be made focusable.
// By default, it's NOT focusable to allow smooth scrolling past it.
// Set focusable=true in constructor if you want it to be selectable.
class FocusableImage : public brls::Image {
public:
    FocusableImage(bool focusable = false) : brls::Image(), isFocusable(focusable) {}
    FocusableImage(unsigned char* buffer, size_t size, bool focusable = false) 
        : brls::Image(buffer, size), isFocusable(focusable) {}

    // Only return self as focus if explicitly set to be focusable
    brls::View* getDefaultFocus() override { 
        return isFocusable ? this : nullptr; 
    }

protected:
    // Disable the background highlight used by list rows
    bool isHighlightBackgroundEnabled() override { return false; }

    // Hide the focus highlight by setting insets that make it invisible
    void getHighlightInsets(unsigned* top, unsigned* right, unsigned* bottom, unsigned* left) override {
        // Set large negative insets to hide the highlight border
        *top = 10000;
        *right = 10000;
        *bottom = 10000;
        *left = 10000;
    }

private:
    bool isFocusable;
};
