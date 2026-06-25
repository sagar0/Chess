#include "pieces/Knight.hpp"

#include "Board.hpp"

#include <cstdlib>

Knight::Knight(Color color) : Piece(color) {}

char Knight::symbol() const {
    return color() == Color::White ? 'N' : 'n';
}

bool Knight::isValidMove(const Board& board, Position from, Position to) const {
    if (from == to || !Position::isOnBoard(to)) {
        return false;
    }

    const int rowDelta = std::abs(to.row - from.row);
    const int colDelta = std::abs(to.col - from.col);

    if (!((rowDelta == 2 && colDelta == 1) || (rowDelta == 1 && colDelta == 2))) {
        return false;
    }
    if (board.isAlly(to, color())) {
        return false;
    }

    return true;
}
