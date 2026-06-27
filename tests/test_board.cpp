#include "Board.hpp"
#include "Types.hpp"

#include <gtest/gtest.h>
#include <sstream>

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

TEST(BoardTest, PrintMoveHistoryWhenEmpty) {
    Board board;
    board.setupStandardPosition();

    std::ostringstream out;
    board.printMoveHistory(out);
    EXPECT_EQ(out.str(), "No moves yet.");
}

TEST(BoardTest, PrintMoveHistoryListsMovesInOrder) {
    Board board;
    board.setupStandardPosition();

    const auto tryAlgebraic = [&](const char* from, const char* to) {
        const auto fromPos = Position::fromAlgebraic(from);
        const auto toPos = Position::fromAlgebraic(to);
        ASSERT_TRUE(fromPos.has_value());
        ASSERT_TRUE(toPos.has_value());
        ASSERT_FALSE(board.tryMove(*fromPos, *toPos).has_value());
    };

    tryAlgebraic("e2", "e4");
    tryAlgebraic("e7", "e5");
    tryAlgebraic("g1", "f3");

    std::ostringstream out;
    board.printMoveHistory(out);
    EXPECT_EQ(out.str(),
              "1. White e2 e4\n"
              "2. Black e7 e5\n"
              "3. White g1 f3");
}

TEST(BoardTest, RejectedMoveNotRecordedInHistory) {
    Board board;
    board.setupStandardPosition();

    const auto e2 = Position::fromAlgebraic("e2");
    const auto e5 = Position::fromAlgebraic("e5");
    ASSERT_TRUE(e2.has_value());
    ASSERT_TRUE(e5.has_value());

    EXPECT_TRUE(board.tryMove(*e2, *e5).has_value());
    EXPECT_TRUE(board.moveHistory().empty());
    EXPECT_FALSE(board.lastMove().has_value());

    std::ostringstream out;
    board.printMoveHistory(out);
    EXPECT_EQ(out.str(), "No moves yet.");
}

TEST(BoardTest, SetupStandardPositionClearsMoveHistory) {
    Board board;
    board.setupStandardPosition();

    const auto e2 = Position::fromAlgebraic("e2");
    const auto e4 = Position::fromAlgebraic("e4");
    ASSERT_TRUE(e2.has_value());
    ASSERT_TRUE(e4.has_value());
    ASSERT_FALSE(board.tryMove(*e2, *e4).has_value());
    ASSERT_EQ(board.moveHistory().size(), 1U);

    board.setupStandardPosition();
    EXPECT_TRUE(board.moveHistory().empty());

    std::ostringstream out;
    board.printMoveHistory(out);
    EXPECT_EQ(out.str(), "No moves yet.");
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

TEST(BoardTest, WhiteKingsideCastle) {
    Board board;
    board.setupStandardPosition();

    const auto tryAlgebraic = [&](const char* from, const char* to) {
        const auto fromPos = Position::fromAlgebraic(from);
        const auto toPos = Position::fromAlgebraic(to);
        ASSERT_TRUE(fromPos.has_value());
        ASSERT_TRUE(toPos.has_value());
        ASSERT_FALSE(board.tryMove(*fromPos, *toPos).has_value());
    };

    tryAlgebraic("e2", "e4");
    tryAlgebraic("e7", "e5");
    tryAlgebraic("g1", "f3");
    tryAlgebraic("b8", "c6");
    tryAlgebraic("f1", "c4");
    tryAlgebraic("g8", "f6");
    tryAlgebraic("e1", "g1");

    const auto g1 = Position::fromAlgebraic("g1");
    const auto f1 = Position::fromAlgebraic("f1");
    const auto h1 = Position::fromAlgebraic("h1");
    ASSERT_TRUE(g1.has_value());
    ASSERT_TRUE(f1.has_value());
    ASSERT_TRUE(h1.has_value());

    ASSERT_NE(board.pieceAt(*g1), nullptr);
    EXPECT_EQ(board.pieceAt(*g1)->type(), PieceType::King);
    ASSERT_NE(board.pieceAt(*f1), nullptr);
    EXPECT_EQ(board.pieceAt(*f1)->type(), PieceType::Rook);
    EXPECT_EQ(board.pieceAt(*h1), nullptr);
}

TEST(BoardTest, EnPassantCaptureRemovesPassedPawn) {
    Board board;
    board.setupStandardPosition();

    const auto tryAlgebraic = [&](const char* from, const char* to) {
        const auto fromPos = Position::fromAlgebraic(from);
        const auto toPos = Position::fromAlgebraic(to);
        ASSERT_TRUE(fromPos.has_value());
        ASSERT_TRUE(toPos.has_value());
        ASSERT_FALSE(board.tryMove(*fromPos, *toPos).has_value());
    };

    tryAlgebraic("e2", "e4");
    tryAlgebraic("c7", "c6");
    tryAlgebraic("e4", "e5");
    tryAlgebraic("d7", "d5");

    const auto target = board.enPassantTarget();
    ASSERT_TRUE(target.has_value());
    EXPECT_EQ(target->toAlgebraic(), "d6");

    tryAlgebraic("e5", "d6");

    const auto d5 = Position::fromAlgebraic("d5");
    const auto d6 = Position::fromAlgebraic("d6");
    ASSERT_TRUE(d5.has_value());
    ASSERT_TRUE(d6.has_value());
    EXPECT_EQ(board.pieceAt(*d5), nullptr);
    ASSERT_NE(board.pieceAt(*d6), nullptr);
    EXPECT_EQ(board.pieceAt(*d6)->type(), PieceType::Pawn);
    EXPECT_EQ(board.pieceAt(*d6)->color(), Color::White);
}
