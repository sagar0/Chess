#pragma once

#include "Types.hpp"

class Board;

class Piece {
public:
    virtual ~Piece() = default;

    Color color() const { return color_; }
    virtual PieceType type() const = 0;
    virtual char symbol() const = 0;

    virtual bool isValidMove(const Board& board, Position from, Position to) const = 0;

protected:
    explicit Piece(Color color) : color_(color) {}

private:
    Color color_;
};
