#pragma once
#include "SudokuBoard.h"
#include <set>

class BacktrackingSolver
{
protected:
    SudokuBoard* _pBoard;

    // get possible values for a cell
    std::set<int> getPossibleValues(int row, int col) const
    {
        std::set<int> possible;
        if (_pBoard->getValue(row, col) != 0)
            return possible; // cell already filled

        for (int value = 1; value <= 9; ++value)
        {
            if (_pBoard->isValidPlacement(row, col, value))
                possible.insert(value);
        }

        return possible;
    }

    // MRV: find the empty cell with fewest possible values
    bool findBestCell(int& bestRow, int& bestCol) const
    {
        int minPossible = 10;
        bestRow = -1;
        bestCol = -1;

        for (int row = 0; row < SUDOKU_SIZE; ++row)
        {
            for (int col = 0; col < SUDOKU_SIZE; ++col)
            {
                if (_pBoard->getValue(row, col) == 0)
                {
                    auto possible = getPossibleValues(row, col);
                    int count = possible.size();

                    if (count == 0)
                        return false; // dead end

                    if (count < minPossible)
                    {
                        minPossible = count;
                        bestRow = row;
                        bestCol = col;

                        if (count == 1)
                            return true;
                    }
                }
            }
        }
        return (bestRow != -1);
    }

    bool solveBacktrack()
    {
        int row, col;

        if (!findBestCell(row, col))
        {
            // no empty cell found via MRV — verify board is fully filled
            for (int r = 0; r < SUDOKU_SIZE; ++r)
                for (int c = 0; c < SUDOKU_SIZE; ++c)
                    if (_pBoard->getValue(r, c) == 0)
                        return false;
            return true;
        }

        for (int value : getPossibleValues(row, col))
        {
            _pBoard->setValue(row, col, value, false);

            if (solveBacktrack())
                return true;

            _pBoard->setValue(row, col, 0, false);
        }

        return false;
    }

    int countSolutions(int limit = 2)
    {
        int row, col;
        if (!findBestCell(row, col))
        {
            for (int r = 0; r < SUDOKU_SIZE; ++r)
                for (int c = 0; c < SUDOKU_SIZE; ++c)
                    if (_pBoard->getValue(r, c) == 0)
                        return 0;
            return 1;
        }
        int count = 0;
        auto possible = getPossibleValues(row, col);

        for (int value : possible)
        {
            _pBoard->setValue(row, col, value, false);
            count += countSolutions(limit - count);
            _pBoard->setValue(row, col, 0, false);
            if (count >= limit)
                return count;
        }

        return count;
    }

    // save and restore board state around an operation that mutates the board
    template<typename Func>
    auto withSavedState(Func&& fn) -> decltype(fn())
    {
        auto savedState = _pBoard->getAllCells();
        auto result = fn();
        for (int r = 0; r < SUDOKU_SIZE; ++r)
            for (int c = 0; c < SUDOKU_SIZE; ++c)
                _pBoard->setCell(r, c, savedState[r][c]);
        return result;
    }

public:
    BacktrackingSolver() : _pBoard(nullptr) {}
    BacktrackingSolver(SudokuBoard* pBoard) : _pBoard(pBoard) {}

    void setBoard(SudokuBoard* pBoard)
    {
        _pBoard = pBoard;
    }

    bool solve()
    {
        if (!_pBoard)
            return false;
        return solveBacktrack();
    }

    bool hasUniqueSolution()
    {
        if (!_pBoard)
            return false;
        return withSavedState([this] { return countSolutions(2) == 1; });
    }

    bool isSolvable()
    {
        if (!_pBoard)
            return false;
        return withSavedState([this] { return solve(); });
    }

    bool getHint(int& hintRow, int& hintCol, int& hintValue)
    {
        if (!_pBoard)
            return false;

        // First pass: look for a naked single (only one possible value)
        for (int row = 0; row < SUDOKU_SIZE; ++row)
        {
            for (int col = 0; col < SUDOKU_SIZE; ++col)
            {
                if (_pBoard->getValue(row, col) == 0)
                {
                    auto possible = getPossibleValues(row, col);
                    if (possible.size() == 1)
                    {
                        hintRow = row;
                        hintCol = col;
                        hintValue = *possible.begin();
                        return true;
                    }
                }
            }
        }

        // Second pass: solve and return the value for the first empty cell
        for (int row = 0; row < SUDOKU_SIZE; ++row)
        {
            for (int col = 0; col < SUDOKU_SIZE; ++col)
            {
                if (_pBoard->getValue(row, col) == 0 && !getPossibleValues(row, col).empty())
                {
                    return withSavedState([&]() -> bool
                        {
                            if (solve())
                            {
                                hintRow = row;
                                hintCol = col;
                                hintValue = _pBoard->getValue(row, col);
                                return true;
                            }
                            return false;
                        });
                }
            }
        }
        return false;
    }
};