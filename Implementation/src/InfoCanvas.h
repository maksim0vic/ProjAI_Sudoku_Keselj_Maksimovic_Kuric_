#pragma once
#include <gui/Image.h>
#include <gui/DrawableString.h>
#include <gui/Shape.h>
#include <td/String.h>

enum class InfoTopic
{
    Row = 0,
    Column = 1,
    Box = 2
};

class InfoCanvas
{
protected:
    gui::Image _rowImg;
    gui::Image _columnImg;
    gui::Image _boxImg;
    gui::Image _ponchoGirl;
    gui::Size _viewSize;
    InfoTopic _currentTopic;

public:
    InfoCanvas()
        : _rowImg(":row")
        , _columnImg(":column")
        , _boxImg(":box")
        , _ponchoGirl(":megaPonchoGirl")
        , _currentTopic(InfoTopic::Row)
    {
    }

    void setTopic(InfoTopic topic)
    {
        _currentTopic = topic;
    }

    InfoTopic getCurrentTopic() const
    {
        return _currentTopic;
    }

    void nextTopic()
    {
        int current = static_cast<int>(_currentTopic);
        current = (current + 1) % 3;
        _currentTopic = static_cast<InfoTopic>(current);
    }

    void previousTopic()
    {
        int current = static_cast<int>(_currentTopic);
        current = (current - 1 + 3) % 3;
        _currentTopic = static_cast<InfoTopic>(current);
    }

    void draw()
    {
        if (_viewSize.width == 0 || _viewSize.height == 0)
            return;
        float padding = 20.0f;
        gui::Image* currentImg = nullptr;
        td::String title;
        td::String instruction;

        switch (_currentTopic)
        {
        case InfoTopic::Row:
            currentImg = &_rowImg;
            title = "Understanding ROWS";
            instruction = "A ROW is a horizontal line of 9 cells.\n\n"
                "Rule: Each row must contain the numbers 1-9 exactly once.\n\n"
                "This means:\n"
                "- No number can appear twice in the same row\n"
                "- Every number from 1 to 9 must appear somewhere in each row\n\n"
                "When solving:\n"
                "1. Look at which numbers are already in the row\n"
                "2. The missing numbers are your candidates\n"
                "3. Check if any candidate can only go in one cell";
            break;

        case InfoTopic::Column:
            currentImg = &_columnImg;
            title = "Understanding COLUMNS";
            instruction = "A COLUMN is a vertical line of 9 cells.\n\n"
                "Rule: Each column must contain the numbers 1-9 exactly once.\n\n"
                "This means:\n"
                "- No number can appear twice in the same column\n"
                "- Every number from 1 to 9 must appear somewhere in each column\n\n"
                "When solving:\n"
                "1. Look at which numbers are already in the column\n"
                "2. The missing numbers are your candidates\n"
                "3. Check if any candidate can only go in one cell";
            break;

        case InfoTopic::Box:
            currentImg = &_boxImg;
            title = "Understanding BOXES";
            instruction = "A BOX is a 3x3 square region. There are 9 boxes total.\n\n"
                "Rule: Each box must contain the numbers 1-9 exactly once.\n\n"
                "This means:\n"
                "- No number can appear twice in the same box\n"
                "- Every number from 1 to 9 must appear somewhere in each box\n\n"
                "When solving:\n"
                "1. Look at which numbers are already in the box\n"
                "2. The missing numbers are your candidates\n"
                "3. Check if any candidate can only go in one cell\n\n"
                "Boxes are numbered 1-9 from left to right, top to bottom.";
            break;
        }

        float leftWidth = _viewSize.width * 0.4f;  
        float rightWidth = _viewSize.width * 0.6f; 

        gui::Rect leftImageRect(padding, padding,
            leftWidth - padding,
            _viewSize.height - padding);

        if (currentImg && currentImg->isOK())
        {
            currentImg->draw(leftImageRect, gui::Image::AspectRatio::Keep,
                td::HAlignment::Center, td::VAlignment::Center);
        }

        
        float rightStartX = leftWidth;
        float messageBoxHeight = _viewSize.height * 0.6f; 
        float ponchoHeight = _viewSize.height * 0.4f;     

        gui::Rect messageBoxRect(rightStartX + padding, padding, _viewSize.width - padding, messageBoxHeight - padding);

        float borderWidth = 3.0f;
        gui::Rect borderRect(messageBoxRect.left - borderWidth,
            messageBoxRect.top - borderWidth,
            messageBoxRect.right + borderWidth,
            messageBoxRect.bottom + borderWidth);
        gui::Shape::drawRect(borderRect, td::ColorID::DeepPink);

       
        gui::Shape::drawRect(messageBoxRect, td::ColorID::LightPink);


        gui::Rect titleRect(messageBoxRect.left + padding, messageBoxRect.top + padding,
            messageBoxRect.right - padding,
            messageBoxRect.top + padding + 50);

        gui::DrawableString::draw(title.c_str(),
            titleRect,
            gui::Font::ID::SystemLargerBold,
            td::ColorID::DeepPink,
            td::TextAlignment::Center,
            td::VAlignment::Top);


        gui::Rect textRect(messageBoxRect.left + padding, titleRect.bottom + padding,
            messageBoxRect.right - padding,
            messageBoxRect.bottom - padding);

        gui::DrawableString::draw(instruction.c_str(),
            textRect,
            gui::Font::ID::SystemSmaller,
            td::ColorID::DeepPink,
            td::TextAlignment::Left,
            td::VAlignment::Top);


        gui::Rect ponchoRect(rightStartX + padding, messageBoxHeight,
            _viewSize.width - padding,
            _viewSize.height - padding);

        if (_ponchoGirl.isOK())
        {
            _ponchoGirl.draw(ponchoRect, gui::Image::AspectRatio::Keep, td::HAlignment::Center, td::VAlignment::Center);
        }
    }

    void updateModelSize(const gui::Size& newSize)
    {
        _viewSize = newSize;
    }
};