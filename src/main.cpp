#include "Board.hpp"
#include "Types.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

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

}  // namespace

int main() {
    Board board;
    board.setupStandardPosition();

    std::cout << "Chess (text mode). Enter moves like \"e2 e4\" or \"e7 e8 Q\", or \"quit\" to exit.\n\n";

    while (!isGameOver(board.gameState())) {
        board.print(std::cout);
        std::cout << '\n' << colorName(board.currentTurn()) << " to move.\n";
        if (board.isInCheck(board.currentTurn())) {
            std::cout << "Check!\n";
        }
        std::cout << "Your move (e.g. e2 e4 or e7 e8 Q): ";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\nGoodbye.\n";
            break;
        }

        if (line == "quit" || line == "exit") {
            std::cout << "Goodbye.\n";
            break;
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
            std::cout << "Moved. Promoted to " << pieceTypeName(*promoted) << ".\n\n";
        } else {
            std::cout << "Moved.\n\n";
        }
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
    }

    return 0;
}
