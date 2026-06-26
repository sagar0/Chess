#include "Board.hpp"

#include "pieces/Bishop.hpp"
#include "pieces/King.hpp"
#include "pieces/Knight.hpp"
#include "pieces/Pawn.hpp"
#include "pieces/Queen.hpp"
#include "pieces/Rook.hpp"

#include <iostream>
#include <utility>

namespace {

std::unique_ptr<Piece> makePiece(PieceType type, Color color) {
    switch (type) {
        case PieceType::Pawn:
            return std::make_unique<Pawn>(color);
        case PieceType::Rook:
            return std::make_unique<Rook>(color);
        case PieceType::Knight:
            return std::make_unique<Knight>(color);
        case PieceType::Bishop:
            return std::make_unique<Bishop>(color);
        case PieceType::Queen:
            return std::make_unique<Queen>(color);
        case PieceType::King:
            return std::make_unique<King>(color);
        case PieceType::None:
            break;
    }
    return nullptr;
}

void promotePawnIfNeeded(std::unique_ptr<Piece>& piece, PieceType promotionType) {
    if (piece != nullptr && piece->type() == PieceType::Pawn) {
        piece = makePiece(promotionType, piece->color());
    }
}

int kingHomeRow(Color color) {
    return color == Color::White ? 7 : 0;
}

}  // namespace

bool Board::isPawnPromotionSquare(Color color, int row) {
    return (color == Color::White && row == 0) || (color == Color::Black && row == 7);
}

std::optional<PieceType> Board::resolvePromotion(const Piece* movingPiece,
                                                 Position to,
                                                 std::optional<PieceType> promotion) {
    if (movingPiece == nullptr || movingPiece->type() != PieceType::Pawn ||
        !isPawnPromotionSquare(movingPiece->color(), to.row)) {
        return std::nullopt;
    }

    const PieceType promotionType = promotion.value_or(PieceType::Queen);
    if (!isPromotablePiece(promotionType)) {
        return std::nullopt;
    }
    return promotionType;
}

Board::Board() = default;

void Board::placePiece(PieceType type, Color color, Position pos) {
    grid_[static_cast<std::size_t>(pos.row)][static_cast<std::size_t>(pos.col)] =
        makePiece(type, color);
}

void Board::setupStandardPosition() {
    for (auto& row : grid_) {
        for (auto& square : row) {
            square.reset();
        }
    }

    const PieceType backRank[] = {
        PieceType::Rook,   PieceType::Knight, PieceType::Bishop, PieceType::Queen,
        PieceType::King,   PieceType::Bishop, PieceType::Knight, PieceType::Rook,
    };

    for (int col = 0; col < 8; ++col) {
        placePiece(backRank[col], Color::Black, {0, col});
        placePiece(PieceType::Pawn, Color::Black, {1, col});
        placePiece(PieceType::Pawn, Color::White, {6, col});
        placePiece(backRank[col], Color::White, {7, col});
    }

    currentTurn_ = Color::White;
    moveHistory_.clear();
    lastPromotion_.reset();
    castlingRights_ = CastlingRights{};
    enPassantTarget_.reset();
}

void Board::print(std::ostream& out) const {
    out << "  a b c d e f g h\n";
    for (int row = 0; row < 8; ++row) {
        out << (8 - row) << ' ';
        for (int col = 0; col < 8; ++col) {
            const Piece* piece = pieceAt({row, col});
            if (piece != nullptr) {
                out << piece->symbol();
            } else {
                out << '.';
            }
            if (col < 7) {
                out << ' ';
            }
        }
        out << ' ' << (8 - row) << '\n';
    }
    out << "  a b c d e f g h\n";
}

const Piece* Board::pieceAt(Position pos) const {
    if (!Position::isOnBoard(pos)) {
        return nullptr;
    }
    return grid_[static_cast<std::size_t>(pos.row)][static_cast<std::size_t>(pos.col)].get();
}

bool Board::isEmpty(Position pos) const {
    return pieceAt(pos) == nullptr;
}

