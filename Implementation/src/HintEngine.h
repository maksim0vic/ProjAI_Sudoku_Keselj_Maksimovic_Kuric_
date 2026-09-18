#pragma once
#include "SudokuBoard.h"
#include "BacktrackingSolver.h"
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <algorithm>

struct Hint
{
    size_t row;
    size_t col;
    int value;
    std::string technique;
    std::string explanation;
    std::vector<std::pair<size_t, size_t>> relatedCells;  
    std::vector<size_t> highlightRows;                     
    std::vector<size_t> highlightCols;                     
    std::vector<int> highlightBoxes;                       

    Hint() : row(static_cast<size_t>(-1)), col(static_cast<size_t>(-1)), value(0) {}

    bool isValid() const { return row != static_cast<size_t>(-1) && col != static_cast<size_t>(-1) && value > 0; }
};

class HintEngine
{
private:
    SudokuBoard* _pBoard;

	//possible values for a cell
    std::set<int> getCandidates(size_t row, size_t col) const
    {
        std::set<int> candidates;

        if (_pBoard->getValue(row, col) != 0)
            return candidates;

        for (int value = 1; value <= 9; ++value)
        {
            if (_pBoard->isValidPlacement(row, col, value))
                candidates.insert(value);
        }

        return candidates;
    }

    std::string cellName(size_t row, size_t col) const
    {
        std::ostringstream oss;
        oss << "R" << (row + 1) << "C" << (col + 1);
        return oss.str();
    }

    int getBoxNumber(size_t row, size_t col) const
    {
        return (row / 3) * 3 + (col / 3) + 1;
    }

