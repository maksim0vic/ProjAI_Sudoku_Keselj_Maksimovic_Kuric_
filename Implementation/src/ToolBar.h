#pragma once
#include <gui/ToolBar.h>
#include <gui/Image.h>
#include <gui/Symbol.h>
#include "ViewSettings.h"

class ViewHome; // forward declaration

class ToolBar : public gui::ToolBar
{
    gui::Image _imgOK;
    gui::Image _imgNOK;
    ViewSettings _viewSettings;

public:
    ToolBar(ViewHome* pViewHome)
        : gui::ToolBar("mainTB", 5)
        , _imgOK(":ok")
        , _imgNOK(":nok")
        , _viewSettings(pViewHome, this)
    {
        addItem(&_viewSettings, 1000, tr("settings"), tr("settingsTT"));
        addSpaceItem();
        addItem(tr("Easy"), &_imgNOK, tr("easyTT"), 20, 0, 0, 20);
        addItem(tr("Medium"), &_imgNOK, tr("mediumTT"), 20, 0, 0, 21);
        addItem(tr("Hard"), &_imgNOK, tr("hardTT"), 20, 0, 0, 22);
    }
};