bool Board::isEnemy(Position pos, Color color) const {
    const Piece* piece = pieceAt(pos);
    return piece != nullptr && piece->color() != color;
}

bool Board::isAlly(Position pos, Color color) const {
    const Piece* piece = pieceAt(pos);
    return piece != nullptr && piece->color() == color;
}

bool Board::isPathClear(Position from, Position to) const {
    const int rowStep = (to.row > from.row) ? 1 : (to.row < from.row ? -1 : 0);
    const int colStep = (to.col > from.col) ? 1 : (to.col < from.col ? -1 : 0);

    int row = from.row + rowStep;
    int col = from.col + colStep;

    while (row != to.row || col != to.col) {
        if (!isEmpty({row, col})) {
            return false;
        }
        row += rowStep;
        col += colStep;
    }

    return true;
}

bool Board::isCastlingMove(Position from, Position to, Color color) const {
    return isCastlingStructureValid(from, to, color);
}

bool Board::isCastlingStructureValid(Position from, Position to, Color color) const {
    const int homeRow = kingHomeRow(color);
    if (from.row != homeRow || to.row != homeRow || from.col != 4) {
        return false;
    }

    const int colDelta = to.col - from.col;
    if (std::abs(colDelta) != 2) {
        return false;
    }

    const bool kingside = colDelta > 0;
    if (color == Color::White) {
        if (kingside && !castlingRights_.whiteKingside) {
            return false;
        }
        if (!kingside && !castlingRights_.whiteQueenside) {
            return false;
        }
    } else {
        if (kingside && !castlingRights_.blackKingside) {
            return false;
        }
        if (!kingside && !castlingRights_.blackQueenside) {
            return false;
        }
    }

    const Piece* king = pieceAt(from);
    if (king == nullptr || king->type() != PieceType::King || king->color() != color) {
        return false;
    }

    if (kingside) {
        if (!isEmpty({homeRow, 5}) || !isEmpty({homeRow, 6})) {
            return false;
        }
        const Piece* rook = pieceAt({homeRow, 7});
        return rook != nullptr && rook->type() == PieceType::Rook && rook->color() == color;
    }

    if (!isEmpty({homeRow, 1}) || !isEmpty({homeRow, 2}) || !isEmpty({homeRow, 3})) {
        return false;
    }
    const Piece* rook = pieceAt({homeRow, 0});
    return rook != nullptr && rook->type() == PieceType::Rook && rook->color() == color;
}

bool Board::canCastle(Position from, Position to, Color color) const {
    if (!isCastlingStructureValid(from, to, color)) {
        return false;
    }
    if (isInCheck(color)) {
        return false;
    }

    const Color attacker = color == Color::White ? Color::Black : Color::White;
    const int step = to.col > from.col ? 1 : -1;
    for (int col = from.col; col != to.col + step; col += step) {
        if (isSquareAttacked({from.row, col}, attacker)) {
            return false;
        }
    }
    return true;
}

void Board::applyCastling(Position from, Position to) {
    const int homeRow = from.row;
    const bool kingside = to.col > from.col;
    const int rookFromCol = kingside ? 7 : 0;
    const int rookToCol = kingside ? 5 : 3;

    grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] =
        std::move(grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)]);
    grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] = nullptr;
    grid_[static_cast<std::size_t>(homeRow)][static_cast<std::size_t>(rookToCol)] =
        std::move(grid_[static_cast<std::size_t>(homeRow)][static_cast<std::size_t>(rookFromCol)]);
    grid_[static_cast<std::size_t>(homeRow)][static_cast<std::size_t>(rookFromCol)] = nullptr;
}

std::optional<Position> Board::enPassantCapturedSquare(Position to, Color color) const {
    if (!enPassantTarget_.has_value() || *enPassantTarget_ != to) {
        return std::nullopt;
    }
    return Position{to.row - pawnDirection(color), to.col};
}

