#pragma once
#include <gui/Window.h>
#include <gui/Label.h>
#include "MenuBar.h"
#include "ToolBar.h"
#include "MainView.h"
#include "Difficulty.h"

class MainWindow : public gui::Window
{
protected:
    gui::Image _imgOK;
    gui::Image _imgNOK;
    MenuBar _mainMenuBar;
    MainView _mainView;
    ToolBar _toolBar; 

protected:
    void onInitialAppearance() override
    {
        _mainView.setFocus();
    }

    void updateDifficultyIcons(Difficulty d)
    {
        {
            gui::MenuItem* pMenuItem = _mainMenuBar.getItem(20, 0, 0, 20);
            if (pMenuItem)
                pMenuItem->setChecked(d == Difficulty::Easy);
            gui::ToolBarItem* pTBItem = _toolBar.getItem(20, 0, 0, 20);
            if (pTBItem)
                pTBItem->setImage((d == Difficulty::Easy) ? &_imgOK : &_imgNOK);
        }
        {
            gui::MenuItem* pMenuItem = _mainMenuBar.getItem(20, 0, 0, 21);
            if (pMenuItem)
                pMenuItem->setChecked(d == Difficulty::Medium);
            gui::ToolBarItem* pTBItem = _toolBar.getItem(20, 0, 0, 21);
            if (pTBItem)
                pTBItem->setImage((d == Difficulty::Medium) ? &_imgOK : &_imgNOK);
        }
        {
            gui::MenuItem* pMenuItem = _mainMenuBar.getItem(20, 0, 0, 22);
            if (pMenuItem)
                pMenuItem->setChecked(d == Difficulty::Hard);
            gui::ToolBarItem* pTBItem = _toolBar.getItem(20, 0, 0, 22);
            if (pTBItem)
                pTBItem->setImage((d == Difficulty::Hard) ? &_imgOK : &_imgNOK);
        }
    }

    bool onActionItem(gui::ActionItemDescriptor& aiDesc) override
    {
        auto [menuID, firstSubMenuID, lastSubMenuID, actionID] = aiDesc.getIDs();

      
        if (menuID == 20 && firstSubMenuID == 0 && lastSubMenuID == 0)
        {
            switch (actionID)
            {
            case 20:
            {
                if (_mainView.setGameDifficulty(Difficulty::Easy))
                {
                    updateDifficultyIcons(Difficulty::Easy);
                    reDraw();
                }
                _mainView.setFocus();
                return true;
            }
            case 21:
            {
                if (_mainView.setGameDifficulty(Difficulty::Medium))
                {
                    updateDifficultyIcons(Difficulty::Medium);
                    reDraw();
                }
                _mainView.setFocus();
                return true;
            }
            case 22:
            {
                if (_mainView.setGameDifficulty(Difficulty::Hard))
                {
                    updateDifficultyIcons(Difficulty::Hard);
                    reDraw();
                }
                _mainView.setFocus();
                return true;
            }
            }
        }
        return false;
    }

    bool shouldClose() override
    {
        return true;
    }

public:
    MainWindow()
        : gui::Window(gui::Geometry(50, 50, 1200, 800)) 
        , _imgOK(":ok")
        , _imgNOK(":nok")
        , _toolBar(&_mainView.getViewHome()) 
    {
        setTitle("Poncho Girl Sudoku");
        _mainMenuBar.setAsMain(this);
        setToolBar(_toolBar);
        setResizable(true);
        setCentralView(&_mainView);
    }
    ~MainWindow(){}
};