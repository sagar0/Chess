#pragma once

#include "Types.hpp"

#include <array>
#include <optional>
#include <string>
#include <vector>

struct SquareOccupant {
    PieceType type{PieceType::None};
    Color color{Color::White};
};

struct GameSnapshot {
    static constexpr int kFormatVersion = 1;

    Color currentTurn{Color::White};
    CastlingRights castlingRights{};
    std::optional<Position> enPassantTarget;
    std::array<std::array<SquareOccupant, 8>, 8> grid{};
    std::vector<Move> moveHistory;
};

[[nodiscard]] std::string serializeGameSnapshot(const GameSnapshot& snapshot);
[[nodiscard]] std::optional<std::string> deserializeGameSnapshot(const std::string& data,
                                                                   GameSnapshot& snapshot);
