#pragma once

#include "Piece.hpp"
#include "Types.hpp"

class Bishop : public Piece {
public:
    explicit Bishop(Color color);

    PieceType type() const override { return PieceType::Bishop; }
    char symbol() const override;
    bool isValidMove(const Board& board, Position from, Position to) const override;
};
