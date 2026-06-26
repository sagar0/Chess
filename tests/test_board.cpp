#include "Board.hpp"
#include "Types.hpp"

#include <gtest/gtest.h>

namespace {

int countPieces(const Board& board) {
    int count = 0;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if (board.pieceAt({row, col}) != nullptr) {
                ++count;
            }
        }
    }
    return count;
}

}  // namespace

TEST(BoardTest, StandardPositionHasThirtyTwoPieces) {
    Board board;
    board.setupStandardPosition();
    EXPECT_EQ(countPieces(board), 32);
}

TEST(BoardTest, WhiteToMoveAfterStandardSetup) {
    Board board;
    board.setupStandardPosition();
    EXPECT_EQ(board.currentTurn(), Color::White);
}

TEST(BoardTest, StandardPositionPlacesWhiteKingOnE1) {
    Board board;
    board.setupStandardPosition();

    const auto e1 = Position::fromAlgebraic("e1");
    ASSERT_TRUE(e1.has_value());

    const Piece* king = board.pieceAt(*e1);
    ASSERT_NE(king, nullptr);
    EXPECT_EQ(king->type(), PieceType::King);
    EXPECT_EQ(king->color(), Color::White);
}

TEST(PositionTest, AlgebraicRoundTrip) {
    const auto pos = Position::fromAlgebraic("e4");
    ASSERT_TRUE(pos.has_value());
    EXPECT_EQ(*pos, (Position{4, 4}));
    EXPECT_EQ(pos->toAlgebraic(), "e4");
}
