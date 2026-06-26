#pragma once

#include <cctype>
#include <optional>
#include <string>

enum class Color { White, Black };

enum class PieceType { Pawn, Rook, Knight, Bishop, Queen, King, None };

enum class GameState { InProgress, Checkmate, Stalemate };

struct Position {
    int row{};
    int col{};

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    bool operator!=(const Position& other) const { return !(*this == other); }

    static bool isOnBoard(const Position& pos) {
        return pos.row >= 0 && pos.row < 8 && pos.col >= 0 && pos.col < 8;
    }

    static std::optional<Position> fromAlgebraic(const std::string& sq) {
        if (sq.size() != 2) {
            return std::nullopt;
        }
        const char file = static_cast<char>(std::tolower(sq[0]));
        const char rank = sq[1];
        if (file < 'a' || file > 'h' || rank < '1' || rank > '8') {
            return std::nullopt;
        }
        return Position{7 - (rank - '1'), file - 'a'};
    }

    std::string toAlgebraic() const {
        const char file = static_cast<char>('a' + col);
        const char rank = static_cast<char>('1' + (7 - row));
        return std::string{file, rank};
    }
};

inline std::string colorName(Color color) {
    return color == Color::White ? "White" : "Black";
}