bool Board::isEnPassantCapture(Position from, Position to, Color color) const {
    const Piece* pawn = pieceAt(from);
    if (pawn == nullptr || pawn->type() != PieceType::Pawn) {
        return false;
    }
    return enPassantCapturedSquare(to, color).has_value() && isEmpty(to);
}

void Board::updateEnPassantTarget(Position from, Position to, const Piece* movingPiece) {
    enPassantTarget_.reset();
    if (movingPiece == nullptr || movingPiece->type() != PieceType::Pawn) {
        return;
    }

    const int direction = pawnDirection(movingPiece->color());
    const int startRow = movingPiece->color() == Color::White ? 6 : 1;
    if (from.row == startRow && to.row - from.row == 2 * direction) {
        enPassantTarget_ = Position{from.row + direction, from.col};
    }
}

void Board::updateCastlingRights(Position from, Position to, const Piece* movingPiece) {
    if (movingPiece == nullptr) {
        return;
    }

    if (movingPiece->type() == PieceType::King) {
        if (movingPiece->color() == Color::White) {
            castlingRights_.whiteKingside = false;
            castlingRights_.whiteQueenside = false;
        } else {
            castlingRights_.blackKingside = false;
            castlingRights_.blackQueenside = false;
        }
    }

    if (movingPiece->type() == PieceType::Rook) {
        if (from == Position{7, 0}) {
            castlingRights_.whiteQueenside = false;
        }
        if (from == Position{7, 7}) {
            castlingRights_.whiteKingside = false;
        }
        if (from == Position{0, 0}) {
            castlingRights_.blackQueenside = false;
        }
        if (from == Position{0, 7}) {
            castlingRights_.blackKingside = false;
        }
    }

    if (to == Position{7, 0}) {
        castlingRights_.whiteQueenside = false;
    }
    if (to == Position{7, 7}) {
        castlingRights_.whiteKingside = false;
    }
    if (to == Position{0, 0}) {
        castlingRights_.blackQueenside = false;
    }
    if (to == Position{0, 7}) {
        castlingRights_.blackKingside = false;
    }
}

std::optional<Position> Board::findKing(Color color) const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Piece* piece = pieceAt({row, col});
            if (piece != nullptr && piece->type() == PieceType::King && piece->color() == color) {
                return Position{row, col};
            }
        }
    }
    return std::nullopt;
}

bool Board::isSquareAttacked(Position sq, Color byColor) const {
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            const Position from{row, col};
            const Piece* piece = pieceAt(from);
            if (piece == nullptr || piece->color() != byColor) {
                continue;
            }
            if (piece->isValidMove(*this, from, sq)) {
                return true;
            }
        }
    }
    return false;
}

bool Board::isInCheck(Color color) const {
    const auto kingPos = findKing(color);
    if (!kingPos.has_value()) {
        return false;
    }
    const Color attacker = color == Color::White ? Color::Black : Color::White;
    return isSquareAttacked(*kingPos, attacker);
}

bool Board::isPseudoLegal(Position from, Position to, Color color) const {
    const Piece* movingPiece = pieceAt(from);
    if (movingPiece == nullptr || movingPiece->color() != color) {
        return false;
    }
    if (isAlly(to, color)) {
        return false;
    }
    const Piece* target = pieceAt(to);
    if (target != nullptr && target->type() == PieceType::King) {
        return false;
    }
    return movingPiece->isValidMove(*this, from, to);
}

