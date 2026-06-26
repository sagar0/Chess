#pragma once

#include <cctype>
#include <optional>
#include <string>
#include <vector>

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

inline char promotionChar(PieceType type) {
    switch (type) {
        case PieceType::Queen:
            return 'Q';
        case PieceType::Rook:
            return 'R';
        case PieceType::Bishop:
            return 'B';
        case PieceType::Knight:
            return 'N';
        default:
            return '?';
    }
}

struct Move {
    Color color{};
    Position from{};
    Position to{};
    PieceType piece{PieceType::None};
    std::optional<PieceType> captured;
    std::optional<PieceType> promotion;

    std::string toAlgebraic() const {
        std::string notation = from.toAlgebraic() + " " + to.toAlgebraic();
        if (promotion.has_value()) {
            notation.push_back(' ');
            notation.push_back(promotionChar(*promotion));
        }
        return notation;
    }
};

inline bool isPromotablePiece(PieceType type) {
    return type == PieceType::Queen || type == PieceType::Rook || type == PieceType::Knight ||
           type == PieceType::Bishop;
}

inline std::optional<PieceType> pieceTypeFromPromotionChar(char ch) {
    switch (static_cast<char>(std::toupper(ch))) {
        case 'Q':
            return PieceType::Queen;
        case 'R':
            return PieceType::Rook;
        case 'B':
            return PieceType::Bishop;
        case 'N':
            return PieceType::Knight;
        default:
            return std::nullopt;
    }
}

inline std::string pieceTypeName(PieceType type) {
    switch (type) {
        case PieceType::Queen:
            return "Queen";
        case PieceType::Rook:
            return "Rook";
        case PieceType::Bishop:
            return "Bishop";
        case PieceType::Knight:
            return "Knight";
        default:
            return "Piece";
    }
}
