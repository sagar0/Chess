#pragma once

#include <optional>
#include <string>

class Board;

class GameStore {
public:
    virtual ~GameStore() = default;

    [[nodiscard]] virtual bool hasSavedGame() const = 0;
    [[nodiscard]] virtual std::optional<std::string> save(const Board& board) = 0;
    [[nodiscard]] virtual std::optional<std::string> load(Board& board) = 0;
    virtual void clearSavedGame() = 0;
};
