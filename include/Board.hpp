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

    [[nodiscard]] std::optional<std::string> tryMove(Position from,
                                                     Position to,
                                                     std::optional<PieceType> promotion = std::nullopt);

    std::optional<Position> findKing(Color color) const;
    bool isSquareAttacked(Position sq, Color byColor) const;
    bool isInCheck(Color color) const;
    bool hasLegalMoves(Color color) const;
    GameState gameState() const;
    std::optional<Color> winner() const;
    std::optional<PieceType> lastPromotion() const { return lastPromotion_; }

    bool isEmpty(Position pos) const;
    bool isEnemy(Position pos, Color color) const;
    bool isAlly(Position pos, Color color) const;
    bool isPathClear(Position from, Position to) const;

private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid_{};
    Color currentTurn_{Color::White};
    std::optional<PieceType> lastPromotion_;

    void placePiece(PieceType type, Color color, Position pos);
    bool isPseudoLegal(Position from, Position to, Color color) const;
    bool wouldLeaveKingInCheck(Position from,
                               Position to,
                               Color color,
                               std::optional<PieceType> promotion = std::nullopt) const;
    static bool isPawnPromotionSquare(Color color, int row);
    static std::optional<PieceType> resolvePromotion(const Piece* movingPiece,
                                                     Position to,
                                                     std::optional<PieceType> promotion);
};
