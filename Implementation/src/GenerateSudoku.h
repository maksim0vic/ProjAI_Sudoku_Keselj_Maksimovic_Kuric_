#pragma once
#include "SudokuBoard.h"
#include "BacktrackingSolver.h"
#include "HintEngine.h"
#include "Difficulty.h"
#include <random>
#include <vector>
#include <set>
#include <map>

class GenerateSudoku
{
private:
    std::mt19937 _gen;
    
    struct TechniqueStats
    {
        bool needsNakedSingle = false;
        bool needsHiddenSingle = false;
        bool needsNakedPair = false;
        bool needsNakedTriple = false;
        bool needsHiddenPair = false;
        bool needsPointingPair = false;
        bool needsBoxLine = false;
        bool needsXWing = false;
        bool needsBacktracking = false;
        
        int maxTechniqueLevel = 0;  
    };
    
    TechniqueStats analyzePuzzleTechniques(SudokuBoard& board)
    {
        TechniqueStats stats;
        HintEngine hintEngine(&board);
        auto originalState = board.getAllCells();
        int movesCount = 0;
        const int MAX_MOVES = 200;  
        
        while (!board.isSolved() && movesCount < MAX_MOVES)
        {
            Hint hint = hintEngine.getHint();
            if (!hint.isValid())
                break;
            
            if (hint.technique == "Naked Single")
            {
                stats.needsNakedSingle = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 1);
            }
            else if (hint.technique == "Hidden Single (Row)" || 
                     hint.technique == "Hidden Single (Column)" || 
                     hint.technique == "Hidden Single (Box)")
            {
                stats.needsHiddenSingle = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 1);
            }
            else if (hint.technique == "Naked Pair")
            {
                stats.needsNakedPair = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 2);
            }
            else if (hint.technique == "Pointing Pair/Triple")
            {
                stats.needsPointingPair = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 2);
            }
            else if (hint.technique == "Box/Line Reduction")
            {
                stats.needsBoxLine = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 2);
            }
            else if (hint.technique == "Naked Triple")
            {
                stats.needsNakedTriple = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 3);
            }
            else if (hint.technique == "Hidden Pair")
            {
                stats.needsHiddenPair = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 3);
            }
            else if (hint.technique == "X-Wing")
            {
                stats.needsXWing = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 4);
            }
            else if (hint.technique == "Backtracking")
            {
                stats.needsBacktracking = true;
                stats.maxTechniqueLevel = std::max(stats.maxTechniqueLevel, 5);
            }
            
            board.setValue(hint.row, hint.col, hint.value, false);
            movesCount++;
        }
        
        for (size_t row = 0; row < SUDOKU_SIZE; ++row)
        {
            for (size_t col = 0; col < SUDOKU_SIZE; ++col)
            {
                board.setCell(row, col, originalState[row][col]);
            }
        }
        
        return stats;
    }
    
    bool generateSolvedBoard(SudokuBoard& board)
    {
        board.clearAll(true);
        
        for (int box = 0; box < 3; ++box)
        {
            std::vector<int> numbers = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
            std::shuffle(numbers.begin(), numbers.end(), _gen);
            int startRow = box * 3;
            int startCol = box * 3;
            int idx = 0;
            
            for (int row = 0; row < 3; ++row)
            {
                for (int col = 0; col < 3; ++col)
                {
                    board.setValue(startRow + row, startCol + col, numbers[idx++], false);
                }
            }
        }
        BacktrackingSolver solver(&board);
        return solver.solve();
    }
    
    bool removeCluesStrategically(SudokuBoard& board, Difficulty targetDifficulty, int maxAttempts = 50)
    {
        auto solution = board.getAllValues();
        std::vector<std::pair<int, int>> allCells;
        for (int r = 0; r < 9; r++)
        {
            for (int c = 0; c < 9; c++)
            {
                allCells.push_back({r, c});
            }
        }
        
		// shuffling for random removal 
        std::shuffle(allCells.begin(), allCells.end(), _gen);
        
		// difficulty based on number of clues and techniques required to solve
        int targetClues;
        int minTechniqueLevel;
        
        switch (targetDifficulty)
        {
        case Difficulty::Easy:
            targetClues = 40;  // more clues for easier puzzles
            minTechniqueLevel = 1;  // only needs basic techniques
            break;
        case Difficulty::Medium:
            targetClues = 32;  // Moderate clues
            minTechniqueLevel = 2;  //needs at least Naked Pairs or Pointing Pairs
            break;
        case Difficulty::Hard:
            targetClues = 26;  //fewer clues
            minTechniqueLevel = 3;  //needs Naked Triples or Hidden Pairs
            break;
        default:
            targetClues = 32;
            minTechniqueLevel = 2;
            break;
        }
        
        int attempts = 0;
        int bestClueCount = 81;
        TechniqueStats bestStats;
        auto bestBoard = board.getAllCells();
        
        while (attempts < maxAttempts)
        {
            attempts++;
                       
            for (int r = 0; r < 9; r++)
            {
                for (int c = 0; c < 9; c++)
                {
                    board.setValue(r, c, solution[r][c], false);
                }
            }
            
            std::shuffle(allCells.begin(), allCells.end(), _gen);
            int removedCount = 0;
            
            for (const auto& cell : allCells)
            {
                int r = cell.first;
                int c = cell.second;
                int savedValue = board.getValue(r, c);
                board.setValue(r, c, 0, false);
                BacktrackingSolver solver(&board);
                if (!solver.hasUniqueSolution())
                {
                    board.setValue(r, c, savedValue, false);
                }
                else
                {
                    removedCount++;
                    if (removedCount >= (81 - targetClues))
                        break;
                }
            }
            TechniqueStats stats = analyzePuzzleTechniques(board);
            int currentClues = 81 - removedCount;
            bool meetsRequirements = (stats.maxTechniqueLevel >= minTechniqueLevel);
            if (targetDifficulty == Difficulty::Easy && stats.maxTechniqueLevel > 1)
            {
                meetsRequirements = false;
            }
            
            if (meetsRequirements && currentClues < bestClueCount)
            {
                bestClueCount = currentClues;
                bestStats = stats;
                bestBoard = board.getAllCells();
                
                
                if (currentClues <= targetClues)
                    break;
            }
        }
        
        if (bestClueCount < 81)
        {
            for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            {
                for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                {
                    board.setCell(row, col, bestBoard[row][col]);
                }
            }
            
            
            for (size_t row = 0; row < SUDOKU_SIZE; ++row)
            {
                for (size_t col = 0; col < SUDOKU_SIZE; ++col)
                {
                    int value = board.getValue(row, col);
                    if (value > 0)
                    {
                        board.setValue(row, col, value, true);
                    }
                }
            }
            return true;
        }
        return false;
    }
public:
    GenerateSudoku()
    {
        std::random_device rd;
        _gen.seed(rd());
    }
    
    bool generatePuzzle(SudokuBoard& board, Difficulty difficulty)
    {
        if (!generateSolvedBoard(board))
            return false;
        return removeCluesStrategically(board, difficulty);
    }
};