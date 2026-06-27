#pragma once

#include "GameStore.hpp"

#include <string>

class FileGameStore : public GameStore {
public:
    explicit FileGameStore(std::string path);

    [[nodiscard]] bool hasSavedGame() const override;
    [[nodiscard]] std::optional<std::string> save(const Board& board) override;
    [[nodiscard]] std::optional<std::string> load(Board& board) override;
    void clearSavedGame() override;

    [[nodiscard]] const std::string& path() const { return path_; }

private:
    std::string path_;
};
