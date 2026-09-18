#pragma once
#include <gui/View.h>
#include <gui/VerticalLayout.h>
#include <gui/FileDialog.h>
#include <gui/Sound.h>
#include <gui/Timer.h>
#include <fo/FileOperations.h>
#include "SudokuBoard.h"
#include "SudokuCanvas.h"
#include "BacktrackingSolver.h"
#include "GenerateSudoku.h"
#include "HintEngine.h"
#include "Difficulty.h"
#include <array>
#include <set>

class ViewHome : public gui::View
{
private:
    Difficulty _currentDifficulty = Difficulty::Medium;

protected:
    SudokuBoard        _board;
    SudokuCanvas       _canvas;
    BacktrackingSolver _solver;
    GenerateSudoku     _smartGenerator;
    HintEngine         _hintEngine;

    gui::Sound _soundAchievement;
    gui::Sound _soundMistake;
    gui::Sound _soundFocus1;
    gui::Sound _soundFocus2;
    gui::Sound _soundFocus3;
    gui::Sound _soundGameOver;

    gui::Timer _gameOverTimer;
    gui::Timer _tickTimer;

    bool _insertMode      = false;
    bool _hasSolution     = false;
    bool _showingGameOver = false;

    std::array<std::array<int, SUDOKU_SIZE>, SUDOKU_SIZE> _solution;

    int _mistakeCount = 0;
    static constexpr int MAX_MISTAKES = 3;

    bool   _timerRunning       = false;
    double _accumulatedSeconds = 0.0;

    gui::VerticalLayout _vlMain;

    // ---- mistakes ----
    void updateMistakesLabel() { _canvas.setMistakeCount(_mistakeCount, MAX_MISTAKES); }

    void resetMistakes()
    {
        _mistakeCount = 0;
        updateMistakesLabel();
    }

    void incrementMistakes()
    {
        ++_mistakeCount;
        updateMistakesLabel();
        _soundMistake.play();
    }

    // ---- board helpers ----
    void clearUserEntries()
    {
        for (size_t r = 0; r < SUDOKU_SIZE; ++r)
            for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                if (!_board.isInitial(r, c))
                    _board.setValue(r, c, 0, false);
    }

    void resetGameState()
    {
        _insertMode = false;
        _hasSolution = false;
        _showingGameOver = false;

        _canvas.setInsertMode(false);
        _canvas.setGameActive(false);
        _canvas.setSolved(false);
        _canvas.setStartButtonTitle("Start Game");
        _canvas.clearPonchoMessage();
        _canvas.hideGameOverPopup();

        _timerRunning = false;
        _accumulatedSeconds = 0.0;
        _canvas.setTimeString("00:00");

        _tickTimer.stop();
        _gameOverTimer.stop();

        resetMistakes();
    }

    void triggerGameOver()
    {
        if (_showingGameOver) return;
        _showingGameOver = true;

        _canvas.setStatusMessage("Game Over! Too many mistakes.");
        _canvas.setGameActive(false);
        _canvas.setStartButtonTitle("Restart");
        _canvas.setPonchoMessage("Where is the intelligence?!?");
        _soundGameOver.play();
        _canvas.showGameOverPopup();

        _timerRunning = false;
        _tickTimer.stop();
        _gameOverTimer.start();
    }

    void updateTimerString()
    {
        int total = (int)_accumulatedSeconds;
        td::String s;
        s.format("%02d:%02d", total / 60, total % 60);
        _canvas.setTimeString(s);
    }

