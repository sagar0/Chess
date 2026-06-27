#include "Board.hpp"
#include "FileGameStore.hpp"
#include "GameSnapshot.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::string uniqueTempPath(const std::string& name) {
    const auto tempDir = std::filesystem::temp_directory_path();
    return (tempDir / name).string();
}

void removeIfExists(const std::string& path) {
    std::error_code error;
    std::filesystem::remove(path, error);
}

void playAlgebraic(Board& board, const char* from, const char* to) {
    const auto fromPos = Position::fromAlgebraic(from);
    const auto toPos = Position::fromAlgebraic(to);
    ASSERT_TRUE(fromPos.has_value());
    ASSERT_TRUE(toPos.has_value());
    ASSERT_FALSE(board.tryMove(*fromPos, *toPos).has_value());
}

}  // namespace

TEST(GameSnapshotTest, RoundTripPreservesBoardState) {
    Board board;
    board.setupStandardPosition();
    playAlgebraic(board, "e2", "e4");
    playAlgebraic(board, "e7", "e5");
    playAlgebraic(board, "g1", "f3");

    const GameSnapshot original = board.captureSnapshot();
    const std::string serialized = serializeGameSnapshot(original);

    GameSnapshot restoredSnapshot;
    ASSERT_FALSE(deserializeGameSnapshot(serialized, restoredSnapshot).has_value());

    Board restored;
    restored.restoreSnapshot(restoredSnapshot);

    EXPECT_EQ(restored.currentTurn(), board.currentTurn());
    EXPECT_EQ(restored.moveHistory().size(), board.moveHistory().size());
    EXPECT_EQ(restored.enPassantTarget(), board.enPassantTarget());

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Position pos{row, col};
            const Piece* originalPiece = board.pieceAt(pos);
            const Piece* restoredPiece = restored.pieceAt(pos);
            if (originalPiece == nullptr) {
                EXPECT_EQ(restoredPiece, nullptr);
            } else {
                ASSERT_NE(restoredPiece, nullptr);
                EXPECT_EQ(originalPiece->type(), restoredPiece->type());
                EXPECT_EQ(originalPiece->color(), restoredPiece->color());
            }
        }
    }
}

TEST(GameSnapshotTest, RejectsInvalidSnapshotData) {
    GameSnapshot snapshot;
    EXPECT_TRUE(deserializeGameSnapshot("not a save file", snapshot).has_value());
}

TEST(FileGameStoreTest, SaveLoadAndClearPersistGame) {
    const std::string path = uniqueTempPath("chess_save_test.txt");
    removeIfExists(path);

    Board board;
    board.setupStandardPosition();
    playAlgebraic(board, "d2", "d4");
    playAlgebraic(board, "d7", "d5");

    FileGameStore store(path);
    ASSERT_FALSE(store.hasSavedGame());
    ASSERT_FALSE(store.save(board).has_value());
    ASSERT_TRUE(store.hasSavedGame());

    Board loaded;
    loaded.setupStandardPosition();
    ASSERT_FALSE(store.load(loaded).has_value());

    EXPECT_EQ(loaded.currentTurn(), board.currentTurn());
    EXPECT_EQ(loaded.moveHistory().size(), board.moveHistory().size());

    const auto d4 = Position::fromAlgebraic("d4");
    const auto d5 = Position::fromAlgebraic("d5");
    ASSERT_TRUE(d4.has_value());
    ASSERT_TRUE(d5.has_value());
    ASSERT_NE(loaded.pieceAt(*d4), nullptr);
    ASSERT_NE(loaded.pieceAt(*d5), nullptr);
    EXPECT_EQ(loaded.pieceAt(*d4)->color(), Color::White);
    EXPECT_EQ(loaded.pieceAt(*d5)->color(), Color::Black);

    store.clearSavedGame();
    EXPECT_FALSE(store.hasSavedGame());
    removeIfExists(path);
}

TEST(FileGameStoreTest, LoadReportsInvalidSaveFile) {
    const std::string path = uniqueTempPath("chess_invalid_save_test.txt");
    removeIfExists(path);

    {
        std::ofstream out(path);
        out << "corrupted save data\n";
    }

    FileGameStore store(path);
    Board board;
    board.setupStandardPosition();
    EXPECT_TRUE(store.load(board).has_value());

    removeIfExists(path);
}
