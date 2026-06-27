#include "FileGameStore.hpp"

#include "Board.hpp"
#include "GameSnapshot.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

FileGameStore::FileGameStore(std::string path) : path_(std::move(path)) {}

bool FileGameStore::hasSavedGame() const {
    return std::filesystem::exists(path_);
}

std::optional<std::string> FileGameStore::save(const Board& board) {
    const std::string data = serializeGameSnapshot(board.captureSnapshot());

    const std::filesystem::path filePath(path_);
    if (filePath.has_parent_path()) {
        std::error_code error;
        std::filesystem::create_directories(filePath.parent_path(), error);
        if (error) {
            return "Could not create save directory.";
        }
    }

    std::ofstream out(path_, std::ios::trunc);
    if (!out) {
        return "Could not open save file for writing.";
    }
    out << data;
    if (!out) {
        return "Could not write save file.";
    }
    return std::nullopt;
}

std::optional<std::string> FileGameStore::load(Board& board) {
    std::ifstream in(path_);
    if (!in) {
        return "Could not open save file for reading.";
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    if (!in && !in.eof()) {
        return "Could not read save file.";
    }

    GameSnapshot snapshot;
    const auto error = deserializeGameSnapshot(buffer.str(), snapshot);
    if (error.has_value()) {
        return *error;
    }

    board.restoreSnapshot(snapshot);
    return std::nullopt;
}

void FileGameStore::clearSavedGame() {
    std::error_code error;
    std::filesystem::remove(path_, error);
}
