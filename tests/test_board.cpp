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

TEST(BoardTest, RecordsMoveHistoryAfterLegalMove) {
    Board board;
    board.setupStandardPosition();
    EXPECT_TRUE(board.moveHistory().empty());
    EXPECT_FALSE(board.lastMove().has_value());

    const auto e2 = Position::fromAlgebraic("e2");
    const auto e4 = Position::fromAlgebraic("e4");
    ASSERT_TRUE(e2.has_value());
    ASSERT_TRUE(e4.has_value());

    ASSERT_FALSE(board.tryMove(*e2, *e4).has_value());
    ASSERT_EQ(board.moveHistory().size(), 1U);

    const auto last = board.lastMove();
    ASSERT_TRUE(last.has_value());
    EXPECT_EQ(last->color, Color::White);
    EXPECT_EQ(last->piece, PieceType::Pawn);
    EXPECT_EQ(last->from, *e2);
    EXPECT_EQ(last->to, *e4);
    EXPECT_FALSE(last->captured.has_value());
    EXPECT_EQ(last->toAlgebraic(), "e2 e4");
}

TEST(MoveTest, FormatsPromotionNotation) {
    const Move move{
        Color::White,
        {1, 2},
        {0, 2},
        PieceType::Pawn,
        std::nullopt,
        PieceType::Queen,
    };
    EXPECT_EQ(move.toAlgebraic(), "c7 c8 Q");
}
