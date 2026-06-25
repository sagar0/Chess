#include "pieces/Rook.hpp"

#include "Board.hpp"

#include <cstdlib>

Rook::Rook(Color color) : Piece(color) {}

char Rook::symbol() const {
    return color() == Color::White ? 'R' : 'r';
}

bool Rook::isValidMove(const Board& board, Position from, Position to) const {
    if (from == to || !Position::isOnBoard(to)) {
        return false;
    }

    const int rowDelta = to.row - from.row;
    const int colDelta = to.col - from.col;

    if (rowDelta != 0 && colDelta != 0) {
        return false;
    }
    if (rowDelta == 0 && colDelta == 0) {
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
