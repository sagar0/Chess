#include "Board.hpp"
#include "Types.hpp"

#include <iostream>
#include <sstream>
#include <string>

namespace {

std::optional<std::pair<Position, Position>> parseMove(const std::string& line) {
    std::istringstream input(line);
    std::string fromStr;
    std::string toStr;

    if (!(input >> fromStr >> toStr)) {
        return std::nullopt;
    }

    const auto from = Position::fromAlgebraic(fromStr);
    const auto to = Position::fromAlgebraic(toStr);

    if (!from.has_value() || !to.has_value()) {
        return std::nullopt;
    }

    return std::pair<Position, Position>{*from, *to};
}

}  // namespace

int main() {
    Board board;
    board.setupStandardPosition();

    std::cout << "Chess (text mode). Enter moves like \"e2 e4\", or \"quit\" to exit.\n\n";

    while (true) {
        board.print(std::cout);
        std::cout << '\n' << colorName(board.currentTurn()) << " to move.\n";
        std::cout << "Your move (e.g. e2 e4): ";
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
            std::cout << "Invalid input. Use two squares like \"e2 e4\".\n\n";
            continue;
        }

        const auto error = board.tryMove(move->first, move->second);
        if (error.has_value()) {
            std::cout << "Move rejected: " << *error << "\n\n";
            continue;
        }

        std::cout << "Moved.\n\n";
    }

    return 0;
}