    void handleNumberInput(int value)
    {
        if (_mistakeCount >= MAX_MISTAKES && !_insertMode)
        {
            _canvas.setStatusMessage("Game Over! Too many mistakes. Click Restart to try again.");
            _canvas.setPonchoMessage("Where is the intelligence?!?");
            return;
        }

        auto [selectedRow, selectedCol] = _canvas.getSelectedCell();
        if (selectedRow < 0 || selectedCol < 0)
        {
            _canvas.setStatusMessage("Please select a cell first!");
            return;
        }

        if (!_insertMode && (!_timerRunning || !_canvas.isGameActive()))
        {
            _canvas.setStatusMessage("Please start or resume the game first!");
            return;
        }

        if (_insertMode)
        {
            if (value > 0)
            {
                _canvas.setSelectedCellValue(value);
                _canvas.setStatusMessage("Value placed in insert mode.");
            }
            else
            {
                _canvas.clearSelectedCell();
                _canvas.setStatusMessage("Cell cleared in insert mode.");
            }
            return;
        }

        if (_canvas.isNotesMode())
        {
            if (value == 0)
            {
                _canvas.clearNotesInSelectedCell();
                _canvas.setStatusMessage("Notes cleared.");
            }
            else
            {
                _canvas.toggleNoteForSelectedCell(value);
                _canvas.setStatusMessage("Note toggled.");
            }
            return;
        }

        if (value == 0)
        {
            _canvas.clearSelectedCell();
            _canvas.setStatusMessage("Cell cleared.");
            return;
        }

        if (!_canvas.setSelectedCellValue(value))
        {
            incrementMistakes();
            _canvas.setStatusMessage("Invalid placement! Number already exists in row, column, or box.");
            _canvas.setPonchoMessage("You know you have notes and hints right? Try to avoid reaching 3 mistakes.");
            if (_mistakeCount >= MAX_MISTAKES) triggerGameOver();
            return;
        }

        _canvas.clearHintVisualization();
        _canvas.clearPonchoMessage();

        if (_hasSolution && value != _solution[selectedRow][selectedCol])
        {
            _board.setValue(selectedRow, selectedCol, 0, false);
            _canvas.clearSelection();
            _canvas.refresh();
            incrementMistakes();
            if (_mistakeCount >= MAX_MISTAKES)
                triggerGameOver();
            else
                _canvas.setStatusMessage("Wrong! That's a mistake.");
            return;
        }

        if (_board.isSolved())
        {
            _canvas.setSolved(true);
            _soundAchievement.play();
            _canvas.setStatusMessage("Congratulations! Puzzle solved!");
            _timerRunning = false;
            _tickTimer.stop();
            _canvas.setGameActive(false);
        }
        else
        {
            _canvas.setStatusMessage(_hasSolution ? "Correct! Keep going." : "Value placed successfully.");
        }
    }

    void handleInsert()
    {
        _canvas.setPonchoMessage("You can cancel inserting if board is empty ~");

        if (_insertMode)
        {
            int enteredCount = 0;
            for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                    if (_board.getValue(r, c) > 0) ++enteredCount;

            if (enteredCount > 0 && enteredCount < 17)
            {
                _canvas.setStatusMessage("You must enter at least 17 numbers!");
                return;
            }

            if (enteredCount > 0)
            {
                for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                    for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                    {
                        int v = _board.getValue(r, c);
                        if (v > 0) _board.setValue(r, c, v, true);
                    }
                _canvas.setStatusMessage("Puzzle inserted successfully! Click Start Game to play.");
            }
            else
            {
                _canvas.setStatusMessage("Insert mode cancelled.");
                _canvas.clearPonchoMessage();
            }

            _insertMode = false;
            _canvas.setInsertMode(false);
            _hasSolution = false;
        }
        else
        {
            _insertMode = true;
            _board.clearAll(true);
            _canvas.setInsertMode(true);
            _canvas.setGameActive(true);
            _canvas.setStatusMessage("Insert mode activated. Enter your puzzle (minimum 17 numbers).");
            _canvas.setStartButtonTitle("Start Game");
            _canvas.clearPonchoMessage();
            _timerRunning = false;
            _accumulatedSeconds = 0.0;
            _tickTimer.stop();
            _canvas.setTimeString("00:00");
            resetMistakes();
        }

        _canvas.refresh();
    }

    void handleSave() { saveBoardToFile(); }
    void handleLoad() { loadBoardFromFile(); }

