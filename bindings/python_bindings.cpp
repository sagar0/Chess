#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <optional>
#include <sstream>
#include <string>
#include <utility>

#include "Board.hpp"
#include "Types.hpp"

namespace py = pybind11;

namespace {

struct PieceInfo {
    PieceType type{PieceType::None};
    Color color{Color::White};
};

std::optional<PieceInfo> pieceAt(const Board& board, Position pos) {
    const Piece* piece = board.pieceAt(pos);
    if (piece == nullptr) {
        return std::nullopt;
    }
    return PieceInfo{piece->type(), piece->color()};
}

std::optional<std::string> tryMove(Board& board,
                                   Position from,
                                   Position to,
                                   std::optional<PieceType> promotion) {
    return board.tryMove(from, to, promotion);
}

std::string boardToString(const Board& board) {
    std::ostringstream out;
    board.print(out);
    return out.str();
}

std::string lastMoveToString(const Board& board) {
    std::ostringstream out;
    board.printLastMove(out);
    return out.str();
}

std::string moveHistoryToString(const Board& board) {
    std::ostringstream out;
    board.printMoveHistory(out);
    return out.str();
}

std::optional<Position> positionFromAlgebraic(const std::string& square) {
    return Position::fromAlgebraic(square);
}

}  // namespace

PYBIND11_MODULE(chess_engine, m) {
    m.doc() = "Chess engine core bindings";

    py::enum_<Color>(m, "Color")
        .value("White", Color::White)
        .value("Black", Color::Black);

    py::enum_<PieceType>(m, "PieceType")
        .value("Pawn", PieceType::Pawn)
        .value("Rook", PieceType::Rook)
        .value("Knight", PieceType::Knight)
        .value("Bishop", PieceType::Bishop)
        .value("Queen", PieceType::Queen)
        .value("King", PieceType::King)
        .value("None", PieceType::None);

    py::enum_<GameState>(m, "GameState")
        .value("InProgress", GameState::InProgress)
        .value("Checkmate", GameState::Checkmate)
        .value("Stalemate", GameState::Stalemate);

    py::class_<Position>(m, "Position")
        .def(py::init<int, int>(), py::arg("row"), py::arg("col"))
        .def_readwrite("row", &Position::row)
        .def_readwrite("col", &Position::col)
        .def_static("from_algebraic", &positionFromAlgebraic)
        .def("to_algebraic", &Position::toAlgebraic)
        .def_static("is_on_board", &Position::isOnBoard)
        .def("__eq__", &Position::operator==)
        .def("__repr__", [](const Position& pos) {
            return "<Position " + pos.toAlgebraic() + ">";
        });

    py::class_<Move>(m, "Move")
        .def_readonly("color", &Move::color)
        .def_readonly("from", &Move::from)
        .def_readonly("to", &Move::to)
        .def_readonly("piece", &Move::piece)
        .def_readonly("captured", &Move::captured)
        .def_readonly("promotion", &Move::promotion)
        .def("to_algebraic", &Move::toAlgebraic);

    py::class_<PieceInfo>(m, "PieceInfo")
        .def_readonly("type", &PieceInfo::type)
        .def_readonly("color", &PieceInfo::color);

    py::class_<Board>(m, "Board")
        .def(py::init<>())
        .def("setup_standard_position", &Board::setupStandardPosition)
        .def("current_turn", &Board::currentTurn)
        .def("try_move", &tryMove, py::arg("from"), py::arg("to"), py::arg("promotion") = std::nullopt)
        .def("piece_at", &pieceAt)
        .def("is_in_check", &Board::isInCheck)
        .def("has_legal_moves", &Board::hasLegalMoves)
        .def("game_state", &Board::gameState)
        .def("winner", &Board::winner)
        .def("last_promotion", &Board::lastPromotion)
        .def("move_history", &Board::moveHistory, py::return_value_policy::reference_internal)
        .def("last_move", &Board::lastMove)
        .def("en_passant_target", &Board::enPassantTarget)
        .def("to_string", &boardToString)
        .def("last_move_string", &lastMoveToString)
        .def("move_history_string", &moveHistoryToString);

    m.def("position_from_algebraic", &positionFromAlgebraic);
    m.def("piece_type_from_promotion_char", &pieceTypeFromPromotionChar);
    m.def("color_name", &colorName);
    m.def("piece_type_name", &pieceTypeName);
}
