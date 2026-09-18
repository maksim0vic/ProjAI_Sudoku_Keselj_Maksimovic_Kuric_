#pragma once
#include <gui/ViewSwitcher.h>
#include "ViewHome.h"
#include "ViewInfo.h"

class SwitcherView : public gui::ViewSwitcher
{
private:
protected:
    ViewHome _viewHome;
    ViewInfo _viewInfo;

    int _viewPos = 0;

public:
    SwitcherView()
        : gui::ViewSwitcher(2)
    {
        addView(&_viewHome, true);
        addView(&_viewInfo, false);
    }

    ViewHome& getViewHome()
    {
        return _viewHome;
    }

    std::tuple<int, int> showPrev(gui::ActionItem* pAI)
    {
        int nElems = getNoOfViews();

        --_viewPos;
        if (_viewPos < 0)
        {
            _viewPos = 0;
            return { 0, nElems };
        }
        showView(_viewPos);
        if (_viewPos <= 0)
            pAI->disable();
        return { _viewPos, nElems };
    }

    std::tuple<int, int> showNext(gui::ActionItem* pAI)
    {
        int nElems = getNoOfViews();
        ++_viewPos;
        if (_viewPos >= nElems)
        {
            _viewPos = nElems - 1;
            return { 0, nElems };
        }
        showView(_viewPos);

        if (_viewPos + 1 >= nElems)
            pAI->disable();

        return { _viewPos, nElems };
    }
};