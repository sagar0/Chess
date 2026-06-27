#pragma once

#include <optional>
#include <string>
#include <vector>

class Board;

class GameStore {
public:
    virtual ~GameStore() = default;

    [[nodiscard]] virtual std::vector<std::string> listSavedGames() const = 0;
    [[nodiscard]] virtual bool hasSavedGames() const = 0;
    [[nodiscard]] virtual std::optional<std::string> validateGameName(
        const std::string& name) const = 0;
    [[nodiscard]] virtual std::optional<std::string> save(const Board& board,
                                                           const std::string& name) = 0;
    [[nodiscard]] virtual std::optional<std::string> load(Board& board,
                                                          const std::string& name) = 0;
    virtual void deleteSavedGame(const std::string& name) = 0;
};
