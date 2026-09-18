#pragma once
#include <array>
#include <set>
#include <td/Types.h>
#include <gui/Application.h>

constexpr size_t SUDOKU_SIZE = 9;

enum class BoardTheme : int
{
    PinkDream = 0,
    HotPink = 1,
    LilacMist = 2,
    CoralReef = 3
};

enum class BackgroundMusic : int
{
    None = 0,
    Focus1 = 1,
    Focus2 = 2,
    Focus3 = 3
};

class SudokuBoard
{
public:
    class Options
    {
    public:
        BoardTheme theme = BoardTheme::PinkDream;
        BackgroundMusic music = BackgroundMusic::None;

        Options()
        {
            auto appProperties = gui::getApplication()->getProperties();
            theme = appProperties->getValue("boardTheme", theme);
            music = appProperties->getValue("backgroundMusic", music);
        }

        void save()
        {
            auto appProperties = gui::getApplication()->getProperties();
            appProperties->setValue("boardTheme", theme);
            appProperties->setValue("backgroundMusic", music);
        }
    };

    struct Cell
    {
        int value = 0;          // 0 = empty, 1-9 = number
        bool isInitial = false;
        std::set<int> notes;
    };

protected:
    std::array<std::array<Cell, SUDOKU_SIZE>, SUDOKU_SIZE> _cells;
    Options _options;

public:
    SudokuBoard()
    {
        clearAll(true);
    }

    int getValue(size_t row, size_t col) const
    {
        if (row < SUDOKU_SIZE && col < SUDOKU_SIZE)
            return _cells[row][col].value;
        return 0;
    }

    const Cell& getCell(size_t row, size_t col) const
    {
        static Cell emptyCell;
        if (row < SUDOKU_SIZE && col < SUDOKU_SIZE)
            return _cells[row][col];
        return emptyCell;
    }

    std::array<std::array<Cell, SUDOKU_SIZE>, SUDOKU_SIZE> getAllCells() const
    {
        return _cells;
    }

    void setCell(size_t row, size_t col, const Cell& cell)
    {
        if (row < SUDOKU_SIZE && col < SUDOKU_SIZE)
            _cells[row][col] = cell;
    }

    bool setValue(size_t row, size_t col, int value, bool isInitial = false)
    {
        if (row >= SUDOKU_SIZE || col >= SUDOKU_SIZE)
            return false;
        if (value < 0 || value > 9)
            return false;

        if (value == 0)
        {
            _cells[row][col].value = 0;
            _cells[row][col].notes.clear();
            if (!isInitial)
                _cells[row][col].isInitial = false;
            return true;
        }

        if (!isInitial && !isValidPlacement(row, col, value))
            return false;

        _cells[row][col].value = value;
        _cells[row][col].isInitial = isInitial;
        _cells[row][col].notes.clear();
        return true;
    }

    void setAllValues(const std::array<std::array<int, SUDOKU_SIZE>, SUDOKU_SIZE>& values,
        bool markAsInitial = false)
    {
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                auto& cell = _cells[row][col];
                cell.value = values[row][col];
                cell.isInitial = markAsInitial;
                cell.notes.clear();
            }
    }

    bool isInitial(size_t row, size_t col) const
    {
        if (row < SUDOKU_SIZE && col < SUDOKU_SIZE)
            return _cells[row][col].isInitial;
        return false;
    }

    void toggleNote(size_t row, size_t col, int value)
    {
        if (row >= SUDOKU_SIZE || col >= SUDOKU_SIZE || value < 1 || value > 9)
            return;
        if (_cells[row][col].value > 0 || _cells[row][col].isInitial)
            return;

        auto& cellNotes = _cells[row][col].notes;
        auto it = cellNotes.find(value);
        if (it != cellNotes.end())
            cellNotes.erase(it);
        else
            cellNotes.insert(value);
    }

    void clearNotes(size_t row, size_t col)
    {
        if (row < SUDOKU_SIZE && col < SUDOKU_SIZE)
            _cells[row][col].notes.clear();
    }

    const std::set<int>& getNotes(size_t row, size_t col) const
    {
        static std::set<int> emptySet;
        if (row >= SUDOKU_SIZE || col >= SUDOKU_SIZE)
            return emptySet;
        return _cells[row][col].notes;
    }

    std::array<std::array<std::set<int>, SUDOKU_SIZE>, SUDOKU_SIZE> getAllNotes() const
    {
        std::array<std::array<std::set<int>, SUDOKU_SIZE>, SUDOKU_SIZE> allNotes{};
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                allNotes[row][col] = _cells[row][col].notes;
        return allNotes;
    }

    void clearAll(bool clearInitial = false)
    {
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                if (clearInitial)
                {
                    _cells[row][col] = Cell{};  // reset everything including isInitial
                }
                else if (!_cells[row][col].isInitial)
                {
                    _cells[row][col].value = 0;
                    _cells[row][col].notes.clear();
                }
            }
    }

    std::array<std::array<int, SUDOKU_SIZE>, SUDOKU_SIZE> getAllValues() const
    {
        std::array<std::array<int, SUDOKU_SIZE>, SUDOKU_SIZE> values{};
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                values[row][col] = _cells[row][col].value;
        return values;
    }

    bool isValidPlacement(size_t row, size_t col, int value) const
    {
        // check row
        for (size_t c = 0; c < SUDOKU_SIZE; ++c)
            if (c != col && _cells[row][c].value == value)
                return false;

        // check column
        for (size_t r = 0; r < SUDOKU_SIZE; ++r)
            if (r != row && _cells[r][col].value == value)
                return false;

        // check 3x3 box
        size_t boxRow = (row / 3) * 3;
        size_t boxCol = (col / 3) * 3;
        for (size_t r = boxRow; r < boxRow + 3; ++r)
            for (size_t c = boxCol; c < boxCol + 3; ++c)
                if ((r != row || c != col) && _cells[r][c].value == value)
                    return false;

        return true;
    }

    // a fully filled board is solved if every cell is non-zero.
    bool isSolved() const
    {
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                if (_cells[row][col].value == 0)
                    return false;
        return true;
    }

    void updateNotes()
    {
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                auto& cell = _cells[row][col];
                cell.notes.clear();

                if (cell.value == 0 && !cell.isInitial)
                    for (int v = 1; v <= 9; ++v)
                        if (isValidPlacement(row, col, v))
                            cell.notes.insert(v);
            }
    }

    Options& getOptions() { return _options; }
    const Options& getOptions() const { return _options; }
};