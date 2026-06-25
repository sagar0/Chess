#pragma once

#include "Piece.hpp"
#include "Types.hpp"

class Queen : public Piece {
public:
    explicit Queen(Color color);

    PieceType type() const override { return PieceType::Queen; }
    char symbol() const override;
    bool isValidMove(const Board& board, Position from, Position to) const override;
};
