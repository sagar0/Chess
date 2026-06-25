#pragma once

#include "Piece.hpp"
#include "Types.hpp"

class Pawn : public Piece {
public:
    explicit Pawn(Color color);

    PieceType type() const override { return PieceType::Pawn; }
    char symbol() const override;
    bool isValidMove(const Board& board, Position from, Position to) const override;
};
