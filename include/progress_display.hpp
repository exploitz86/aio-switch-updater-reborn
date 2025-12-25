#pragma once

#include <borealis.hpp>
#include <functional>
#include <string>
#include <thread>
#include <chrono>

namespace ProgressDisplay {

const NVGcolor greenNvgColor{nvgRGB(0x00, 0xff, 0xc8)};

class ProgressBarMonitorView : public brls::View {
public:
    ProgressBarMonitorView() = default;
    ~ProgressBarMonitorView() override {
        if (_execOnDelete_) { _execOnDelete_(); }
    }

    void setHeaderTitle(const std::string& header) { _header_ = header; }
    void setProgressColor(const NVGcolor& progressColor) { _progressColor_ = progressColor; }
    void setExecOnDelete(const std::function<void()>& execOnDelete) { _execOnDelete_ = execOnDelete; }
    void setTitlePtr(const std::string* titlePtr) { _titlePtr_ = titlePtr; }
    void setSubTitlePtr(const std::string* subTitlePtr) { _subTitlePtr_ = subTitlePtr; }
    void setProgressFractionPtr(const double* progressFractionPtr) { _progressFractionPtr_ = progressFractionPtr; }
    void setSubProgressFractionPtr(const double* subProgressFractionPtr) { _subProgressFractionPtr_ = subProgressFractionPtr; }

    void resetMonitorAddresses() {
        _titlePtr_ = nullptr;
        _subTitlePtr_ = nullptr;
        _progressFractionPtr_ = nullptr;
        _subProgressFractionPtr_ = nullptr;
    }

    void draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height,
              brls::Style* style, brls::FrameContext* ctx) override {
        float y_offset = 0;
        if (!_header_.empty()) {
            y_offset += 12;
            nvgBeginPath(vg);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgFontSize(vg, float(style->Header.fontSize));
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgText(vg, float(x) + float(style->Header.rectangleWidth) + float(style->Header.padding),
                    float(y) - float(height / 2.), _header_.c_str(), nullptr);

            nvgBeginPath(vg);
            nvgFillColor(vg, a(ctx->theme->separatorColor));
            nvgRect(vg, float(x), float(y) - float(height / 2.) + float(style->Header.fontSize), float(width), 1);
            nvgFill(vg);
        }

        if (_titlePtr_ != nullptr) {
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgFontSize(vg, float(style->Label.dialogFontSize));
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgTextLineHeight(vg, 1.0f);
            nvgTextAlign(vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
            nvgBeginPath(vg);
            nvgText(vg, float(x) + float(width / 2.), y + y_offset + height / 2. - 1.8 * style->Label.dialogFontSize,
                    (*_titlePtr_).c_str(), nullptr);
        }

        if (_subTitlePtr_ != nullptr && !_subTitlePtr_->empty()) {
            nvgBeginPath(vg);
            nvgFontFaceId(vg, ctx->fontStash->regular);
            nvgFontSize(vg, style->Header.fontSize);
            nvgFillColor(vg, a(ctx->theme->textColor));
            nvgTextAlign(vg, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
            nvgText(vg, x + style->Header.rectangleWidth + style->Header.padding, y + y_offset + height / 2.,
                    (*_subTitlePtr_).c_str(), nullptr);
        }

        unsigned pBarYoffset = y + y_offset + 0.8f * height;
        if (_progressFractionPtr_ != nullptr) {
            drawProgressBar(vg, x, width, pBarYoffset, *_progressFractionPtr_);
        }
        
        // Draw sub-progress bar below the main progress bar
        if (_subProgressFractionPtr_ != nullptr) {
            unsigned subPBarYoffset = pBarYoffset + 25; // Offset below main bar
            drawProgressBar(vg, x, width, subPBarYoffset, *_subProgressFractionPtr_);
        }
    }

private:
    void drawProgressBar(NVGcontext* vg, int x, unsigned int width, unsigned int yPosition, double fraction) {
        if (fraction > 1) fraction = 1;
        else if (fraction < 0) fraction = 0;

        unsigned x_margin = 15;
        unsigned totalBarLength = width - 2 * x_margin;
        unsigned progress_x_offset = x + x_margin;

        if (fraction != 1) {
            nvgFillColor(vg, a(nvgRGBAf(1.f, 1.f, 1.f, 0.5f)));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, float(progress_x_offset), float(yPosition), float(totalBarLength), 16, 8);
            nvgFill(vg);
        }

        if (fraction != 0) {
            nvgFillColor(vg, a(_progressColor_));
            nvgBeginPath(vg);
            nvgRoundedRect(vg, float(progress_x_offset), float(yPosition), totalBarLength * fraction, 16, 8);
            nvgFill(vg);
        }
    }

    std::string _header_{};
    NVGcolor _progressColor_{greenNvgColor};
    std::function<void()> _execOnDelete_{};
    const double* _progressFractionPtr_{nullptr};
    const double* _subProgressFractionPtr_{nullptr};
    const std::string* _titlePtr_{nullptr};
    const std::string* _subTitlePtr_{nullptr};
};

class PopupLoadingBox {
public:
    PopupLoadingBox() = default;

    ProgressBarMonitorView* getMonitorView() const { return _monitorView_; }
    brls::Dialog* getLoadingBox() const { return _loadingBox_; }

    void pushView() {
        _monitorView_ = new ProgressBarMonitorView();
        _loadingBox_ = new brls::Dialog(_monitorView_);
        _loadingBox_->setCancelable(true);

        // Wait for any pending view to disappear
        while (brls::Application::hasViewDisappearing()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        brls::Application::pushView(_loadingBox_, brls::ViewAnimation::FADE);
    }

    void popView() const {
        // Wait for any pending view to disappear
        while (brls::Application::hasViewDisappearing()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
        // Only pop if this is the top view
        if (_loadingBox_ == brls::Application::getTopStackView()) {
            brls::Application::popView(brls::ViewAnimation::FADE);
        }
    }

    bool isOnTopView() const {
        return (_loadingBox_ == brls::Application::getTopStackView());
    }

private:
    brls::Dialog* _loadingBox_{nullptr};
    ProgressBarMonitorView* _monitorView_{nullptr};
};

}  // namespace ProgressDisplay
