#pragma once
#include <gui/View.h>
#include <gui/HorizontalLayout.h>
#include <gui/VerticalNavigator.h>
#include <gui/Image.h>
#include "SwitcherView.h"
#include "Difficulty.h"

class MainView : public gui::View
{
protected:
    gui::HorizontalLayout _hl;
    gui::VerticalNavigator _navigator;
    gui::Image _navImg1;
    gui::Image _navImg2;
    SwitcherView _switcherView;

public:
    MainView()
        : _hl(2)
        , _navigator(2, 48, 3.2f)
        , _navImg1(":home")
        , _navImg2(":play")
    {
        setMargins(0, 0, 0, 0);
        _navigator.setItem(0, &_navImg1, "Home");
        _navigator.setItem(1, &_navImg2, "How to Play");
        _hl.append(_navigator);
        _hl.append(_switcherView);
        setLayout(&_hl);
    }

    bool setGameDifficulty(Difficulty difficulty)
    {
        return _switcherView.getViewHome().setGameDifficulty(difficulty);
    }

    ViewHome& getViewHome()
    {
        return _switcherView.getViewHome();
    }

protected:
    bool onChangedSelection(gui::Navigator* pNav) override
    {
        int currSelection = (int)pNav->getCurrentSelection();
        _switcherView.showView(currSelection);
        _switcherView.getViewHome().setFocus();
        return true;
    }
};