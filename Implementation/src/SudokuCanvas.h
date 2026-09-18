#pragma once
#include <gui/Canvas.h>
#include <gui/Image.h>
#include <gui/Shape.h>
#include <gui/DrawableString.h>
#include <functional>
#include <set>
#include <array>
#include "SudokuBoard.h"
#include "HintEngine.h"

class SudokuCanvas : public gui::Canvas
{
public:
    std::function<void(int)> onNumberInput;
    std::function<void()>    onInsert;
    std::function<void()>    onSave;
    std::function<void()>    onLoad;
    std::function<void()>    onSolution;
    std::function<void()>    onStartPause;
    std::function<void()>    onNotesToggle;
    std::function<void()>    onHint;
    std::function<void()>    onShowNumber;
    std::function<void()>    onClearAll;
    std::function<void()>    onSelectionChanged;

protected:
    SudokuBoard* _pBoard = nullptr;

    gui::Image _pinkBoard;
    gui::Image _hotPinkBoard;
    gui::Image _lilacBoard;
    gui::Image _coralBoard;
    gui::Size  _boardImgSize;

    gui::Size  _viewSize;
    gui::Rect  _toolbarRect;
    gui::Rect  _boardZone;
    gui::Rect  _rightZone;
    gui::Rect  _statusRect;

    gui::Rect  _boardPlacement;
    gui::Point _ptOrig;
    gui::Size  _cellSize;
    bool       _calcBoardPlacement = true;

    std::array<std::array<gui::Rect, SUDOKU_SIZE>, SUDOKU_SIZE> _cellRects;

    gui::Rect _btnInsertRect;
    gui::Rect _btnSaveRect;
    gui::Rect _btnLoadRect;
    gui::Rect _btnSolutionRect;
    gui::Rect _mistakesRect;

    gui::Rect _btnNum[9];
    gui::Rect _btnClearRect;
    gui::Rect _btnClearAllRect;

    gui::Rect _timerRect;
    gui::Rect _ponchoRect;
    gui::Rect _bubbleRect;
    gui::Rect _btnStartRect;
    gui::Rect _btnNotesRect;
    gui::Rect _btnHintRect;
    gui::Rect _btnShowNumberRect;

    int  _selectedRow = -1;
    int  _selectedCol = -1;
    bool _gameActive  = false;
    bool _insertMode  = false;
    bool _notesMode   = false;
    int  _highlightValue = 0;
    bool _isSolved    = false;

    Hint _activeHint;
    bool _showingHint = false;
    std::set<std::pair<size_t, size_t>> _hintRelatedSet;
    std::set<size_t> _hintRowSet;
    std::set<size_t> _hintColSet;
    std::set<int>    _hintBoxSet;

    gui::Image _gameOverImg;
    bool _showingGameOver = false;

    td::String _timeStr = "00:00";
    td::String _ponchoMessage;
    bool       _showPonchoMessage = false;
    gui::Image _ponchoImg;

    td::String _statusMessage;
    int  _mistakeCount = 0;
    int  _maxMistakes  = 3;

    td::String _startTitle = "Start Game";

    static constexpr td::ColorID kBtnFill    = td::ColorID::DeepPink;
    static constexpr td::ColorID kBtnFillDim = td::ColorID::DarkGray;
    static constexpr td::ColorID kBtnWire    = td::ColorID::White;
    static constexpr td::ColorID kBtnText    = td::ColorID::White;
    static constexpr td::ColorID kPanelFill  = td::ColorID::DeepPink;
    static constexpr td::ColorID kPanelWire  = td::ColorID::White;

    void drawPill(const gui::Rect& r, const char* text, bool enabled = true,
                  gui::Font::ID font = gui::Font::ID::SystemNormal)
    {
        gui::Shape s;
        s.createRoundedRect(r, 6);
        s.drawFill(enabled ? kBtnFill : kBtnFillDim);
        s.drawWire(kBtnWire, 0.8f);
        gui::DrawableString::draw(text, strlen(text), r, font,
            enabled ? kBtnText : td::ColorID::LightGray,
            td::TextAlignment::Center, td::VAlignment::Center);
    }

