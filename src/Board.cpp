#include "Board.hpp"

const Piece* Board::pieceAt(Position pos) const {
    if (!Position::isOnBoard(pos)) {
        return nullptr;
    }
    return grid_[static_cast<std::size_t>(pos.row)][static_cast<std::size_t>(pos.col)].get();
}

bool Board::isEmpty(Position pos) const {
    return pieceAt(pos) == nullptr;
}

bool Board::isEnemy(Position pos, Color color) const {
    const Piece* piece = pieceAt(pos);
    return piece != nullptr && piece->color() != color;
}

bool Board::isAlly(Position pos, Color color) const {
    const Piece* piece = pieceAt(pos);
    return piece != nullptr && piece->color() == color;
}

bool Board::isPathClear(Position from, Position to) const {
    const int rowStep = (to.row > from.row) ? 1 : (to.row < from.row ? -1 : 0);
    const int colStep = (to.col > from.col) ? 1 : (to.col < from.col ? -1 : 0);

    int row = from.row + rowStep;
    int col = from.col + colStep;

    while (row != to.row || col != to.col) {
        if (!isEmpty({row, col})) {
            return false;
        }
        row += rowStep;
        col += colStep;
    }

    return true;
}
