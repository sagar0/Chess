#pragma once

#include "Piece.hpp"
#include "Types.hpp"

class Knight : public Piece {
public:
    explicit Knight(Color color);

    PieceType type() const override { return PieceType::Knight; }
    char symbol() const override;
    bool isValidMove(const Board& board, Position from, Position to) const override;
};
