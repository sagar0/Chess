#include "Board.hpp"

#include "pieces/Bishop.hpp"
#include "pieces/King.hpp"
#include "pieces/Knight.hpp"
#include "pieces/Pawn.hpp"
#include "pieces/Queen.hpp"
#include "pieces/Rook.hpp"

#include <utility>

namespace {

std::unique_ptr<Piece> makePiece(PieceType type, Color color) {
    switch (type) {
        case PieceType::Pawn:
            return std::make_unique<Pawn>(color);
        case PieceType::Rook:
            return std::make_unique<Rook>(color);
        case PieceType::Knight:
            return std::make_unique<Knight>(color);
        case PieceType::Bishop:
            return std::make_unique<Bishop>(color);
        case PieceType::Queen:
            return std::make_unique<Queen>(color);
        case PieceType::King:
            return std::make_unique<King>(color);
        case PieceType::None:
            break;
    }
    return nullptr;
}

}  // namespace

Board::Board() = default;

void Board::placePiece(PieceType type, Color color, Position pos) {
    grid_[static_cast<std::size_t>(pos.row)][static_cast<std::size_t>(pos.col)] =
        makePiece(type, color);
}

void Board::setupStandardPosition() {
    for (auto& row : grid_) {
        for (auto& square : row) {
            square.reset();
        }
    }

    const PieceType backRank[] = {
        PieceType::Rook,   PieceType::Knight, PieceType::Bishop, PieceType::Queen,
        PieceType::King,   PieceType::Bishop, PieceType::Knight, PieceType::Rook,
    };

    for (int col = 0; col < 8; ++col) {
        placePiece(backRank[col], Color::Black, {0, col});
        placePiece(PieceType::Pawn, Color::Black, {1, col});
        placePiece(PieceType::Pawn, Color::White, {6, col});
        placePiece(backRank[col], Color::White, {7, col});
    }

    currentTurn_ = Color::White;
}

void Board::print(std::ostream& out) const {
    out << "  a b c d e f g h\n";
    for (int row = 0; row < 8; ++row) {
        out << (8 - row) << ' ';
        for (int col = 0; col < 8; ++col) {
            const Piece* piece = pieceAt({row, col});
            if (piece != nullptr) {
                out << piece->symbol();
            } else {
                out << '.';
            }
            if (col < 7) {
                out << ' ';
            }
        }
        out << ' ' << (8 - row) << '\n';
    }
    out << "  a b c d e f g h\n";
}

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

std::optional<std::string> Board::tryMove(Position from, Position to) {
    if (!Position::isOnBoard(from) || !Position::isOnBoard(to)) {
        return "Square is off the board.";
    }
    if (from == to) {
        return "From and to squares must differ.";
    }

    const Piece* movingPiece = pieceAt(from);
    if (movingPiece == nullptr) {
        return "No piece on the from-square.";
    }
    if (movingPiece->color() != currentTurn_) {
        return "It is not that piece's turn.";
    }
    if (isAlly(to, currentTurn_)) {
        return "Cannot capture your own piece.";
    }
    if (!movingPiece->isValidMove(*this, from, to)) {
        return "Illegal move for that piece.";
    }

    grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] =
        std::move(grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)]);
    grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] = nullptr;

    currentTurn_ = currentTurn_ == Color::White ? Color::Black : Color::White;
    return std::nullopt;
}