    gui::Image* getCurrentBoardImage()
    {
        if (!_pBoard) return &_pinkBoard;
        switch (_pBoard->getOptions().theme)
        {
        case BoardTheme::HotPink:   return &_hotPinkBoard;
        case BoardTheme::LilacMist: return &_lilacBoard;
        case BoardTheme::CoralReef: return &_coralBoard;
        default:                    return &_pinkBoard;
        }
    }

    void updateBoardImageSize()
    {
        gui::Image* img = getCurrentBoardImage();
        if (img->isOK())
        {
            img->getSize(_boardImgSize);
        }
    }

    void computeCellRects()
    {
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                gui::CoordType x = _ptOrig.x + col * _cellSize.width;
                gui::CoordType y = _ptOrig.y + row * _cellSize.height;
                _cellRects[row][col] = { x, y, x + _cellSize.width, y + _cellSize.height };
            }
    }

    void calculateBoardPlacement()
    {
        auto* img = getCurrentBoardImage();
        if (!img->isOK() || _boardZone.width() <= 0 || _boardZone.height() <= 0) return;

        updateBoardImageSize();

        float imgAspect  = (float)_boardImgSize.width / (float)_boardImgSize.height;
        float viewAspect = (float)_boardZone.width() / (float)_boardZone.height();

        if (viewAspect > imgAspect)
        {
            float scaledWidth = _boardZone.height() * imgAspect;
            float offsetX = _boardZone.left + (_boardZone.width() - scaledWidth) / 2.0f;
            _boardPlacement = { offsetX, _boardZone.top, offsetX + scaledWidth, _boardZone.bottom };
        }
        else
        {
            float scaledHeight = _boardZone.width() / imgAspect;
            float offsetY = _boardZone.top + (_boardZone.height() - scaledHeight) / 2.0f;
            _boardPlacement = { _boardZone.left, offsetY, _boardZone.right, offsetY + scaledHeight };
        }

        const float marginX = 0.045f, marginY = 0.045f;
        gui::CoordType origX = _boardPlacement.left + marginX * _boardPlacement.width();
        gui::CoordType origY = _boardPlacement.top  + marginY * _boardPlacement.height();
        gui::CoordType rX    = _boardPlacement.right  - marginX * _boardPlacement.width();
        gui::CoordType rY    = _boardPlacement.bottom - marginY * _boardPlacement.height();

        _ptOrig   = { origX, origY };
        _cellSize = { (rX - origX) / SUDOKU_SIZE, (rY - origY) / SUDOKU_SIZE };
        computeCellRects();
        _calcBoardPlacement = false;
    }

    td::Point<td::INT2> getCellCoordinate(const gui::Point& p) const
    {
        if (_cellSize.width <= 0 || _cellSize.height <= 0) return { -1, -1 };
        td::INT2 col = td::INT2((p.x - _ptOrig.x) / _cellSize.width);
        td::INT2 row = td::INT2((p.y - _ptOrig.y) / _cellSize.height);
        if (col < 0 || col >= (td::INT2)SUDOKU_SIZE || row < 0 || row >= (td::INT2)SUDOKU_SIZE)
            return { -1, -1 };
        return { col, row };
    }

    void drawNotes(const gui::Rect& cellRect, const std::set<int>& notes, td::ColorID color)
    {
        if (notes.empty()) return;
        float dim = std::min(cellRect.width(), cellRect.height());
        gui::Font::ID f = dim < 30 ? gui::Font::ID::SystemSmallest
                        : dim < 60 ? gui::Font::ID::SystemSmaller
                                   : gui::Font::ID::SystemNormal;
        float nw = cellRect.width() / 3.0f;
        float nh = cellRect.height() / 3.0f;
        for (int v = 1; v <= 9; ++v)
        {
            if (!notes.count(v)) continue;
            int gr = (v - 1) / 3, gc = (v - 1) % 3;
            gui::Rect r;
            r.left   = cellRect.left + gc * nw;
            r.top    = cellRect.top  + gr * nh;
            r.right  = r.left + nw;
            r.bottom = r.top  + nh;
            char s[2] = { char('0' + v), '\0' };
            gui::DrawableString::draw(s, 1, r, f, color,
                td::TextAlignment::Center, td::VAlignment::Center);
        }
    }

    void drawBoard()
    {
        if (!_pBoard) return;
        auto* img = getCurrentBoardImage();
        if (!img->isOK()) return;
        if (_calcBoardPlacement) calculateBoardPlacement();

        img->draw(_boardZone);

        float dim = std::min(_cellSize.width, _cellSize.height);
        gui::Font::ID mainFont =
            dim < 30 ? gui::Font::ID::SystemSmaller :
            dim < 45 ? gui::Font::ID::SystemNormal :
            dim < 60 ? gui::Font::ID::SystemLarger :
            dim < 75 ? gui::Font::ID::SystemLargerBold :
                       gui::Font::ID::SystemLargest;

        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
        {
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                const gui::Rect& cr = _cellRects[row][col];
                const auto& cell = _pBoard->getCell(row, col);

                if (_showingHint && _activeHint.isValid())
                {
                    bool isTarget  = ((int)row == (int)_activeHint.row && (int)col == (int)_activeHint.col);
                    bool isRelated = _hintRelatedSet.count({ row, col }) > 0;
                    int boxNum = (int)((row / 3) * 3 + col / 3 + 1);
                    bool isRegion = _hintRowSet.count(row) || _hintColSet.count(col) || _hintBoxSet.count(boxNum);

                    if (isTarget)        gui::Shape::drawRect(cr, td::ColorID::Mauve);
                    else if (isRelated)  gui::Shape::drawRect(cr, td::ColorID::Periwinkle);
                    else if (isRegion)   gui::Shape::drawRect(cr, td::ColorID::Plum);
                }

                if (!_insertMode && !_showingHint && _highlightValue > 0 && cell.value == _highlightValue)
                    gui::Shape::drawRect(cr, td::ColorID::HotPink);

                if (_selectedRow == (int)row && _selectedCol == (int)col && !_showingHint)
                    gui::Shape::drawRect(cr, td::ColorID::Pink);

                if (cell.value > 0)
                {
                    char s[2] = { char('0' + cell.value), '\0' };
                    td::ColorID c = cell.isInitial ? td::ColorID::Black : td::ColorID::DeepPink;
                    gui::DrawableString::draw(s, 1, cr, mainFont, c,
                        td::TextAlignment::Center, td::VAlignment::Center);
                }
                else if (!cell.notes.empty())
                {
                    drawNotes(cr, cell.notes, td::ColorID::DeepPink);
                }
            }
        }

        if (_isSolved)
        {
            gui::Rect frame(_ptOrig.x, _ptOrig.y,
                            _ptOrig.x + SUDOKU_SIZE * _cellSize.width,
                            _ptOrig.y + SUDOKU_SIZE * _cellSize.height);
            gui::Shape::drawRect(frame, td::ColorID::Magenta, 5.5f);
        }
    }

    void drawToolbar()
    {
        drawPill(_btnInsertRect,   _insertMode ? "Finish Inserting" : "Insert Sudoku");
        drawPill(_btnSaveRect,     "Save for Later");
        drawPill(_btnLoadRect,     "Load Board");
        drawPill(_btnSolutionRect, "Solution");

        char m[32];
        snprintf(m, sizeof(m), "Mistakes: %d/%d", _mistakeCount, _maxMistakes);
        gui::DrawableString::draw(m, strlen(m), _mistakesRect,
            gui::Font::ID::SystemSmallerBold, td::ColorID::White,
            td::TextAlignment::Right, td::VAlignment::Center);
    }

    void drawNumberPad()
    {
        char lbl[2];
        for (int i = 0; i < 9; ++i)
        {
            lbl[0] = char('1' + i); lbl[1] = '\0';
            drawPill(_btnNum[i], lbl, true, gui::Font::ID::SystemBold);
        }
        drawPill(_btnClearRect,    "Clear",     true, gui::Font::ID::SystemSmaller);
        drawPill(_btnClearAllRect, "Clear All", true, gui::Font::ID::SystemSmaller);
    }

    const char* _startButtonTitle() const { return _startTitle.c_str(); }

    void drawRightColumn()
    {
        gui::Shape tp; tp.createRoundedRect(_timerRect, 8);
        tp.drawFill(kPanelFill); tp.drawWire(kPanelWire, 1.0f);
        gui::DrawableString::draw(_timeStr, _timerRect,
            gui::Font::ID::SystemLargerBold, td::ColorID::White,
            td::TextAlignment::Center, td::VAlignment::Center);

        if (_ponchoRect.height() > 0 && _ponchoImg.isOK())
        {
            _ponchoImg.draw(_ponchoRect, gui::Image::AspectRatio::Keep,
                td::HAlignment::Center, td::VAlignment::Center);
        }

        if (_bubbleRect.height() > 0 && _showPonchoMessage && !_ponchoMessage.isEmpty())
        {
            gui::Shape bubble; bubble.createRoundedRect(_bubbleRect, 10);
            bubble.drawFillAndWire(td::ColorID::Pink, td::ColorID::HotPink, 2.0f);
            gui::DrawableString ds(_ponchoMessage);
            gui::Rect inner(_bubbleRect.left + 8, _bubbleRect.top + 6,
                            _bubbleRect.right - 8, _bubbleRect.bottom - 6);
            ds.draw(inner, gui::Font::ID::SystemSmaller, td::ColorID::DeepPink,
                td::TextAlignment::Center, td::VAlignment::Center,
                td::TextEllipsize::None);
        }

        drawPill(_btnStartRect,      _startButtonTitle(), true, gui::Font::ID::SystemNormal);
        drawPill(_btnNotesRect,      _notesMode ? "Notes ON" : "Notes OFF", true,
                 gui::Font::ID::SystemNormal);
        drawPill(_btnHintRect,       "Get Hint",     true, gui::Font::ID::SystemNormal);
        drawPill(_btnShowNumberRect, "Show Number",  true, gui::Font::ID::SystemNormal);
    }

    void drawStatus()
    {
        gui::DrawableString::draw(_statusMessage, _statusRect,
            gui::Font::ID::SystemSmaller, td::ColorID::White,
            td::TextAlignment::Left, td::VAlignment::Center);
    }

    void recomputeLayout()
    {
        if (_viewSize.width <= 0 || _viewSize.height <= 0) return;

        const gui::CoordType PAD = 8;
        const gui::CoordType TOOLBAR_H = 40;
        const gui::CoordType STATUS_H  = 24;
        const gui::CoordType RIGHT_W   = 260;

        _toolbarRect = { PAD, PAD, _viewSize.width - PAD, PAD + TOOLBAR_H };

        _statusRect  = { PAD, _viewSize.height - PAD - STATUS_H,
                         _viewSize.width - PAD, _viewSize.height - PAD };

        gui::CoordType midTop    = _toolbarRect.bottom + PAD;
        gui::CoordType midBottom = _statusRect.top - PAD;

        _rightZone = { _viewSize.width - PAD - RIGHT_W, midTop,
                       _viewSize.width - PAD,            midBottom };

        _boardZone = { PAD, midTop, _rightZone.left - PAD, midBottom };

        gui::CoordType bx = _toolbarRect.left;
        gui::CoordType by = _toolbarRect.top;
        gui::CoordType bh = TOOLBAR_H;
        gui::CoordType bw = 130;
        _btnInsertRect   = { bx, by, bx + bw, by + bh }; bx += bw + 8;
        _btnSaveRect     = { bx, by, bx + bw, by + bh }; bx += bw + 8;
        _btnLoadRect     = { bx, by, bx + bw, by + bh }; bx += bw + 8;
        _btnSolutionRect = { bx, by, bx + bw, by + bh };
        _mistakesRect    = { _toolbarRect.left + bw * 4 + 24, by, _toolbarRect.right, by + bh };

        const gui::CoordType padH = 56;
        gui::Rect boardTop = _boardZone;
        boardTop.bottom -= (padH + 6);

        gui::Rect padZone = { _boardZone.left, _boardZone.bottom - padH,
                              _boardZone.right, _boardZone.bottom };

        const gui::CoordType gap = 4;
        int totalBtns = 11;
        gui::CoordType totalW = padZone.width() - gap * (totalBtns - 1);
        gui::CoordType btnW   = totalW / totalBtns;
        gui::CoordType px     = padZone.left;
        for (int i = 0; i < 9; ++i)
        {
            _btnNum[i] = { px, padZone.top, px + btnW, padZone.bottom };
            px += btnW + gap;
        }
        _btnClearRect    = { px, padZone.top, px + btnW, padZone.bottom }; px += btnW + gap;
        _btnClearAllRect = { px, padZone.top, px + btnW, padZone.bottom };

        _boardZone = boardTop;
        _calcBoardPlacement = true;

        gui::CoordType ry = _rightZone.top;
        const gui::CoordType rowGap = 8;

        const gui::CoordType btnH   = 44;
        const gui::CoordType stackH = btnH * 4 + rowGap * 3; // Start/Notes/Hint/ShowNumber

        const gui::CoordType timerH        = 60; 
        const gui::CoordType ponchoHNom    = 110;
        const gui::CoordType bubbleHNom    = 100;

        gui::CoordType availableForTop = _rightZone.height() - stackH - rowGap;
        gui::CoordType remaining = availableForTop - timerH - rowGap;

        gui::CoordType ponchoH{0};
        gui::CoordType bubbleH{0};
        if (remaining > 20)
        {
            if (remaining - rowGap > 0)
            {
                gui::CoordType remAfterGap = remaining - rowGap;
                ponchoH = remAfterGap * (ponchoHNom / (float)(ponchoHNom + bubbleHNom));
                ponchoH = std::min(ponchoH, ponchoHNom);
                bubbleH = std::min(remAfterGap - ponchoH, bubbleHNom);
            }
            else
            {
                ponchoH = std::min(remaining, ponchoHNom);
            }
        }

        _timerRect = { _rightZone.left, ry, _rightZone.right, ry + timerH };
        ry = _timerRect.bottom + rowGap;

        _ponchoRect = { _rightZone.left, ry, _rightZone.right, ry + ponchoH };
        if (ponchoH > 0) ry = _ponchoRect.bottom + rowGap;

        _bubbleRect = { _rightZone.left, ry, _rightZone.right, ry + bubbleH };

        const gui::CoordType startY = _rightZone.bottom - stackH;

        _btnStartRect      = { _rightZone.left, startY,                       _rightZone.right, startY + btnH };
        _btnNotesRect      = { _rightZone.left, startY + (btnH + rowGap),     _rightZone.right, startY + (btnH + rowGap) + btnH };
        _btnHintRect       = { _rightZone.left, startY + (btnH + rowGap) * 2, _rightZone.right, startY + (btnH + rowGap) * 2 + btnH };
        _btnShowNumberRect = { _rightZone.left, startY + (btnH + rowGap) * 3, _rightZone.right, startY + (btnH + rowGap) * 3 + btnH };
    }

    void onResize(const gui::Size& newSize) override
    {
        _viewSize = newSize;
        recomputeLayout();
        gui::Canvas::onResize(newSize);
        reDraw();
    }

    void onDraw(const gui::Rect&) override
    {
        if (!_pBoard) return;

        if (_calcBoardPlacement) calculateBoardPlacement();

        drawToolbar();
        drawBoard();
        drawNumberPad();
        drawRightColumn();
        drawStatus();

        if (_showingGameOver && _gameOverImg.isOK())
        {
            gui::Size imgSize; _gameOverImg.getSize(imgSize);
            gui::CoordType cx = (_boardPlacement.left + _boardPlacement.right) / 2.0f;
            gui::CoordType cy = (_boardPlacement.top + _boardPlacement.bottom) / 2.0f;
            gui::Point pos;
            pos.x = cx - imgSize.width  / 2.0f;
            pos.y = cy - imgSize.height / 2.0f;
            _gameOverImg.draw(gui::Rect(pos, imgSize));
        }
    }

    void onPrimaryButtonPressed(const gui::InputDevice& inputDevice) override
    {
        setFocus();
        const gui::Point p = inputDevice.getModelPoint();

        if (_btnInsertRect.contains(p))   { if (onInsert)   onInsert();   return; }
        if (_btnSaveRect.contains(p))     { if (onSave)     onSave();     return; }
        if (_btnLoadRect.contains(p))     { if (onLoad)     onLoad();     return; }
        if (_btnSolutionRect.contains(p)) { if (onSolution) onSolution(); return; }

        for (int i = 0; i < 9; ++i)
        {
            if (_btnNum[i].contains(p))
            {
                if (onNumberInput) onNumberInput(i + 1);
                return;
            }
        }
        if (_btnClearRect.contains(p))    { if (onNumberInput) onNumberInput(0); return; }
        if (_btnClearAllRect.contains(p)) { if (onClearAll)    onClearAll();     return; }

        if (_btnStartRect.contains(p))      { if (onStartPause) onStartPause(); return; }
        if (_btnNotesRect.contains(p))
        {
            _notesMode = !_notesMode;
            if (onNotesToggle) onNotesToggle();
            reDraw();
            return;
        }
        if (_btnHintRect.contains(p))       { if (onHint)       onHint();       return; }
        if (_btnShowNumberRect.contains(p)) { if (onShowNumber) onShowNumber(); return; }

        if (_showingHint) clearHintVisualization();

        auto coord = getCellCoordinate(p);
        if (coord.x < 0 || coord.y < 0)
        {
            _selectedRow = -1;
            _selectedCol = -1;
            _highlightValue = 0;
        }
        else
        {
            _selectedRow = coord.y;
            _selectedCol = coord.x;
            _highlightValue = _pBoard->getCell(_selectedRow, _selectedCol).value;
        }
        if (onSelectionChanged) onSelectionChanged();
        reDraw();
    }

    bool onKeyPressed(const gui::Key& key) override
    {
        if (_selectedRow < 0 || _selectedCol < 0) return gui::Canvas::onKeyPressed(key);
        if (!_gameActive && !_insertMode) return gui::Canvas::onKeyPressed(key);

        if (key.isASCII())
        {
            if (_showingHint) clearHintVisualization();

            char c = key.getChar();
            if (c >= '1' && c <= '9') { if (onNumberInput) onNumberInput(c - '0'); return true; }
            if (c == '0' || c == 127 || c == 8) { if (onNumberInput) onNumberInput(0); return true; }
        }

        return gui::Canvas::onKeyPressed(key);
    }

