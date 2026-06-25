#pragma once

#include "Piece.hpp"
#include "Types.hpp"

class Rook : public Piece {
public:
    explicit Rook(Color color);

    PieceType type() const override { return PieceType::Rook; }
    char symbol() const override;
    bool isValidMove(const Board& board, Position from, Position to) const override;
};
