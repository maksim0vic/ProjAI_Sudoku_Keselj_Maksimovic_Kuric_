#pragma once
#include <gui/Canvas.h>
#include <gui/View.h>
#include <gui/Button.h>
#include <gui/HorizontalLayout.h>
#include <gui/VerticalLayout.h>
#include "InfoCanvas.h"

class InfoCanvasView : public gui::Canvas
{
protected:
    InfoCanvas* _pModel;

protected:
    void onDraw(const gui::Rect& rect) override
    {
        if (_pModel)
            _pModel->draw();
    }

    void onResize(const gui::Size& newSize) override
    {
        if (_pModel)
            _pModel->updateModelSize(newSize);
        gui::Canvas::onResize(newSize);
    }

public:
    InfoCanvasView(): Canvas({}), _pModel(nullptr)
    {
        enableResizeEvent(true);
    }

    void setModel(InfoCanvas* pModel)
    {
        _pModel = pModel;
    }

    void refresh()
    {
        reDraw();
    }
};

class ViewInfo : public gui::View
{
protected:
    InfoCanvas _model;
    InfoCanvasView _canvasView;
    gui::Button _btnPrevious;
    gui::Button _btnNext;
    gui::HorizontalLayout _hlButtons;
    gui::VerticalLayout _vlMain;

protected:
    bool onClick(gui::Button* pBtn) override
    {
        if (pBtn == &_btnPrevious)
        {
            _model.previousTopic();
            _canvasView.refresh();
            return true;
        }
        else if (pBtn == &_btnNext)
        {
            _model.nextTopic();
            _canvasView.refresh();
            return true;
        }

        return false;
    }

public:
    ViewInfo()
        : _btnPrevious("Previous")
        , _btnNext("Next")
        , _hlButtons(2)
        , _vlMain(2)
    {
        _canvasView.setModel(&_model);
        _hlButtons << _btnPrevious << _btnNext;
        _vlMain.append(_canvasView); 
        _vlMain.append(_hlButtons);   
        _vlMain.setMargins(5, 5);
        setLayout(&_vlMain);
    }
};