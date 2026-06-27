#include "Board.hpp"
#include "FileGameStore.hpp"
#include "Types.hpp"

#include <cctype>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {

constexpr const char* kSaveDirectory = ".chess/saves";

struct ParsedMove {
    Position from;
    Position to;
    std::optional<PieceType> promotion;
};

std::optional<ParsedMove> parseMove(const std::string& line) {
    std::istringstream input(line);
    std::string fromStr;
    std::string toStr;
    std::string promotionStr;

    if (!(input >> fromStr >> toStr)) {
        return std::nullopt;
    }

    const auto from = Position::fromAlgebraic(fromStr);
    const auto to = Position::fromAlgebraic(toStr);

    if (!from.has_value() || !to.has_value()) {
        return std::nullopt;
    }

    std::optional<PieceType> promotion;
    if (input >> promotionStr) {
        if (promotionStr.size() != 1) {
            return std::nullopt;
        }
        promotion = pieceTypeFromPromotionChar(promotionStr[0]);
        if (!promotion.has_value()) {
            return std::nullopt;
        }
    }

    return ParsedMove{*from, *to, promotion};
}

bool isGameOver(GameState state) {
    return state == GameState::Checkmate || state == GameState::Stalemate;
}

bool promptYesNo(const std::string& question) {
    while (true) {
        std::cout << question;
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            return false;
        }

        if (line.empty()) {
            continue;
        }

        const char answer = static_cast<char>(std::tolower(static_cast<unsigned char>(line[0])));
        if (answer == 'y') {
            return true;
        }
        if (answer == 'n') {
            return false;
        }

        std::cout << "Please answer y or n.\n";
    }
}

std::optional<std::string> promptGameName(GameStore& store) {
    while (true) {
        std::cout << "Enter a name for this game: ";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            return std::nullopt;
        }

        if (const auto error = store.validateGameName(line)) {
            std::cout << *error << '\n';
            continue;
        }

        const auto start = line.find_first_not_of(" \t\r\n");
        const auto end = line.find_last_not_of(" \t\r\n");
        return line.substr(start, end - start + 1);
    }
}

std::optional<std::string> promptSelectSavedGame(GameStore& store) {
    const std::vector<std::string> games = store.listSavedGames();
    if (games.empty()) {
        return std::nullopt;
    }

    std::cout << "Saved games:\n";
    for (std::size_t i = 0; i < games.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << games[i] << '\n';
    }
    std::cout << "\nEnter a number or name to continue, or press Enter for a new game: ";
    std::cout.flush();

    std::string line;
    if (!std::getline(std::cin, line)) {
        return std::nullopt;
    }

    if (line.empty()) {
        return std::nullopt;
    }

    try {
        const std::size_t index = static_cast<std::size_t>(std::stoul(line));
        if (index >= 1 && index <= games.size()) {
            return games[index - 1];
        }
    } catch (const std::exception&) {
        // Not a number; try matching by name below.
    }

    for (const std::string& game : games) {
        if (game == line) {
            return game;
        }
    }

    std::cout << "No saved game matched \"" << line << "\". Starting a new game.\n\n";
    return std::nullopt;
}

void initializeBoard(Board& board, GameStore& store, std::optional<std::string>& activeGameName) {
    activeGameName.reset();

    if (!store.hasSavedGames()) {
        board.setupStandardPosition();
        return;
    }

    const std::optional<std::string> selected = promptSelectSavedGame(store);
    if (!selected.has_value()) {
        board.setupStandardPosition();
        return;
    }

    if (const auto error = store.load(board, *selected)) {
        std::cout << "Could not load \"" << *selected << "\": " << *error << "\n";
        std::cout << "Starting a new game instead.\n\n";
        board.setupStandardPosition();
        return;
    }

    activeGameName = *selected;
    std::cout << "Loaded \"" << *selected << "\".\n\n";
}

void offerSaveOnQuit(Board& board,
                     GameStore& store,
                     std::optional<std::string>& activeGameName) {
    if (board.gameState() != GameState::InProgress) {
        return;
    }

    if (!promptYesNo("Save game before quitting? (y/n): ")) {
        return;
    }

    const std::optional<std::string> name = promptGameName(store);
    if (!name.has_value()) {
        return;
    }

    if (const auto error = store.save(board, *name)) {
        std::cout << "Could not save game: " << *error << '\n';
        return;
    }

    activeGameName = *name;
    std::cout << "Game saved as \"" << *name << "\". You can continue it the next time you play.\n";
}

void runGameLoop(Board& board, GameStore& store, std::optional<std::string>& activeGameName) {
    std::cout << "Chess (text mode). Enter moves like \"e2 e4\" or \"e7 e8 Q\", "
                 "\"history\" to list moves, or \"quit\" to exit.\n\n";

    while (!isGameOver(board.gameState())) {
        board.print(std::cout);
        std::cout << '\n' << colorName(board.currentTurn()) << " to move.\n";
        if (board.isInCheck(board.currentTurn())) {
            std::cout << "Check!\n";
        }
        board.printLastMove(std::cout);
        if (board.lastMove().has_value()) {
            std::cout << '\n';
        }
        std::cout << "Your move (e.g. e2 e4 or e7 e8 Q): ";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            offerSaveOnQuit(board, store, activeGameName);
            std::cout << "Goodbye.\n";
            break;
        }

        if (line == "quit" || line == "exit") {
            offerSaveOnQuit(board, store, activeGameName);
            std::cout << "Goodbye.\n";
            break;
        }
        if (line == "history") {
            board.printMoveHistory(std::cout);
            std::cout << "\n\n";
            continue;
        }
        if (line.empty()) {
            continue;
        }

        const auto move = parseMove(line);
        if (!move.has_value()) {
            std::cout << "Invalid input. Use two squares like \"e2 e4\", optionally \"e7 e8 Q\".\n\n";
            continue;
        }

        const auto error = board.tryMove(move->from, move->to, move->promotion);
        if (error.has_value()) {
            std::cout << "Move rejected: " << *error << "\n\n";
            continue;
        }

        const auto promoted = board.lastPromotion();
        if (promoted.has_value()) {
            std::cout << "Moved. Promoted to " << pieceTypeName(*promoted) << ".\n";
        } else {
            std::cout << "Moved.\n";
        }
        board.printLastMove(std::cout);
        std::cout << "\n\n";
    }

    if (isGameOver(board.gameState())) {
        board.print(std::cout);
        std::cout << '\n';
        if (board.gameState() == GameState::Checkmate) {
            const auto winner = board.winner();
            if (winner.has_value()) {
                std::cout << "Checkmate! " << colorName(*winner) << " wins.\n";
            }
        } else {
            std::cout << "Stalemate! The game is a draw.\n";
        }
        if (activeGameName.has_value()) {
            store.deleteSavedGame(*activeGameName);
            activeGameName.reset();
        }
    }
}

}  // namespace

int main() {
    auto store = std::make_unique<FileGameStore>(kSaveDirectory);
    std::optional<std::string> activeGameName;
    Board board;
    initializeBoard(board, *store, activeGameName);
    runGameLoop(board, *store, activeGameName);
    return 0;
}
