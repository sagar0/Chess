#include "GameSnapshot.hpp"

#include <cctype>
#include <sstream>
#include <string_view>

namespace {

constexpr std::string_view kHeader = "chess-save";

char pieceToSymbol(PieceType type, Color color) {
    char ch = '?';
    switch (type) {
        case PieceType::Pawn:
            ch = 'P';
            break;
        case PieceType::Rook:
            ch = 'R';
            break;
        case PieceType::Knight:
            ch = 'N';
            break;
        case PieceType::Bishop:
            ch = 'B';
            break;
        case PieceType::Queen:
            ch = 'Q';
            break;
        case PieceType::King:
            ch = 'K';
            break;
        case PieceType::None:
            return '.';
    }
    return color == Color::White ? ch : static_cast<char>(std::tolower(ch));
}

std::optional<SquareOccupant> occupantFromSymbol(char ch) {
    if (ch == '.') {
        return SquareOccupant{};
    }

    const Color color = std::isupper(static_cast<unsigned char>(ch)) ? Color::White : Color::Black;
    switch (static_cast<char>(std::tolower(static_cast<unsigned char>(ch)))) {
        case 'p':
            return SquareOccupant{PieceType::Pawn, color};
        case 'r':
            return SquareOccupant{PieceType::Rook, color};
        case 'n':
            return SquareOccupant{PieceType::Knight, color};
        case 'b':
            return SquareOccupant{PieceType::Bishop, color};
        case 'q':
            return SquareOccupant{PieceType::Queen, color};
        case 'k':
            return SquareOccupant{PieceType::King, color};
        default:
            return std::nullopt;
    }
}

std::string colorToken(Color color) {
    return color == Color::White ? "white" : "black";
}

std::optional<Color> colorFromToken(std::string_view token) {
    if (token == "white") {
        return Color::White;
    }
    if (token == "black") {
        return Color::Black;
    }
    return std::nullopt;
}

std::string pieceTypeToken(PieceType type) {
    switch (type) {
        case PieceType::Pawn:
            return "pawn";
        case PieceType::Rook:
            return "rook";
        case PieceType::Knight:
            return "knight";
        case PieceType::Bishop:
            return "bishop";
        case PieceType::Queen:
            return "queen";
        case PieceType::King:
            return "king";
        case PieceType::None:
            return "none";
    }
    return "none";
}

std::optional<PieceType> pieceTypeFromToken(std::string_view token) {
    if (token == "pawn") {
        return PieceType::Pawn;
    }
    if (token == "rook") {
        return PieceType::Rook;
    }
    if (token == "knight") {
        return PieceType::Knight;
    }
    if (token == "bishop") {
        return PieceType::Bishop;
    }
    if (token == "queen") {
        return PieceType::Queen;
    }
    if (token == "king") {
        return PieceType::King;
    }
    return std::nullopt;
}

std::optional<PieceType> optionalPieceTypeFromToken(const std::string& token) {
    if (token == "-") {
        return std::nullopt;
    }
    return pieceTypeFromToken(token);
}

std::string optionalPieceTypeToken(const std::optional<PieceType>& type) {
    if (!type.has_value()) {
        return "-";
    }
    return pieceTypeToken(*type);
}

bool parseBoolToken(const std::string& token, bool& value) {
    if (token == "1" || token == "true") {
        value = true;
        return true;
    }
    if (token == "0" || token == "false") {
        value = false;
        return true;
    }
    return false;
}

std::string trim(std::string_view text) {
    const auto start = text.find_first_not_of(" \t\r\n");
    if (start == std::string_view::npos) {
        return {};
    }
    const auto end = text.find_last_not_of(" \t\r\n");
    return std::string{text.substr(start, end - start + 1)};
}

std::vector<std::string> splitLine(const std::string& line) {
    std::istringstream input(line);
    std::vector<std::string> tokens;
    std::string token;
    while (input >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

}  // namespace

std::string serializeGameSnapshot(const GameSnapshot& snapshot) {
    std::ostringstream out;
    out << kHeader << ' ' << GameSnapshot::kFormatVersion << '\n';
    out << "turn " << colorToken(snapshot.currentTurn) << '\n';
    out << "castling "
        << (snapshot.castlingRights.whiteKingside ? '1' : '0') << ' '
        << (snapshot.castlingRights.whiteQueenside ? '1' : '0') << ' '
        << (snapshot.castlingRights.blackKingside ? '1' : '0') << ' '
        << (snapshot.castlingRights.blackQueenside ? '1' : '0') << '\n';
    out << "enpassant "
        << (snapshot.enPassantTarget.has_value() ? snapshot.enPassantTarget->toAlgebraic() : "-")
        << '\n';
    out << "board\n";
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const SquareOccupant& square = snapshot.grid[static_cast<std::size_t>(row)]
                                                      [static_cast<std::size_t>(col)];
            out << pieceToSymbol(square.type, square.color);
        }
        out << '\n';
    }
    out << "moves " << snapshot.moveHistory.size() << '\n';
    for (const Move& move : snapshot.moveHistory) {
        out << "move " << colorToken(move.color) << ' ' << pieceTypeToken(move.piece) << ' '
            << move.from.toAlgebraic() << ' ' << move.to.toAlgebraic() << ' '
            << optionalPieceTypeToken(move.captured) << ' '
            << optionalPieceTypeToken(move.promotion) << '\n';
    }
    return out.str();
}

