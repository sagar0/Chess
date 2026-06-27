#include "Board.hpp"
#include "FileGameStore.hpp"
#include "GameSnapshot.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::string uniqueTempDir(const std::string& name) {
    const auto tempDir = std::filesystem::temp_directory_path();
    return (tempDir / name).string();
}

void removeDirectoryIfExists(const std::string& path) {
    std::error_code error;
    std::filesystem::remove_all(path, error);
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

TEST(FileGameStoreTest, SaveLoadAndDeleteNamedGame) {
    const std::string directory = uniqueTempDir("chess_saves_test");
    removeDirectoryIfExists(directory);

    Board board;
    board.setupStandardPosition();
    playAlgebraic(board, "d2", "d4");
    playAlgebraic(board, "d7", "d5");

    FileGameStore store(directory);
    ASSERT_FALSE(store.hasSavedGames());
    ASSERT_FALSE(store.save(board, "opening").has_value());
    ASSERT_TRUE(store.hasSavedGames());

    const auto games = store.listSavedGames();
    ASSERT_EQ(games.size(), 1U);
    EXPECT_EQ(games[0], "opening");

    Board loaded;
    loaded.setupStandardPosition();
    ASSERT_FALSE(store.load(loaded, "opening").has_value());

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

    store.deleteSavedGame("opening");
    EXPECT_FALSE(store.hasSavedGames());
    removeDirectoryIfExists(directory);
}

TEST(FileGameStoreTest, SupportsMultipleNamedSaves) {
    const std::string directory = uniqueTempDir("chess_multi_saves_test");
    removeDirectoryIfExists(directory);

    FileGameStore store(directory);

    Board gameA;
    gameA.setupStandardPosition();
    playAlgebraic(gameA, "e2", "e4");
    ASSERT_FALSE(store.save(gameA, "e4-opening").has_value());

    Board gameB;
    gameB.setupStandardPosition();
    playAlgebraic(gameB, "d2", "d4");
    ASSERT_FALSE(store.save(gameB, "queens-gambit").has_value());

    const auto games = store.listSavedGames();
    ASSERT_EQ(games.size(), 2U);
    EXPECT_EQ(games[0], "e4-opening");
    EXPECT_EQ(games[1], "queens-gambit");

    Board loadedA;
    loadedA.setupStandardPosition();
    ASSERT_FALSE(store.load(loadedA, "e4-opening").has_value());
    ASSERT_EQ(loadedA.moveHistory().size(), 1U);
    EXPECT_EQ(loadedA.moveHistory().front().toAlgebraic(), "e2 e4");

    Board loadedB;
    loadedB.setupStandardPosition();
    ASSERT_FALSE(store.load(loadedB, "queens-gambit").has_value());
    ASSERT_EQ(loadedB.moveHistory().size(), 1U);
    EXPECT_EQ(loadedB.moveHistory().front().toAlgebraic(), "d2 d4");

    removeDirectoryIfExists(directory);
}

TEST(FileGameStoreTest, RejectsInvalidGameNames) {
    const std::string directory = uniqueTempDir("chess_name_validation_test");
    removeDirectoryIfExists(directory);

    FileGameStore store(directory);
    Board board;
    board.setupStandardPosition();

    EXPECT_TRUE(store.validateGameName("").has_value());
    EXPECT_TRUE(store.validateGameName("   ").has_value());
    EXPECT_TRUE(store.validateGameName("bad/name").has_value());
    EXPECT_TRUE(store.validateGameName("..").has_value());

    EXPECT_TRUE(store.save(board, "").has_value());
    EXPECT_TRUE(store.save(board, "bad/name").has_value());
    EXPECT_FALSE(store.hasSavedGames());

    removeDirectoryIfExists(directory);
}

TEST(FileGameStoreTest, LoadReportsMissingOrInvalidSaveFile) {
    const std::string directory = uniqueTempDir("chess_invalid_save_test");
    removeDirectoryIfExists(directory);

    FileGameStore store(directory);
    Board board;
    board.setupStandardPosition();

    EXPECT_TRUE(store.load(board, "missing-game").has_value());

    ASSERT_FALSE(store.save(board, "broken").has_value());
    const std::filesystem::path brokenPath =
        std::filesystem::path(directory) / "broken";
    {
        std::ofstream out(brokenPath, std::ios::trunc);
        out << "corrupted save data\n";
    }

    EXPECT_TRUE(store.load(board, "broken").has_value());

    removeDirectoryIfExists(directory);
}