public:
    SudokuCanvas()
        : Canvas({ gui::InputDevice::Event::PrimaryClicks, gui::InputDevice::Event::Keyboard })
        , _pinkBoard(":pinkboard")
        , _hotPinkBoard(":hotpinkboard")
        , _lilacBoard(":lilacboard")
        , _coralBoard(":coralboard")
        , _ponchoImg(":megaPonchoGirl")
        , _gameOverImg(":gameover")
    {
        gui::Size s;
        if (_pinkBoard.isOK()) _pinkBoard.getSize(s);
        _boardImgSize = s;
        enableResizeEvent(true);
    }

    void setBoard(SudokuBoard* b) { _pBoard = b; _isSolved = false; }

    void refresh() { reDraw(); }

    void themeChanged()
    {
        _calcBoardPlacement = true;
        reDraw();
    }

    void setGameActive(bool a)
    {
        _gameActive = a;
        if (!a)
        {
            _selectedRow = -1; _selectedCol = -1; _highlightValue = 0;
            clearHintVisualization();
        }
        reDraw();
    }

    void setInsertMode(bool im) { _insertMode = im; reDraw(); }
    void setNotesMode(bool on)  { _notesMode = on;  reDraw(); }
    bool isNotesMode() const    { return _notesMode; }
    bool isGameActive() const   { return _gameActive; }
    bool isInsertMode() const   { return _insertMode; }
    bool isSolved() const       { return _isSolved; }
    void setSolved(bool s)      { _isSolved = s; reDraw(); }

    void setStartButtonTitle(const char* t)    { _startTitle = t; reDraw(); }
    void setStatusMessage(const td::String& m) { _statusMessage = m; reDraw(); }
    void setStatusMessage(const char* m)       { _statusMessage = m; reDraw(); }
    void setMistakeCount(int c, int mx = 3)    { _mistakeCount = c; _maxMistakes = mx; reDraw(); }

    std::pair<int,int> getSelectedCell() const { return { _selectedRow, _selectedCol }; }

    bool setSelectedCellValue(int value)
    {
        if (!_pBoard || _selectedRow < 0 || _selectedCol < 0) return false;
        const auto& cell = _pBoard->getCell(_selectedRow, _selectedCol);
        if (!_insertMode && cell.isInitial) return false;

        if (_insertMode)
        {
            if (!_pBoard->setValue(_selectedRow, _selectedCol, value, false)) return false;
            _highlightValue = value;
            _isSolved = _pBoard->isSolved();
            reDraw();
            return true;
        }
        if (!_pBoard->setValue(_selectedRow, _selectedCol, value)) return false;
        _pBoard->clearNotes(_selectedRow, _selectedCol);
        _highlightValue = value;
        _isSolved = _pBoard->isSolved();
        reDraw();
        return true;
    }

    bool toggleNoteForSelectedCell(int v)
    {
        if (!_pBoard || _selectedRow < 0 || _selectedCol < 0) return false;
        const auto& cell = _pBoard->getCell(_selectedRow, _selectedCol);
        if (cell.value > 0 || cell.isInitial || v < 1 || v > 9) return false;
        _pBoard->toggleNote(_selectedRow, _selectedCol, v);
        reDraw();
        return true;
    }

    bool clearSelectedCell()
    {
        if (!_pBoard || _selectedRow < 0 || _selectedCol < 0) return false;
        if (!_insertMode && _pBoard->getCell(_selectedRow, _selectedCol).isInitial) return false;
        _pBoard->setValue(_selectedRow, _selectedCol, 0);
        _highlightValue = 0;
        _isSolved = false;
        reDraw();
        return true;
    }

    bool clearNotesInSelectedCell()
    {
        if (!_pBoard || _selectedRow < 0 || _selectedCol < 0) return false;
        _pBoard->clearNotes(_selectedRow, _selectedCol);
        reDraw();
        return true;
    }

    void clearSelection()
    {
        _selectedRow = -1; _selectedCol = -1; _highlightValue = 0;
        reDraw();
    }

    void showHintVisualization(const Hint& h)
    {
        _activeHint = h; _showingHint = true;
        _hintRelatedSet.clear();
        for (auto& rc : h.relatedCells) _hintRelatedSet.insert({ rc.first, rc.second });
        _hintRowSet = std::set<size_t>(h.highlightRows.begin(), h.highlightRows.end());
        _hintColSet = std::set<size_t>(h.highlightCols.begin(), h.highlightCols.end());
        _hintBoxSet = std::set<int>(h.highlightBoxes.begin(), h.highlightBoxes.end());
        reDraw();
    }

    void clearHintVisualization()
    {
        _showingHint = false; _activeHint = Hint{};
        _hintRelatedSet.clear(); _hintRowSet.clear(); _hintColSet.clear(); _hintBoxSet.clear();
        reDraw();
    }

    bool isShowingHint() const { return _showingHint; }

    void showGameOverPopup() { _showingGameOver = true;  reDraw(); }
    void hideGameOverPopup() { _showingGameOver = false; reDraw(); }

    void setTimeString(const td::String& s) { _timeStr = s; reDraw(); }
    void setPonchoMessage(const td::String& m) { _ponchoMessage = m; _showPonchoMessage = true; reDraw(); }
    void clearPonchoMessage() { _ponchoMessage.clean(); _showPonchoMessage = false; reDraw(); }
};
