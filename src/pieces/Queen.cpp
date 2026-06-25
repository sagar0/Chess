#include "pieces/Queen.hpp"

#include "Board.hpp"

#include <cstdlib>

Queen::Queen(Color color) : Piece(color) {}

char Queen::symbol() const {
    return color() == Color::White ? 'Q' : 'q';
}

bool Queen::isValidMove(const Board& board, Position from, Position to) const {
    if (from == to || !Position::isOnBoard(to)) {
        return false;
    }

    const int rowDelta = to.row - from.row;
    const int colDelta = to.col - from.col;
    const int absRowDelta = std::abs(rowDelta);
    const int absColDelta = std::abs(colDelta);

    const bool straight = (rowDelta == 0 && colDelta != 0) || (colDelta == 0 && rowDelta != 0);
    const bool diagonal = absRowDelta == absColDelta && absRowDelta != 0;

    if (!straight && !diagonal) {
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
