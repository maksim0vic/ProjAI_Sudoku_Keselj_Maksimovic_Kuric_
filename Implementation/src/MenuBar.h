#pragma once
#include <gui/MenuBar.h>

class MenuBar : public gui::MenuBar
{
private:
    gui::SubMenu _menuFile;

protected:
    void populateFileMenu()
    {
        auto& items = _menuFile.getItems();
        items[0].initAsQuitAppActionItem(tr("Quit"), "q");       
    }

public:
    MenuBar()
        : gui::MenuBar(1)                      
        , _menuFile(1, tr("File"), 3)         
    {
        populateFileMenu();
        _menus[0] = &_menuFile;
    }
};
