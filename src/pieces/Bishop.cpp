#include "pieces/Bishop.hpp"

#include "Board.hpp"

#include <cstdlib>

Bishop::Bishop(Color color) : Piece(color) {}

char Bishop::symbol() const {
    return color() == Color::White ? 'B' : 'b';
}

bool Bishop::isValidMove(const Board& board, Position from, Position to) const {
    if (from == to || !Position::isOnBoard(to)) {
        return false;
    }

    const int rowDelta = to.row - from.row;
    const int colDelta = to.col - from.col;

    if (std::abs(rowDelta) != std::abs(colDelta)) {
        return false;
    }
    if (!board.isPathClear(from, to)) {
        return false;
    }
    if (board.isAlly(to, color())) {
        return false;
    }

    return true;
}