    void handleSolution()
    {
        if (!_hasSolution)
        {
            _canvas.setStatusMessage("No solution available. Start the game first!");
            return;
        }
        clearUserEntries();
        for (size_t r = 0; r < SUDOKU_SIZE; ++r)
            for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                _board.setValue(r, c, _solution[r][c], _board.isInitial(r, c));

        _canvas.setSolved(true);
        _soundAchievement.play();
        _canvas.setStatusMessage("Solution revealed!");
        _canvas.setPonchoMessage("Black Magic :p");
        _timerRunning = false;
        _tickTimer.stop();
        _canvas.setGameActive(false);

        _mistakeCount = MAX_MISTAKES;
        updateMistakesLabel();
        _canvas.setStartButtonTitle("Restart");
        _showingGameOver = false;
        _gameOverTimer.stop();
    }

    void handleStartPause()
    {
        if (_mistakeCount >= MAX_MISTAKES)
        {
            clearUserEntries();
            resetGameState();
            _canvas.refresh();
            _canvas.setStatusMessage("Game restarted. Press Start Game to begin.");
            _canvas.setPonchoMessage("Press Start Game to play !!");
            return;
        }

        if (_insertMode) return;

        if (!_timerRunning && !_hasSolution)
        {
            int enteredCount = 0;
            for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                    if (_board.getValue(r, c) > 0) ++enteredCount;

            if (enteredCount < 17)
            {
                _canvas.setStatusMessage("Board must contain at least 17 numbers to start!");
                return;
            }
        }

        if (_timerRunning)
        {
            _timerRunning = false;
            _tickTimer.stop();
            _canvas.setGameActive(false);
            _canvas.setStartButtonTitle("Resume Game");
            _canvas.setStatusMessage("Game paused. Click Resume Game to continue.");
        }
        else
        {
            if (!_hasSolution)
            {
                _solver.setBoard(&_board);
                auto currentState = _board.getAllCells();
                if (!_solver.solve())
                {
                    _canvas.setStatusMessage("Error: Puzzle has no solution!");
                    return;
                }
                _solution = _board.getAllValues();
                _hasSolution = true;
                for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                    for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                        _board.setCell(r, c, currentState[r][c]);
                _canvas.refresh();
                resetMistakes();
            }

            _canvas.clearPonchoMessage();
            _timerRunning = true;
            _tickTimer.start();
            _canvas.setGameActive(true);
            _canvas.setFocus();
            _canvas.setStartButtonTitle("Pause Game");
            _canvas.setStatusMessage("Game started! Place numbers to solve the puzzle.");
        }
    }

    void handleNotesToggle()
    {
        _canvas.setStatusMessage(_canvas.isNotesMode()
            ? "Notes mode ON - toggle numbers to add/remove notes."
            : "Notes mode OFF - place numbers normally.");
    }

    void handleHint()
    {
        if (!_hasSolution)
        {
            _canvas.setStatusMessage("No solution available. Start the game first!");
            return;
        }
        if (!_canvas.isGameActive())
        {
            _canvas.setStatusMessage("Game is not active. Resume the game to get hints!");
            return;
        }

        _hintEngine.setBoard(&_board);
        Hint hint = _hintEngine.getHint();

        if (hint.isValid())
        {
            _canvas.showHintVisualization(hint);
            _canvas.setPonchoMessage(td::String(hint.explanation.c_str()));
            _canvas.setStatusMessage("Hint shown! Click on the board to continue playing.");
        }
        else
        {
            _canvas.setStatusMessage("No hints available - puzzle might be complete or very advanced!");
        }
    }

    void handleShowNumber()
    {
        if (!_hasSolution)
        {
            _canvas.setStatusMessage("No solution available. Start the game first!");
            return;
        }
        if (!_canvas.isGameActive())
        {
            _canvas.setStatusMessage("Game is not active. Resume the game to get hints!");
            return;
        }

        auto [r, c] = _canvas.getSelectedCell();
        if (r < 0 || r >= (int)SUDOKU_SIZE || c < 0 || c >= (int)SUDOKU_SIZE)
        {
            _canvas.setStatusMessage("Please select an empty cell first!");
            return;
        }
        if (_board.getValue(r, c) != 0)
        {
            _canvas.setStatusMessage("Selected cell already has a value!");
            return;
        }

        _board.setValue(r, c, _solution[r][c], false);
        _canvas.refresh();
        _canvas.setStatusMessage("Number revealed in selected cell!");
    }

