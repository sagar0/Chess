#include "pieces/King.hpp"

#include "Board.hpp"

#include <cstdlib>

King::King(Color color) : Piece(color) {}

char King::symbol() const {
    return color() == Color::White ? 'K' : 'k';
}

bool King::isValidMove(const Board& board, Position from, Position to) const {
    if (from == to || !Position::isOnBoard(to)) {
        return false;
    }

    const int rowDelta = std::abs(to.row - from.row);
    const int colDelta = std::abs(to.col - from.col);

    if (rowDelta == 0 && colDelta == 2) {
        return board.isCastlingStructureValid(from, to, color());
    }

    if (rowDelta > 1 || colDelta > 1) {
        return false;
    }
    if (board.isAlly(to, color())) {
        return false;
    }

    return true;
}