std::optional<std::string> deserializeGameSnapshot(const std::string& data,
                                                     GameSnapshot& snapshot) {
    std::istringstream input(data);
    std::string line;

    if (!std::getline(input, line)) {
        return "Saved game is empty.";
    }

    const auto headerTokens = splitLine(trim(line));
    if (headerTokens.size() != 2 || headerTokens[0] != kHeader) {
        return "Saved game has an invalid header.";
    }
    if (headerTokens[1] != std::to_string(GameSnapshot::kFormatVersion)) {
        return "Unsupported saved game format version.";
    }

    enum class Section { None, Board, Moves };
    Section section = Section::None;
    int boardRow = 0;
    std::size_t expectedMoves = 0;
    GameSnapshot parsed{};

    while (std::getline(input, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        if (section == Section::Board) {
            if (line.size() != 8) {
                return "Saved game board row has invalid length.";
            }
            for (int col = 0; col < 8; ++col) {
                const auto occupant = occupantFromSymbol(line[static_cast<std::size_t>(col)]);
                if (!occupant.has_value()) {
                    return "Saved game contains an invalid board symbol.";
                }
                parsed.grid[static_cast<std::size_t>(boardRow)][static_cast<std::size_t>(col)] =
                    *occupant;
            }
            ++boardRow;
            if (boardRow == 8) {
                section = Section::None;
            }
            continue;
        }

        if (section == Section::Moves) {
            const auto tokens = splitLine(line);
            if (tokens.size() != 7 || tokens[0] != "move") {
                return "Saved game contains an invalid move record.";
            }

            const auto color = colorFromToken(tokens[1]);
            const auto piece = pieceTypeFromToken(tokens[2]);
            const auto from = Position::fromAlgebraic(tokens[3]);
            const auto to = Position::fromAlgebraic(tokens[4]);
            const auto captured = optionalPieceTypeFromToken(tokens[5]);
            const auto promotion = optionalPieceTypeFromToken(tokens[6]);
            if (!color.has_value() || !piece.has_value() || !from.has_value() ||
                !to.has_value()) {
                return "Saved game contains an invalid move record.";
            }

            parsed.moveHistory.push_back(
                Move{*color, *from, *to, *piece, captured, promotion});
            if (parsed.moveHistory.size() == expectedMoves) {
                section = Section::None;
            }
            continue;
        }

        const auto tokens = splitLine(line);
        if (tokens.empty()) {
            continue;
        }

        if (tokens[0] == "turn") {
            if (tokens.size() != 2) {
                return "Saved game has an invalid turn line.";
            }
            const auto turn = colorFromToken(tokens[1]);
            if (!turn.has_value()) {
                return "Saved game has an invalid turn.";
            }
            parsed.currentTurn = *turn;
            continue;
        }

        if (tokens[0] == "castling") {
            if (tokens.size() != 5) {
                return "Saved game has an invalid castling line.";
            }
            if (!parseBoolToken(tokens[1], parsed.castlingRights.whiteKingside) ||
                !parseBoolToken(tokens[2], parsed.castlingRights.whiteQueenside) ||
                !parseBoolToken(tokens[3], parsed.castlingRights.blackKingside) ||
                !parseBoolToken(tokens[4], parsed.castlingRights.blackQueenside)) {
                return "Saved game has invalid castling rights.";
            }
            continue;
        }

        if (tokens[0] == "enpassant") {
            if (tokens.size() != 2) {
                return "Saved game has an invalid en passant line.";
            }
            if (tokens[1] == "-") {
                parsed.enPassantTarget.reset();
            } else {
                const auto target = Position::fromAlgebraic(tokens[1]);
                if (!target.has_value()) {
                    return "Saved game has an invalid en passant square.";
                }
                parsed.enPassantTarget = *target;
            }
            continue;
        }

        if (tokens[0] == "board") {
            section = Section::Board;
            boardRow = 0;
            continue;
        }

        if (tokens[0] == "moves") {
            if (tokens.size() != 2) {
                return "Saved game has an invalid moves header.";
            }
            try {
                expectedMoves = static_cast<std::size_t>(std::stoul(tokens[1]));
            } catch (const std::exception&) {
                return "Saved game has an invalid move count.";
            }
            section = Section::Moves;
            continue;
        }

        return "Saved game contains an unknown record.";
    }

    if (section != Section::None) {
        return "Saved game is incomplete.";
    }
    if (boardRow != 8) {
        return "Saved game board is incomplete.";
    }
    if (parsed.moveHistory.size() != expectedMoves) {
        return "Saved game move count does not match records.";
    }

    snapshot = std::move(parsed);
    return std::nullopt;
}