    void handleClearAll()
    {
        if (_insertMode)
            _board.clearAll(true);
        else
            clearUserEntries();

        _canvas.refresh();
        _canvas.setStatusMessage(_insertMode ? "Board cleared." : "All entered numbers cleared.");
    }

    void saveBoardToFile()
    {
        td::UINT4 dlgID = 5555;
        gui::SaveFileDialog::show(this, tr("SaveBoard"), "*.txt", dlgID,
            [this](gui::FileDialog* pFileDlg)
            {
                if (pFileDlg->getStatus() != gui::FileDialog::Status::OK)
                {
                    _canvas.setStatusMessage("Save cancelled.");
                    return;
                }
                td::String strFileName = pFileDlg->getFileName();
                fo::OutFile f;
                if (!fo::createTextFile(f, strFileName, true))
                {
                    _canvas.setStatusMessage("Failed to save board!");
                    return;
                }
                auto values = _board.getAllValues();
                for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                {
                    for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                    {
                        f << values[r][c];
                        if (c < SUDOKU_SIZE - 1) f << " ";
                    }
                    f << "\n";
                }
                f << "INITIAL\n";
                for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                {
                    for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                    {
                        f << (_board.isInitial(r, c) ? "1" : "0");
                        if (c < SUDOKU_SIZE - 1) f << " ";
                    }
                    f << "\n";
                }
                f.close();
                _canvas.setStatusMessage("Board saved successfully!");
            });
    }

    void loadBoardFromFile()
    {
        td::UINT4 dlgID = 5556;
        gui::OpenFileDialog::show(this, tr("LoadBoard"), "*.txt", dlgID,
            [this](gui::FileDialog* pFileDlg)
            {
                if (pFileDlg->getStatus() != gui::FileDialog::Status::OK)
                {
                    _canvas.setStatusMessage("Load cancelled.");
                    return;
                }
                td::String strFileName = pFileDlg->getFileName();
                fo::InFile f;
                if (!fo::openFile(f, strFileName))
                {
                    _canvas.setStatusMessage("Failed to load board!");
                    return;
                }
                std::string content;
                char ch;
                while (f.get(ch)) content += ch;
                f.close();

                parseBoardData(td::String(content.c_str()));

                for (size_t r = 0; r < SUDOKU_SIZE; ++r)
                    for (size_t c = 0; c < SUDOKU_SIZE; ++c)
                    {
                        int v = _board.getValue(r, c);
                        if (v > 0) _board.setValue(r, c, v, true);
                    }

                resetGameState();
                _canvas.refresh();
                _canvas.setStatusMessage("Board loaded successfully! Click Start Game to play.");
                _canvas.setPonchoMessage("Press Start Game to play !!");
            });
    }

