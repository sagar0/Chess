#include "pieces/Pawn.hpp"

#include "Board.hpp"

#include <cstdlib>

Pawn::Pawn(Color color) : Piece(color) {}

char Pawn::symbol() const {
    return color() == Color::White ? 'P' : 'p';
}

bool Pawn::isValidMove(const Board& board, Position from, Position to) const {
    if (from == to || !Position::isOnBoard(to)) {
        return false;
    }

    const int direction = color() == Color::White ? -1 : 1;
    const int startRow = color() == Color::White ? 6 : 1;
    const int rowDelta = to.row - from.row;
    const int colDelta = std::abs(to.col - from.col);

    if (colDelta == 0) {
        if (rowDelta != direction && !(from.row == startRow && rowDelta == 2 * direction)) {
            return false;
        }
        if (!board.isEmpty(to)) {
            return false;
        }
        if (std::abs(rowDelta) == 2 && !board.isEmpty({from.row + direction, from.col})) {
            return false;
        }
        return true;
    }

    if (colDelta == 1 && rowDelta == direction) {
        return board.isEnemy(to, color());
    }

    return false;
}
