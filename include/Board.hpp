#pragma once

#include "Piece.hpp"
#include "Types.hpp"

#include <array>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

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

    const std::vector<Move>& moveHistory() const { return moveHistory_; }
    std::optional<Move> lastMove() const;
    void printLastMove(std::ostream& out) const;

    bool isEmpty(Position pos) const;
    bool isEnemy(Position pos, Color color) const;
    bool isAlly(Position pos, Color color) const;
    bool isPathClear(Position from, Position to) const;
    std::optional<Position> enPassantTarget() const { return enPassantTarget_; }
    bool isCastlingStructureValid(Position from, Position to, Color color) const;

private:
    std::array<std::array<std::unique_ptr<Piece>, 8>, 8> grid_{};
    Color currentTurn_{Color::White};
    std::optional<PieceType> lastPromotion_;
    std::vector<Move> moveHistory_;
    CastlingRights castlingRights_{};
    std::optional<Position> enPassantTarget_;

    void placePiece(PieceType type, Color color, Position pos);
    bool isCastlingMove(Position from, Position to, Color color) const;
    bool canCastle(Position from, Position to, Color color) const;
    void applyCastling(Position from, Position to);
    void updateCastlingRights(Position from, Position to, const Piece* movingPiece);
    void updateEnPassantTarget(Position from, Position to, const Piece* movingPiece);
    std::optional<Position> enPassantCapturedSquare(Position to, Color color) const;
    bool isEnPassantCapture(Position from, Position to, Color color) const;
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