    void parseBoardData(const td::String& content)
    {
        _board.clearAll(true);

        const char* it  = content.c_str();
        const char* end = it + content.length();

        std::array<std::array<int,  SUDOKU_SIZE>, SUDOKU_SIZE> values{};
        std::array<std::array<bool, SUDOKU_SIZE>, SUDOKU_SIZE> initials{};

        size_t row = 0, col = 0;
        bool parsingInitials = false;

        while (it < end && row < SUDOKU_SIZE)
        {
            char c = *it;
            if (c == 'I')
            {
                while (it < end && *it != '\n') ++it;
                if (it < end) ++it;
                parsingInitials = true;
                row = col = 0;
                continue;
            }
            if (c == ' ' || c == '\t') { ++it; continue; }
            if (c == '\n' || c == '\r')
            {
                if (col > 0) { ++row; col = 0; }
                ++it;
                continue;
            }
            if (c >= '0' && c <= '9')
            {
                int digit = c - '0';
                if (!parsingInitials) values[row][col] = digit;
                else                  initials[row][col] = (digit == 1);
                if (++col >= SUDOKU_SIZE) { ++row; col = 0; }
            }
            ++it;
        }

        for (size_t r = 0; r < SUDOKU_SIZE; ++r)
            for (size_t c2 = 0; c2 < SUDOKU_SIZE; ++c2)
                if (values[r][c2] > 0)
                    _board.setValue(r, c2, values[r][c2], initials[r][c2]);}

public:
    ViewHome()
        : _soundAchievement(":achievement")
        , _soundMistake(":mistake")
        , _soundFocus1(":focus1")
        , _soundFocus2(":focus2")
        , _soundFocus3(":focus3")
        , _soundGameOver(":gameoversound")
        , _gameOverTimer(this, 5.0f, false)
        , _tickTimer(this, 1.0f, true)
        , _vlMain(1)
    {
        _canvas.setBoard(&_board);
        _solver.setBoard(&_board);
        _hintEngine.setBoard(&_board);

        _canvas.onNumberInput = [this](int v) { handleNumberInput(v); };
        _canvas.onInsert      = [this]()      { handleInsert();       };
        _canvas.onSave        = [this]()      { handleSave();         };
        _canvas.onLoad        = [this]()      { handleLoad();         };
        _canvas.onSolution    = [this]()      { handleSolution();     };
        _canvas.onStartPause  = [this]()      { handleStartPause();   };
        _canvas.onNotesToggle = [this]()      { handleNotesToggle();  };
        _canvas.onHint        = [this]()      { handleHint();         };
        _canvas.onShowNumber  = [this]()      { handleShowNumber();   };
        _canvas.onClearAll    = [this]()      { handleClearAll();     };

        _tickTimer.onTimer([this]()
        {
            if (_timerRunning)
            {
                _accumulatedSeconds += 1.0;
                updateTimerString();
            }
        });

        _gameOverTimer.onTimer([this]()
        {
            _showingGameOver = false;
            _canvas.hideGameOverPopup();
            _gameOverTimer.stop();
        });

        _vlMain.append(_canvas);
        setLayout(&_vlMain);

        _canvas.setStatusMessage("Generate Sudoku (Easy, Medium, Hard) in toolbar above me or Insert your own!!");
        _canvas.setPonchoMessage("Generate Sudoku (Easy, Medium, Hard) in toolbar above me or Insert your own!!");
    }

    bool setGameDifficulty(Difficulty difficulty)
    {
        _currentDifficulty = difficulty;
        if (!_smartGenerator.generatePuzzle(_board, difficulty))
        {
            _canvas.setStatusMessage("Failed to generate puzzle!");
            return false;
        }
        resetGameState();
        _canvas.refresh();

        const char* diffStr =
            difficulty == Difficulty::Easy   ? "Easy"   :
            difficulty == Difficulty::Medium ? "Medium" : "Hard";

        td::String msg;
        msg.format("%s puzzle generated! Click Start Game to play.", diffStr);
        _canvas.setStatusMessage(msg);
        _canvas.setPonchoMessage("Press Start Game to play !!");
        return true;
    }

    SudokuBoard&       getBoard()       { return _board; }
    const SudokuBoard& getBoard() const { return _board; }

    void refresh()      { _canvas.refresh(); }
    void refreshTheme() { _canvas.themeChanged(); }

    void updateBackgroundMusic()
    {
        _soundFocus1.stop();
        _soundFocus2.stop();
        _soundFocus3.stop();

        switch (_board.getOptions().music)
        {
        case BackgroundMusic::Focus1: _soundFocus1.play(true); break;
        case BackgroundMusic::Focus2: _soundFocus2.play(true); break;
        case BackgroundMusic::Focus3: _soundFocus3.play(true); break;
        default: break;
        }
    }

    void saveBoard() { saveBoardToFile(); }

    void resetTimer()
    {
        _timerRunning = false;
        _accumulatedSeconds = 0.0;
        _tickTimer.stop();
        _canvas.setTimeString("00:00");
        _canvas.setStartButtonTitle("Start Game");
        _hasSolution = false;
    }

    bool isNotesMode() const { return _canvas.isNotesMode(); }
};