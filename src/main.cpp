#include "Board.hpp"
#include "FileGameStore.hpp"
#include "Types.hpp"

#include <cctype>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

namespace {

constexpr const char* kDefaultSavePath = ".chess/saved_game.txt";

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

void initializeBoard(Board& board, GameStore& store) {
    if (store.hasSavedGame()) {
        if (promptYesNo("A saved game was found. Continue where you left off? (y/n): ")) {
            if (const auto error = store.load(board)) {
                std::cout << "Could not load saved game: " << *error << "\n";
                std::cout << "Starting a new game instead.\n\n";
                board.setupStandardPosition();
            } else {
                std::cout << "Loaded saved game.\n\n";
            }
            return;
        }

        store.clearSavedGame();
    }

    board.setupStandardPosition();
}

void offerSaveOnQuit(Board& board, GameStore& store) {
    if (board.gameState() != GameState::InProgress) {
        return;
    }

    if (!promptYesNo("Save game before quitting? (y/n): ")) {
        return;
    }

    if (const auto error = store.save(board)) {
        std::cout << "Could not save game: " << *error << '\n';
        return;
    }

    std::cout << "Game saved. You can continue it the next time you play.\n";
}

void runGameLoop(Board& board, GameStore& store) {
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
            offerSaveOnQuit(board, store);
            std::cout << "Goodbye.\n";
            break;
        }

        if (line == "quit" || line == "exit") {
            offerSaveOnQuit(board, store);
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
        store.clearSavedGame();
    }
}

}  // namespace

int main() {
    auto store = std::make_unique<FileGameStore>(kDefaultSavePath);
    Board board;
    initializeBoard(board, *store);
    runGameLoop(board, *store);
    return 0;
}