    // TECHNIQUE 1: Naked Single
    // Only one possible value for a cell
    Hint findNakedSingle() const
    {
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
        {
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                if (_pBoard->getValue(row, col) == 0)
                {
                    auto candidates = getCandidates(row, col);

                    if (candidates.size() == 1)
                    {
                        Hint hint;
                        hint.row = row;
                        hint.col = col;
                        hint.value = *candidates.begin();
                        hint.technique = "Naked Single";

                        std::ostringstream oss;
                        oss << "[Naked Single] Place " << hint.value << " in " << cellName(row, col)
                            << " - it's the only possible value here.";
                        hint.explanation = oss.str();


                        hint.highlightRows.push_back(row);
                        hint.highlightCols.push_back(col);
                        hint.highlightBoxes.push_back(getBoxNumber(row, col));

                        return hint;
                    }
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 2: Hidden Single in Row
    Hint findHiddenSingleInRow() const
    {
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
        {
            for (int value = 1; value <= 9; ++value)
            {
                
                bool found = false;
                for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                {
                    if (_pBoard->getValue(row, c) == value)
                    {
                        found = true;
                        break;
                    }
                }
                if (found) continue;

                std::vector<size_t> possibleCols;
                for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                {
                    if (_pBoard->getValue(row, col) == 0 &&
                        _pBoard->isValidPlacement(row, col, value))
                    {
                        possibleCols.push_back(col);
                    }
                }

                if (possibleCols.size() == 1)
                {
                    Hint hint;
                    hint.row = row;
                    hint.col = possibleCols[0];
                    hint.value = value;
                    hint.technique = "Hidden Single (Row)";

                    std::ostringstream oss;
                    oss << "[Hidden Single] Place " << value << " in " << cellName(row, possibleCols[0])
                        << " - only spot for " << value << " in row " << (row + 1) << ".";
                    hint.explanation = oss.str();

                    hint.highlightRows.push_back(row);

                    return hint;
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 3: Hidden Single in Column
    Hint findHiddenSingleInColumn() const
    {
        for (size_t col = 0; col < SUDOKU_SIZE; ++col)
        {
            for (int value = 1; value <= 9; ++value)
            {

                bool found = false;
                for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                {
                    if (_pBoard->getValue(r, col) == value)
                    {
                        found = true;
                        break;
                    }
                }
                if (found) continue;

                std::vector<size_t> possibleRows;
                for (size_t row = 0; row < SUDOKU_SIZE; ++row)
                {
                    if (_pBoard->getValue(row, col) == 0 &&
                        _pBoard->isValidPlacement(row, col, value))
                    {
                        possibleRows.push_back(row);
                    }
                }


                if (possibleRows.size() == 1)
                {
                    Hint hint;
                    hint.row = possibleRows[0];
                    hint.col = col;
                    hint.value = value;
                    hint.technique = "Hidden Single (Column)";

                    std::ostringstream oss;
                    oss << "[Hidden Single] Place " << value << " in " << cellName(possibleRows[0], col)
                        << " - only spot for " << value << " in column " << (col + 1) << ".";
                    hint.explanation = oss.str();

                    hint.highlightCols.push_back(col);

                    return hint;
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 4: Hidden Single in Box
    Hint findHiddenSingleInBox() const
    {
        for (size_t boxRow = 0; boxRow < 3; ++boxRow)
        {
            for (size_t boxCol = 0; boxCol < 3; ++boxCol)
            {
                size_t startRow = boxRow * 3;
                size_t startCol = boxCol * 3;

                for (int value = 1; value <= 9; ++value)
                {
           
                    bool found = false;
                    for (size_t r = startRow; r < startRow + 3; ++r)
                    {
                        for (size_t c = startCol; c < startCol + 3; ++c)
                        {
                            if (_pBoard->getValue(r, c) == value)
                            {
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                    if (found) continue;

            
                    std::vector<std::pair<size_t, size_t>> possibleCells;
                    for (size_t r = startRow; r < startRow + 3; ++r)
                    {
                        for (size_t c = startCol; c < startCol + 3; ++c)
                        {
                            if (_pBoard->getValue(r, c) == 0 &&
                                _pBoard->isValidPlacement(r, c, value))
                            {
                                possibleCells.push_back({ r, c });
                            }
                        }
                    }


                    if (possibleCells.size() == 1)
                    {
                        Hint hint;
                        hint.row = possibleCells[0].first;
                        hint.col = possibleCells[0].second;
                        hint.value = value;
                        hint.technique = "Hidden Single (Box)";

                        int boxNum = getBoxNumber(hint.row, hint.col);

                        std::ostringstream oss;
                        oss << "[Hidden Single] Place " << value << " in " << cellName(hint.row, hint.col)
                            << " - only spot for " << value << " in box " << boxNum << ".";
                        hint.explanation = oss.str();

   
                        hint.highlightBoxes.push_back(boxNum);

                        return hint;
                    }
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 5: Naked Pair
    // Two cells in a row/col/box that both have exactly the same 2 candidates
    Hint findNakedPair() const
    {

        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
        {
            std::map<std::set<int>, std::vector<size_t>> candidateMap;

            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                if (_pBoard->getValue(row, col) == 0)
                {
                    auto cands = getCandidates(row, col);
                    if (cands.size() == 2)
                    {
                        candidateMap[cands].push_back(col);
                    }
                }
            }

            for (const auto& pair : candidateMap)
            {
                if (pair.second.size() == 2)
                {
           
                    const auto& nakedPairValues = pair.first;

                    for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                    {
                        if (_pBoard->getValue(row, col) == 0 &&
                            col != pair.second[0] && col != pair.second[1])
                        {
                            auto cands = getCandidates(row, col);
                            auto originalSize = cands.size();

        
                            for (int val : nakedPairValues)
                            {
                                cands.erase(val);
                            }

                            if (cands.size() == 1 && originalSize > 1)
                            {
                                Hint hint;
                                hint.row = row;
                                hint.col = col;
                                hint.value = *cands.begin();
                                hint.technique = "Naked Pair";

                                std::ostringstream oss;
                                oss << "[Naked Pair] Place " << hint.value << " in " << cellName(row, col)
                                    << " - cells " << cellName(row, pair.second[0]) << " and "
                                    << cellName(row, pair.second[1]) << " lock values {";
                                bool first = true;
                                for (int v : nakedPairValues)
                                {
                                    if (!first) oss << ",";
                                    oss << v;
                                    first = false;
                                }
                                oss << "} in row " << (row + 1) << ".";
                                hint.explanation = oss.str();

                           
                                hint.relatedCells.push_back({ row, pair.second[0] });
                                hint.relatedCells.push_back({ row, pair.second[1] });

                        
                                hint.highlightRows.push_back(row);

                                return hint;
                            }
                        }
                    }
                }
            }
        }


        for (size_t col = 0; col < SUDOKU_SIZE; ++col)
        {
            std::map<std::set<int>, std::vector<size_t>> candidateMap;

            for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            {
                if (_pBoard->getValue(row, col) == 0)
                {
                    auto cands = getCandidates(row, col);
                    if (cands.size() == 2)
                    {
                        candidateMap[cands].push_back(row);
                    }
                }
            }

            for (const auto& pair : candidateMap)
            {
                if (pair.second.size() == 2)
                {
                    const auto& nakedPairValues = pair.first;

                    for (size_t row = 0; row < SUDOKU_SIZE; ++row)
                    {
                        if (_pBoard->getValue(row, col) == 0 &&
                            row != pair.second[0] && row != pair.second[1])
                        {
                            auto cands = getCandidates(row, col);
                            auto originalSize = cands.size();

                            for (int val : nakedPairValues)
                            {
                                cands.erase(val);
                            }

                            if (cands.size() == 1 && originalSize > 1)
                            {
                                Hint hint;
                                hint.row = row;
                                hint.col = col;
                                hint.value = *cands.begin();
                                hint.technique = "Naked Pair";

                                std::ostringstream oss;
                                oss << "[Naked Pair] Place " << hint.value << " in " << cellName(row, col)
                                    << " - cells " << cellName(pair.second[0], col) << " and "
                                    << cellName(pair.second[1], col) << " lock values {";
                                bool first = true;
                                for (int v : nakedPairValues)
                                {
                                    if (!first) oss << ",";
                                    oss << v;
                                    first = false;
                                }
                                oss << "} in column " << (col + 1) << ".";
                                hint.explanation = oss.str();


                                hint.relatedCells.push_back({ pair.second[0], col });
                                hint.relatedCells.push_back({ pair.second[1], col });

                         
                                hint.highlightCols.push_back(col);

                                return hint;
                            }
                        }
                    }
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 6: Naked Triples
    // Three cells in a row/col/box that share exactly 3 candidates between them
    Hint findNakedTriples() const
    {

        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
        {
            std::vector<std::pair<size_t, std::set<int>>> cellCandidates;

            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                if (_pBoard->getValue(row, col) == 0)
                {
                    auto cands = getCandidates(row, col);
                    if (cands.size() >= 2 && cands.size() <= 3)
                    {
                        cellCandidates.push_back({ col, cands });
                    }
                }
            }

            for (size_t i = 0; i < cellCandidates.size(); ++i)
            {
                for (size_t j = i + 1; j < cellCandidates.size(); ++j)
                {
                    for (size_t k = j + 1; k < cellCandidates.size(); ++k)
                    {
                        std::set<int> unionSet = cellCandidates[i].second;
                        unionSet.insert(cellCandidates[j].second.begin(), cellCandidates[j].second.end());
                        unionSet.insert(cellCandidates[k].second.begin(), cellCandidates[k].second.end());

                  
                        if (unionSet.size() == 3)
                        {
                            std::vector<size_t> tripleCols = {
                                cellCandidates[i].first,
                                cellCandidates[j].first,
                                cellCandidates[k].first
                            };

                            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                            {
                                if (_pBoard->getValue(row, col) == 0 &&
                                    col != tripleCols[0] && col != tripleCols[1] && col != tripleCols[2])
                                {
                                    auto cands = getCandidates(row, col);
                                    auto originalSize = cands.size();

                                    for (int val : unionSet)
                                    {
                                        cands.erase(val);
                                    }

                                    if (cands.size() == 1 && originalSize > 1)
                                    {
                                        Hint hint;
                                        hint.row = row;
                                        hint.col = col;
                                        hint.value = *cands.begin();
                                        hint.technique = "Naked Triple";

                                        std::ostringstream oss;
                                        oss << "[Naked Triple] Place " << hint.value << " in " << cellName(row, col)
                                            << " - cells " << cellName(row, tripleCols[0]) << ", "
                                            << cellName(row, tripleCols[1]) << ", " << cellName(row, tripleCols[2])
                                            << " lock values {";
                                        bool first = true;
                                        for (int v : unionSet)
                                        {
                                            if (!first) oss << ",";
                                            oss << v;
                                            first = false;
                                        }
                                        oss << "} in row " << (row + 1) << ".";
                                        hint.explanation = oss.str();

                                  
                                        hint.relatedCells.push_back({ row, tripleCols[0] });
                                        hint.relatedCells.push_back({ row, tripleCols[1] });
                                        hint.relatedCells.push_back({ row, tripleCols[2] });
                                        hint.highlightRows.push_back(row);

                                        return hint;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        
        for (size_t col = 0; col < SUDOKU_SIZE; ++col)
        {
            std::vector<std::pair<size_t, std::set<int>>> cellCandidates;

            for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            {
                if (_pBoard->getValue(row, col) == 0)
                {
                    auto cands = getCandidates(row, col);
                    if (cands.size() >= 2 && cands.size() <= 3)
                    {
                        cellCandidates.push_back({ row, cands });
                    }
                }
            }

           
            for (size_t i = 0; i < cellCandidates.size(); ++i)
            {
                for (size_t j = i + 1; j < cellCandidates.size(); ++j)
                {
                    for (size_t k = j + 1; k < cellCandidates.size(); ++k)
                    {
                        std::set<int> unionSet = cellCandidates[i].second;
                        unionSet.insert(cellCandidates[j].second.begin(), cellCandidates[j].second.end());
                        unionSet.insert(cellCandidates[k].second.begin(), cellCandidates[k].second.end());

                        if (unionSet.size() == 3)
                        {
                            std::vector<size_t> tripleRows = {
                                cellCandidates[i].first,
                                cellCandidates[j].first,
                                cellCandidates[k].first
                            };

                            for (size_t row = 0; row < SUDOKU_SIZE; ++row)
                            {
                                if (_pBoard->getValue(row, col) == 0 &&
                                    row != tripleRows[0] && row != tripleRows[1] && row != tripleRows[2])
                                {
                                    auto cands = getCandidates(row, col);
                                    auto originalSize = cands.size();

                                    for (int val : unionSet)
                                    {
                                        cands.erase(val);
                                    }

                                    if (cands.size() == 1 && originalSize > 1)
                                    {
                                        Hint hint;
                                        hint.row = row;
                                        hint.col = col;
                                        hint.value = *cands.begin();
                                        hint.technique = "Naked Triple";

                                        std::ostringstream oss;
                                        oss << "[Naked Triple] Place " << hint.value << " in " << cellName(row, col)
                                            << " - cells " << cellName(tripleRows[0], col) << ", "
                                            << cellName(tripleRows[1], col) << ", " << cellName(tripleRows[2], col)
                                            << " lock values {";
                                        bool first = true;
                                        for (int v : unionSet)
                                        {
                                            if (!first) oss << ",";
                                            oss << v;
                                            first = false;
                                        }
                                        oss << "} in column " << (col + 1) << ".";
                                        hint.explanation = oss.str();

                                        hint.relatedCells.push_back({ tripleRows[0], col });
                                        hint.relatedCells.push_back({ tripleRows[1], col });
                                        hint.relatedCells.push_back({ tripleRows[2], col });
                                        hint.highlightCols.push_back(col);

                                        return hint;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 7: Hidden Pairs
    // Two values that can only appear in two cells within a row/col/box
    Hint findHiddenPairs() const
    {
       
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
        {
       
            for (int val1 = 1; val1 <= 9; ++val1)
            {
                for (int val2 = val1 + 1; val2 <= 9; ++val2)
                {
                    std::vector<size_t> val1Cols, val2Cols;

           
                    for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                    {
                        if (_pBoard->getValue(row, col) == 0)
                        {
                            auto cands = getCandidates(row, col);
                            if (cands.count(val1)) val1Cols.push_back(col);
                            if (cands.count(val2)) val2Cols.push_back(col);
                        }
                    }


                    if (val1Cols.size() == 2 && val2Cols.size() == 2 &&
                        val1Cols[0] == val2Cols[0] && val1Cols[1] == val2Cols[1])
                    {
              
                        for (size_t idx = 0; idx < 2; ++idx)
                        {
                            size_t col = val1Cols[idx];
                            auto cands = getCandidates(row, col);

                            
                            std::set<int> hiddenPair = { val1, val2 };
                            std::set<int> toRemove;
                            for (int c : cands)
                            {
                                if (!hiddenPair.count(c))
                                    toRemove.insert(c);
                            }

                            if (toRemove.size() > 0)
                            {
                                cands = hiddenPair;

                                if (cands.size() == 1)
                                {
                                    Hint hint;
                                    hint.row = row;
                                    hint.col = col;
                                    hint.value = *cands.begin();
                                    hint.technique = "Hidden Pair";

                                    std::ostringstream oss;
                                    oss << "[Hidden Pair] Place " << hint.value << " in " << cellName(row, col)
                                        << " - values {" << val1 << "," << val2 << "} only fit in "
                                        << cellName(row, val1Cols[0]) << " and " << cellName(row, val1Cols[1])
                                        << " in row " << (row + 1) << ".";
                                    hint.explanation = oss.str();

                                    hint.relatedCells.push_back({ row, val1Cols[0] });
                                    hint.relatedCells.push_back({ row, val1Cols[1] });
                                    hint.highlightRows.push_back(row);

                                    return hint;
                                }
                            }
                        }
                    }
                }
            }
        }

        
        for (size_t col = 0; col < SUDOKU_SIZE; ++col)
        {
            for (int val1 = 1; val1 <= 9; ++val1)
            {
                for (int val2 = val1 + 1; val2 <= 9; ++val2)
                {
                    std::vector<size_t> val1Rows, val2Rows;

                    for (size_t row = 0; row < SUDOKU_SIZE; ++row)
                    {
                        if (_pBoard->getValue(row, col) == 0)
                        {
                            auto cands = getCandidates(row, col);
                            if (cands.count(val1)) val1Rows.push_back(row);
                            if (cands.count(val2)) val2Rows.push_back(row);
                        }
                    }

                    if (val1Rows.size() == 2 && val2Rows.size() == 2 &&
                        val1Rows[0] == val2Rows[0] && val1Rows[1] == val2Rows[1])
                    {
                        for (size_t idx = 0; idx < 2; ++idx)
                        {
                            size_t row = val1Rows[idx];
                            auto cands = getCandidates(row, col);

                            std::set<int> hiddenPair = { val1, val2 };
                            std::set<int> toRemove;
                            for (int c : cands)
                            {
                                if (!hiddenPair.count(c))
                                    toRemove.insert(c);
                            }

                            if (toRemove.size() > 0)
                            {
                                cands = hiddenPair;

                                if (cands.size() == 1)
                                {
                                    Hint hint;
                                    hint.row = row;
                                    hint.col = col;
                                    hint.value = *cands.begin();
                                    hint.technique = "Hidden Pair";

                                    std::ostringstream oss;
                                    oss << "[Hidden Pair] Place " << hint.value << " in " << cellName(row, col)
                                        << " - values {" << val1 << "," << val2 << "} only fit in "
                                        << cellName(val1Rows[0], col) << " and " << cellName(val1Rows[1], col)
                                        << " in column " << (col + 1) << ".";
                                    hint.explanation = oss.str();

                                    hint.relatedCells.push_back({ val1Rows[0], col });
                                    hint.relatedCells.push_back({ val1Rows[1], col });
                                    hint.highlightCols.push_back(col);

                                    return hint;
                                }
                            }
                        }
                    }
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 8: Box/Line Reduction (Pointing Pairs/Triples)
    Hint findBoxLineReduction() const
    {
        
        for (size_t boxRow = 0; boxRow < 3; ++boxRow)
        {
            for (size_t boxCol = 0; boxCol < 3; ++boxCol)
            {
                size_t startRow = boxRow * 3;
                size_t startCol = boxCol * 3;

                for (int value = 1; value <= 9; ++value)
                {
                    
                    std::set<size_t> rows, cols;
                    std::vector<std::pair<size_t, size_t>> positions;

                    for (size_t r = startRow; r < startRow + 3; ++r)
                    {
                        for (size_t c = startCol; c < startCol + 3; ++c)
                        {
                            if (_pBoard->getValue(r, c) == value)
                            {
                                rows.clear();
                                cols.clear();
                                break;
                            }
                            if (_pBoard->getValue(r, c) == 0 &&
                                _pBoard->isValidPlacement(r, c, value))
                            {
                                rows.insert(r);
                                cols.insert(c);
                                positions.push_back({ r, c });
                            }
                        }
                        if (rows.empty()) break;
                    }

                   
                    if (rows.size() == 1 && !positions.empty())
                    {
                        size_t row = *rows.begin();

                        for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                        {
                            if (c < startCol || c >= startCol + 3)
                            {
                                if (_pBoard->getValue(row, c) == 0)
                                {
                                    auto cands = getCandidates(row, c);
                                    auto originalSize = cands.size();
                                    cands.erase(value);

                                    if (cands.size() == 1 && originalSize > 1)
                                    {
                                        Hint hint;
                                        hint.row = row;
                                        hint.col = c;
                                        hint.value = *cands.begin();
                                        hint.technique = "Pointing Pair/Triple";

                                        std::ostringstream oss;
                                        oss << "[Pointing Pair] Place " << hint.value << " in " << cellName(row, c)
                                            << " - value " << value << " points to row " << (row + 1)
                                            << " from box " << getBoxNumber(startRow, startCol) << ".";
                                        hint.explanation = oss.str();


                                        for (const auto& pos : positions)
                                        {
                                            hint.relatedCells.push_back(pos);
                                        }


                                        hint.highlightRows.push_back(row);
                                        hint.highlightBoxes.push_back(getBoxNumber(startRow, startCol));

                                        return hint;
                                    }
                                }
                            }
                        }
                    }

          
                    if (cols.size() == 1 && !positions.empty())
                    {
                        size_t col = *cols.begin();


                        for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                        {
                            if (r < startRow || r >= startRow + 3)
                            {
                                if (_pBoard->getValue(r, col) == 0)
                                {
                                    auto cands = getCandidates(r, col);
                                    auto originalSize = cands.size();
                                    cands.erase(value);

                                    if (cands.size() == 1 && originalSize > 1)
                                    {
                                        Hint hint;
                                        hint.row = r;
                                        hint.col = col;
                                        hint.value = *cands.begin();
                                        hint.technique = "Pointing Pair/Triple";

                                        std::ostringstream oss;
                                        oss << "[Pointing Pair] Place " << hint.value << " in " << cellName(r, col)
                                            << " - value " << value << " points to column " << (col + 1)
                                            << " from box " << getBoxNumber(startRow, startCol) << ".";
                                        hint.explanation = oss.str();

                                  
                                        for (const auto& pos : positions)
                                        {
                                            hint.relatedCells.push_back(pos);
                                        }

                                   
                                        hint.highlightCols.push_back(col);
                                        hint.highlightBoxes.push_back(getBoxNumber(startRow, startCol));

                                        return hint;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 9: Box/Box Reduction (Claiming)
    Hint findBoxBoxReduction() const
    {
       
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
        {
            for (int value = 1; value <= 9; ++value)
            {

                bool found = false;
                for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                {
                    if (_pBoard->getValue(row, c) == value)
                    {
                        found = true;
                        break;
                    }
                }
                if (found) continue;

                std::set<int> boxes;
                std::vector<size_t> possibleCols;

                for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                {
                    if (_pBoard->getValue(row, col) == 0 &&
                        _pBoard->isValidPlacement(row, col, value))
                    {
                        boxes.insert(col / 3);
                        possibleCols.push_back(col);
                    }
                }

                if (boxes.size() == 1 && !possibleCols.empty())
                {
                    int boxCol = *boxes.begin();
                    size_t startCol = boxCol * 3;
                    size_t startRow = (row / 3) * 3;

                    
                    for (size_t r = startRow; r < startRow + 3; ++r)
                    {
                        if (r != row)
                        {
                            for (size_t c = startCol; c < startCol + 3; ++c)
                            {
                                if (_pBoard->getValue(r, c) == 0)
                                {
                                    auto cands = getCandidates(r, c);
                                    auto originalSize = cands.size();
                                    cands.erase(value);

                                    if (cands.size() == 1 && originalSize > 1)
                                    {
                                        Hint hint;
                                        hint.row = r;
                                        hint.col = c;
                                        hint.value = *cands.begin();
                                        hint.technique = "Box/Line Reduction";

                                        std::ostringstream oss;
                                        oss << "[Box/Line] Place " << hint.value << " in " << cellName(r, c)
                                            << " - value " << value << " claims row " << (row + 1)
                                            << " in box " << getBoxNumber(row, startCol) << ".";
                                        hint.explanation = oss.str();

                                        
                                        for (size_t col : possibleCols)
                                        {
                                            hint.relatedCells.push_back({ row, col });
                                        }

                                        
                                        hint.highlightRows.push_back(row);
                                        hint.highlightBoxes.push_back(getBoxNumber(row, startCol));

                                        return hint;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        for (size_t col = 0; col < SUDOKU_SIZE; ++col)
        {
            for (int value = 1; value <= 9; ++value)
            {
                bool found = false;
                for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                {
                    if (_pBoard->getValue(r, col) == value)
                    {
                        found = true;
                        break;
                    }
                }
                if (found) continue;

                std::set<int> boxes;
                std::vector<size_t> possibleRows;

                for (size_t row = 0; row < SUDOKU_SIZE; ++row)
                {
                    if (_pBoard->getValue(row, col) == 0 &&
                        _pBoard->isValidPlacement(row, col, value))
                    {
                        boxes.insert(row / 3);
                        possibleRows.push_back(row);
                    }
                }

                if (boxes.size() == 1 && !possibleRows.empty())
                {
                    int boxRow = *boxes.begin();
                    size_t startRow = boxRow * 3;
                    size_t startCol = (col / 3) * 3;

                    for (size_t r = startRow; r < startRow + 3; ++r)
                    {
                        for (size_t c = startCol; c < startCol + 3; ++c)
                        {
                            if (c != col && _pBoard->getValue(r, c) == 0)
                            {
                                auto cands = getCandidates(r, c);
                                auto originalSize = cands.size();
                                cands.erase(value);

                                if (cands.size() == 1 && originalSize > 1)
                                {
                                    Hint hint;
                                    hint.row = r;
                                    hint.col = c;
                                    hint.value = *cands.begin();
                                    hint.technique = "Box/Line Reduction";

                                    std::ostringstream oss;
                                    oss << "[Box/Line] Place " << hint.value << " in " << cellName(r, c)
                                        << " - value " << value << " claims column " << (col + 1)
                                        << " in box " << getBoxNumber(startRow, col) << ".";
                                    hint.explanation = oss.str();

                                    
                                    for (size_t row : possibleRows)
                                    {
                                        hint.relatedCells.push_back({ row, col });
                                    }


                                    hint.highlightCols.push_back(col);
                                    hint.highlightBoxes.push_back(getBoxNumber(startRow, col));

                                    return hint;
                                }
                            }
                        }
                    }
                }
            }
        }

        return Hint();
    }

    // TECHNIQUE 10: X-Wing
    // If a candidate appears in exactly 2 cells in 2 different rows, and these cells are in the same 2 columns,
    // then that candidate can be eliminated from other cells in those columns
    Hint findXWing() const
    {
        
        for (int value = 1; value <= 9; ++value)
        {
            std::map<std::set<size_t>, std::vector<size_t>> rowPatterns;

            for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            {
                std::set<size_t> cols;
                for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                {
                    if (_pBoard->getValue(row, col) == 0)
                    {
                        auto cands = getCandidates(row, col);
                        if (cands.count(value))
                        {
                            cols.insert(col);
                        }
                    }
                }

          
                if (cols.size() == 2)
                {
                    rowPatterns[cols].push_back(row);
                }
            }

        
            for (const auto& pattern : rowPatterns)
            {
                if (pattern.second.size() == 2)
                {

                    const std::set<size_t>& wingCols = pattern.first;
                    size_t row1 = pattern.second[0];
                    size_t row2 = pattern.second[1];

                    
                    for (size_t col : wingCols)
                    {
                        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
                        {
                            if (row != row1 && row != row2 && _pBoard->getValue(row, col) == 0)
                            {
                                auto cands = getCandidates(row, col);
                                auto originalSize = cands.size();
                                cands.erase(value);

                                if (cands.size() == 1 && originalSize > 1)
                                {
                                    Hint hint;
                                    hint.row = row;
                                    hint.col = col;
                                    hint.value = *cands.begin();
                                    hint.technique = "X-Wing";

                                    std::ostringstream oss;
                                    oss << "[X-Wing] Place " << hint.value << " in " << cellName(row, col)
                                        << " - value " << value << " forms X-Wing in rows " << (row1 + 1)
                                        << " and " << (row2 + 1) << ", columns ";
                                    bool first = true;
                                    for (size_t c : wingCols)
                                    {
                                        if (!first) oss << " and ";
                                        oss << (c + 1);
                                        first = false;
                                    }
                                    oss << ".";
                                    hint.explanation = oss.str();

                                    // Mark X-Wing corners as related
                                    for (size_t c : wingCols)
                                    {
                                        hint.relatedCells.push_back({ row1, c });
                                        hint.relatedCells.push_back({ row2, c });
                                    }

                                    // Highlight affected columns
                                    for (size_t c : wingCols)
                                    {
                                        hint.highlightCols.push_back(c);
                                    }

                                    return hint;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Check columns for X-Wing pattern
        for (int value = 1; value <= 9; ++value)
        {
            std::map<std::set<size_t>, std::vector<size_t>> colPatterns;

            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                std::set<size_t> rows;
                for (size_t row = 0; row < SUDOKU_SIZE; ++row)
                {
                    if (_pBoard->getValue(row, col) == 0)
                    {
                        auto cands = getCandidates(row, col);
                        if (cands.count(value))
                        {
                            rows.insert(row);
                        }
                    }
                }

                if (rows.size() == 2)
                {
                    colPatterns[rows].push_back(col);
                }
            }

            for (const auto& pattern : colPatterns)
            {
                if (pattern.second.size() == 2)
                {
                    const std::set<size_t>& wingRows = pattern.first;
                    size_t col1 = pattern.second[0];
                    size_t col2 = pattern.second[1];

                    for (size_t row : wingRows)
                    {
                        for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                        {
                            if (col != col1 && col != col2 && _pBoard->getValue(row, col) == 0)
                            {
                                auto cands = getCandidates(row, col);
                                auto originalSize = cands.size();
                                cands.erase(value);

                                if (cands.size() == 1 && originalSize > 1)
                                {
                                    Hint hint;
                                    hint.row = row;
                                    hint.col = col;
                                    hint.value = *cands.begin();
                                    hint.technique = "X-Wing";

                                    std::ostringstream oss;
                                    oss << "[X-Wing] Place " << hint.value << " in " << cellName(row, col)
                                        << " - value " << value << " forms X-Wing in columns " << (col1 + 1)
                                        << " and " << (col2 + 1) << ", rows ";
                                    bool first = true;
                                    for (size_t r : wingRows)
                                    {
                                        if (!first) oss << " and ";
                                        oss << (r + 1);
                                        first = false;
                                    }
                                    oss << ".";
                                    hint.explanation = oss.str();

                                    for (size_t r : wingRows)
                                    {
                                        hint.relatedCells.push_back({ r, col1 });
                                        hint.relatedCells.push_back({ r, col2 });
                                    }

                                    for (size_t r : wingRows)
                                    {
                                        hint.highlightRows.push_back(r);
                                    }

                                    return hint;
                                }
                            }
                        }
                    }
                }
            }
        }

        return Hint();
    }


    Hint findBacktrackingHint() const
    {
        if (!_pBoard)
            return Hint();


        auto currentState = _pBoard->getAllCells();


        BacktrackingSolver solver(const_cast<SudokuBoard*>(_pBoard));

        if (solver.solve())
        {
            
            for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            {
                for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                {
                    if (currentState[row][col].value == 0)
                    {
                        int solvedValue = _pBoard->getValue(row, col);

                      
                        for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                        {
                            for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                            {
                                _pBoard->setCell(r, c, currentState[r][c]);
                            }
                        }

                    
                        Hint hint;
                        hint.row = row;
                        hint.col = col;
                        hint.value = solvedValue;
                        hint.technique = "Backtracking";

                        std::ostringstream oss;
                        oss << "[Backtracking] Place " << solvedValue << " in " << cellName(row, col)
                            << " - sorry, best I can do right now!";
                        hint.explanation = oss.str();

                   
                        hint.highlightRows.push_back(row);
                        hint.highlightCols.push_back(col);
                        hint.highlightBoxes.push_back(getBoxNumber(row, col));

                        return hint;
                    }
                }
            }
        }

        for (size_t r = 0; r < SUDOKU_SIZE; ++r)
        {
            for (size_t c = 0; c < SUDOKU_SIZE; ++c)
            {
                _pBoard->setCell(r, c, currentState[r][c]);
            }
        }

        return Hint();
    }

public:
    HintEngine(SudokuBoard* pBoard = nullptr)
        : _pBoard(pBoard)
    {
    }

    void setBoard(SudokuBoard* pBoard)
    {
        _pBoard = pBoard;
    }

    Hint getHint() const
    {
        if (!_pBoard)
            return Hint();


        Hint hint = findNakedSingle();
        if (hint.isValid())
            return hint;

      
        hint = findHiddenSingleInRow();
        if (hint.isValid())
            return hint;

        hint = findHiddenSingleInColumn();
        if (hint.isValid())
            return hint;

        hint = findHiddenSingleInBox();
        if (hint.isValid())
            return hint;


        hint = findNakedPair();
        if (hint.isValid())
            return hint;

   
        hint = findBoxLineReduction();
        if (hint.isValid())
            return hint;

 
        hint = findBoxBoxReduction();
        if (hint.isValid())
            return hint;

        hint = findNakedTriples();
        if (hint.isValid())
            return hint;

    
        hint = findHiddenPairs();
        if (hint.isValid())
            return hint;

        
        hint = findXWing();
        if (hint.isValid())
            return hint;

        // Fallback: Use backtracking solver
        hint = findBacktrackingHint();
        if (hint.isValid())
            return hint;

        return Hint(); 
    }

 
    std::set<int> getCandidatesForCell(size_t row, size_t col) const
    {
        return getCandidates(row, col);
    }
};