bool Board::wouldLeaveKingInCheck(Position from,
                                  Position to,
                                  Color color,
                                  std::optional<PieceType> promotion) const {
    auto& self = const_cast<Board&>(*this);

    const Piece* movingPiece = pieceAt(from);
    if (movingPiece == nullptr) {
        return true;
    }

    if (isCastlingStructureValid(from, to, color)) {
        const int homeRow = from.row;
        const bool kingside = to.col > from.col;
        const int rookFromCol = kingside ? 7 : 0;
        const int rookToCol = kingside ? 5 : 3;

        std::unique_ptr<Piece> king =
            std::move(self.grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)]);
        std::unique_ptr<Piece> rook =
            std::move(self.grid_[static_cast<std::size_t>(homeRow)][static_cast<std::size_t>(rookFromCol)]);

        self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] = std::move(king);
        self.grid_[static_cast<std::size_t>(homeRow)][static_cast<std::size_t>(rookToCol)] = std::move(rook);

        const bool inCheck = self.isInCheck(color);

        self.grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] =
            std::move(self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)]);
        self.grid_[static_cast<std::size_t>(homeRow)][static_cast<std::size_t>(rookFromCol)] =
            std::move(self.grid_[static_cast<std::size_t>(homeRow)][static_cast<std::size_t>(rookToCol)]);
        self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] = nullptr;
        self.grid_[static_cast<std::size_t>(homeRow)][static_cast<std::size_t>(rookToCol)] = nullptr;

        return inCheck;
    }

    const auto enPassantCapture = enPassantCapturedSquare(to, color);
    if (enPassantCapture.has_value() && movingPiece->type() == PieceType::Pawn && isEmpty(to)) {
        std::unique_ptr<Piece> moving =
            std::move(self.grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)]);
        std::unique_ptr<Piece> captured = std::move(
            self.grid_[static_cast<std::size_t>(enPassantCapture->row)]
                      [static_cast<std::size_t>(enPassantCapture->col)]);

        self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] = std::move(moving);
        self.grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] = nullptr;
        self.grid_[static_cast<std::size_t>(enPassantCapture->row)]
                   [static_cast<std::size_t>(enPassantCapture->col)] = nullptr;

        const bool inCheck = self.isInCheck(color);

        self.grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] =
            std::move(self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)]);
        self.grid_[static_cast<std::size_t>(enPassantCapture->row)]
                   [static_cast<std::size_t>(enPassantCapture->col)] = std::move(captured);
        self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] = nullptr;

        return inCheck;
    }

    const PieceType originalType = movingPiece->type();
    const Color pieceColor = movingPiece->color();
    const auto promotionType = resolvePromotion(movingPiece, to, promotion);

    std::unique_ptr<Piece> captured =
        std::move(self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)]);
    std::unique_ptr<Piece> moving =
        std::move(self.grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)]);

    if (promotionType.has_value()) {
        self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] =
            makePiece(*promotionType, pieceColor);
    } else {
        self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] =
            std::move(moving);
        moving = nullptr;
    }

    const bool inCheck = self.isInCheck(color);

    if (promotionType.has_value()) {
        self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)].reset();
        self.grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] =
            makePiece(originalType, pieceColor);
    } else {
        self.grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] =
            std::move(self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)]);
        self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] = nullptr;
    }
    self.grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] =
        std::move(captured);

    return inCheck;
}

