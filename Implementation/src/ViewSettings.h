#pragma once
#include <gui/PopupView.h>
#include <gui/Label.h>
#include <gui/ComboBox.h>
#include <gui/LineEdit.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include "ViewHome.h"

class ViewSettings : public gui::PopupView
{
    ViewSettings() = delete;

protected:
    ViewHome* _pViewHome;
    gui::ToolBar* _pMainTB;
    gui::Image _imgOnPopoverButton;
    gui::Label _lblTheme;
    gui::ComboBox _cmbTheme;
    gui::Label _lblMusic;
    gui::ComboBox _cmbMusic;
    gui::GridLayout _mainLayout;

public:
    ViewSettings(ViewHome* pViewHome, gui::ToolBar* pMainTB)
        : _pViewHome(pViewHome)
        , _pMainTB(pMainTB)
        , _imgOnPopoverButton(":settings")
        , _lblTheme("Board Theme:")
        , _lblMusic("Music:")
        , _mainLayout(2, 2)
    {
        setPopoverButtonImage(&_imgOnPopoverButton);

        _cmbTheme.addItem("Pink Dream");
        _cmbTheme.addItem("Hot Pink");
        _cmbTheme.addItem("Lilac Mist");
        _cmbTheme.addItem("Coral Reef");

        auto& opts = _pViewHome->getBoard().getOptions();
        _cmbTheme.selectIndex((int)opts.theme);

        _cmbMusic.addItem("No Music");
        _cmbMusic.addItem("Focus 1");
        _cmbMusic.addItem("Focus 2");
        _cmbMusic.addItem("Focus 3");
        _cmbMusic.selectIndex((int)opts.music);

        
        _cmbTheme.onChangedSelection([this]() {
            BoardTheme selectedTheme = (BoardTheme)_cmbTheme.getSelectedIndex();
            auto& opts = _pViewHome->getBoard().getOptions();
            opts.theme = selectedTheme;
            opts.save();
            _pViewHome->refreshTheme();

#ifdef MU_WINDOWS
            close(); 
#endif
            });

        _cmbMusic.onChangedSelection([this]() {
            BackgroundMusic selectedMusic = (BackgroundMusic)_cmbMusic.getSelectedIndex();
            auto& opts = _pViewHome->getBoard().getOptions();
            opts.music = selectedMusic;
            opts.save();
            _pViewHome->updateBackgroundMusic();

#ifdef MU_WINDOWS
            close(); 
#endif
            });

        td::UINT2 maxCmbWidth = td::UINT2(_cmbTheme.getWidthToFitLongestItem());
        _cmbTheme.setSizeLimits(maxCmbWidth, gui::Control::Limit::UseAsMin);

        maxCmbWidth = td::UINT2(_cmbMusic.getWidthToFitLongestItem());
        _cmbMusic.setSizeLimits(maxCmbWidth, gui::Control::Limit::UseAsMin);

        gui::GridComposer gc(_mainLayout);
        gc.appendRow(_lblTheme) << _cmbTheme;
        gc.appendRow(_lblMusic) << _cmbMusic;

        setLayout(&_mainLayout);
    }
};