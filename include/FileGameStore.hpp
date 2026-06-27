#pragma once

#include "GameStore.hpp"

#include <filesystem>
#include <string>

class FileGameStore : public GameStore {
public:
    explicit FileGameStore(std::string saveDirectory);

    [[nodiscard]] std::vector<std::string> listSavedGames() const override;
    [[nodiscard]] bool hasSavedGames() const override;
    [[nodiscard]] std::optional<std::string> validateGameName(
        const std::string& name) const override;
    [[nodiscard]] std::optional<std::string> save(const Board& board,
                                                  const std::string& name) override;
    [[nodiscard]] std::optional<std::string> load(Board& board,
                                                  const std::string& name) override;
    void deleteSavedGame(const std::string& name) override;

    [[nodiscard]] const std::string& saveDirectory() const { return saveDirectory_; }

private:
    std::string saveDirectory_;

    [[nodiscard]] std::filesystem::path pathForGame(const std::string& name) const;
};