bool Board::hasLegalMoves(Color color) const {
    for (int fromRow = 0; fromRow < 8; ++fromRow) {
        for (int fromCol = 0; fromCol < 8; ++fromCol) {
            const Position from{fromRow, fromCol};
            const Piece* piece = pieceAt(from);
            if (piece == nullptr || piece->color() != color) {
                continue;
            }
            for (int toRow = 0; toRow < 8; ++toRow) {
                for (int toCol = 0; toCol < 8; ++toCol) {
                    const Position to{toRow, toCol};
                    if (!isPseudoLegal(from, to, color)) {
                        continue;
                    }
                    if (isCastlingStructureValid(from, to, color) && !canCastle(from, to, color)) {
                        continue;
                    }
                    const Piece* pieceForPromotion = pieceAt(from);
                    const auto promotion =
                        resolvePromotion(pieceForPromotion, to, std::optional<PieceType>{PieceType::Queen});
                    if (!wouldLeaveKingInCheck(from, to, color, promotion)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

GameState Board::gameState() const {
    if (hasLegalMoves(currentTurn_)) {
        return GameState::InProgress;
    }
    if (isInCheck(currentTurn_)) {
        return GameState::Checkmate;
    }
    return GameState::Stalemate;
}

std::optional<Color> Board::winner() const {
    if (gameState() != GameState::Checkmate) {
        return std::nullopt;
    }
    return currentTurn_ == Color::White ? Color::Black : Color::White;
}

std::optional<Move> Board::lastMove() const {
    if (moveHistory_.empty()) {
        return std::nullopt;
    }
    return moveHistory_.back();
}

void Board::printLastMove(std::ostream& out) const {
    const auto move = lastMove();
    if (!move.has_value()) {
        return;
    }
    out << "Last move: " << colorName(move->color) << " " << move->toAlgebraic();
}

std::optional<std::string> Board::tryMove(Position from,
                                          Position to,
                                          std::optional<PieceType> promotion) {
    lastPromotion_.reset();

    if (!Position::isOnBoard(from) || !Position::isOnBoard(to)) {
        return "Square is off the board.";
    }
    if (from == to) {
        return "From and to squares must differ.";
    }

    const Piece* movingPiece = pieceAt(from);
    if (movingPiece == nullptr) {
        return "No piece on the from-square.";
    }
    if (movingPiece->color() != currentTurn_) {
        return "It is not that piece's turn.";
    }
    if (isAlly(to, currentTurn_)) {
        return "Cannot capture your own piece.";
    }
    if (!movingPiece->isValidMove(*this, from, to)) {
        return "Illegal move for that piece.";
    }
    const Piece* target = pieceAt(to);
    if (target != nullptr && target->type() == PieceType::King) {
        return "Cannot capture the king.";
    }
    if (isCastlingStructureValid(from, to, currentTurn_) && !canCastle(from, to, currentTurn_)) {
        return "Castling is not legal in the current position.";
    }
    if (wouldLeaveKingInCheck(from, to, currentTurn_, promotion)) {
        return "You cannot move into or leave your king in check.";
    }

    movingPiece = pieceAt(from);
    if (movingPiece == nullptr) {
        return "No piece on the from-square.";
    }

    const auto promotionType = resolvePromotion(movingPiece, to, promotion);
    if (movingPiece->type() == PieceType::Pawn && isPawnPromotionSquare(currentTurn_, to.row) &&
        !promotionType.has_value()) {
        return "Invalid promotion piece. Use Q, R, B, or N.";
    }
    if (promotion.has_value() && !promotionType.has_value()) {
        return "Only pawns can promote on the last rank.";
    }

    const bool castling = isCastlingStructureValid(from, to, currentTurn_);
    const bool enPassant = isEnPassantCapture(from, to, currentTurn_);
    const std::optional<Position> enPassantCaptured =
        enPassant ? enPassantCapturedSquare(to, currentTurn_) : std::nullopt;

    const Move recordedMove{
        currentTurn_,
        from,
        to,
        movingPiece->type(),
        enPassant ? std::optional<PieceType>{PieceType::Pawn}
                  : (target != nullptr ? std::optional<PieceType>{target->type()} : std::nullopt),
        promotionType,
    };

    if (castling) {
        applyCastling(from, to);
    } else if (enPassant && enPassantCaptured.has_value()) {
        grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] =
            std::move(grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)]);
        grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] = nullptr;
        grid_[static_cast<std::size_t>(enPassantCaptured->row)]
               [static_cast<std::size_t>(enPassantCaptured->col)] = nullptr;
    } else {
        grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)] =
            std::move(grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)]);
        grid_[static_cast<std::size_t>(from.row)][static_cast<std::size_t>(from.col)] = nullptr;

        if (promotionType.has_value()) {
            promotePawnIfNeeded(
                grid_[static_cast<std::size_t>(to.row)][static_cast<std::size_t>(to.col)],
                *promotionType);
            lastPromotion_ = *promotionType;
        }
    }

    updateCastlingRights(from, to, movingPiece);
    updateEnPassantTarget(from, to, movingPiece);
    moveHistory_.push_back(recordedMove);
    currentTurn_ = currentTurn_ == Color::White ? Color::Black : Color::White;
    return std::nullopt;
}
