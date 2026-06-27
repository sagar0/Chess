#include "FileGameStore.hpp"

#include "Board.hpp"
#include "GameSnapshot.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

std::string trim(const std::string& text) {
    const auto start = text.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return {};
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(start, end - start + 1);
}

}  // namespace

FileGameStore::FileGameStore(std::string saveDirectory)
    : saveDirectory_(std::move(saveDirectory)) {}

std::filesystem::path FileGameStore::pathForGame(const std::string& name) const {
    return std::filesystem::path(saveDirectory_) / name;
}

std::vector<std::string> FileGameStore::listSavedGames() const {
    std::vector<std::string> games;
    const std::filesystem::path directory(saveDirectory_);
    if (!std::filesystem::exists(directory)) {
        return games;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            games.push_back(entry.path().filename().string());
        }
    }

    std::sort(games.begin(), games.end());
    return games;
}

bool FileGameStore::hasSavedGames() const {
    return !listSavedGames().empty();
}

std::optional<std::string> FileGameStore::validateGameName(const std::string& name) const {
    const std::string trimmed = trim(name);
    if (trimmed.empty()) {
        return "Game name cannot be empty.";
    }
    if (trimmed.size() > 64) {
        return "Game name is too long (64 characters max).";
    }
    if (trimmed == "." || trimmed == "..") {
        return "Invalid game name.";
    }
    for (const char ch : trimmed) {
        if (ch == '/' || ch == '\\') {
            return "Game name cannot contain path separators.";
        }
    }
    return std::nullopt;
}

std::optional<std::string> FileGameStore::save(const Board& board, const std::string& name) {
    if (const auto nameError = validateGameName(name)) {
        return nameError;
    }

    const std::string trimmed = trim(name);
    const std::filesystem::path filePath = pathForGame(trimmed);

    std::error_code error;
    std::filesystem::create_directories(filePath.parent_path(), error);
    if (error) {
        return "Could not create save directory.";
    }

    std::ofstream out(filePath, std::ios::trunc);
    if (!out) {
        return "Could not open save file for writing.";
    }
    out << serializeGameSnapshot(board.captureSnapshot());
    if (!out) {
        return "Could not write save file.";
    }
    return std::nullopt;
}

std::optional<std::string> FileGameStore::load(Board& board, const std::string& name) {
    if (const auto nameError = validateGameName(name)) {
        return nameError;
    }

    const std::filesystem::path filePath = pathForGame(trim(name));
    if (!std::filesystem::exists(filePath)) {
        return "Saved game not found.";
    }

    std::ifstream in(filePath);
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

void FileGameStore::deleteSavedGame(const std::string& name) {
    if (validateGameName(name).has_value()) {
        return;
    }

    std::error_code error;
    std::filesystem::remove(pathForGame(trim(name)), error);
}
