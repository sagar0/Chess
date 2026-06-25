#pragma once

#include "Piece.hpp"
#include "Types.hpp"

#include <array>
#include <memory>
#include <optional>
#include <ostream>
#include <string>

class Board {
public:
    Board();

    void setupStandardPosition();
    void print(std::ostream& out) const;

    const Piece* pieceAt(Position pos) const;
    Color currentTurn() const { return currentTurn_; }

    [[nodiscard]] std::optional<std::string> tryMove(Position from, Position to);

    bool isEmpty(Position pos) const;
    bool isEnemy(Position pos, Color color) const;
    bool isAlly(Position pos, Color color) const;
    bool isPathClear(Position from, Position to) const;

private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid_{};
    Color currentTurn_{Color::White};

    void placePiece(PieceType type, Color color, Position pos);
};
