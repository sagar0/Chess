"""FastAPI adapter for the native chess_engine pybind11 module."""

from __future__ import annotations

import sys
from enum import Enum
from pathlib import Path
from typing import Any

from fastapi import FastAPI
from fastapi.responses import HTMLResponse
from pydantic import BaseModel, Field

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "bindings"))

import chess_engine as ce  # noqa: E402

app = FastAPI(title="Chess Engine Web Adapter")
board = ce.Board()
board.setup_standard_position()

TEMPLATE_PATH = Path(__file__).parent / "templates" / "index.html"

PIECE_NOTATION: dict[tuple[ce.Color, ce.PieceType], str] = {
    (ce.Color.White, ce.PieceType.Pawn): "wP",
    (ce.Color.White, ce.PieceType.Knight): "wN",
    (ce.Color.White, ce.PieceType.Bishop): "wB",
    (ce.Color.White, ce.PieceType.Rook): "wR",
    (ce.Color.White, ce.PieceType.Queen): "wQ",
    (ce.Color.White, ce.PieceType.King): "wK",
    (ce.Color.Black, ce.PieceType.Pawn): "bP",
    (ce.Color.Black, ce.PieceType.Knight): "bN",
    (ce.Color.Black, ce.PieceType.Bishop): "bB",
    (ce.Color.Black, ce.PieceType.Rook): "bR",
    (ce.Color.Black, ce.PieceType.Queen): "bQ",
    (ce.Color.Black, ce.PieceType.King): "bK",
}

FILES = "abcdefgh"
RANKS = "12345678"


class MoveRequest(BaseModel):
    from_sq: str = Field(alias="from")
    to: str

    model_config = {"populate_by_name": True}


def enum_name(value: Any) -> str:
    if isinstance(value, Enum):
        return value.name
    text = str(value)
    if "." in text:
        return text.rsplit(".", 1)[-1]
    return text


def turn_label(color: ce.Color) -> str:
    return ce.color_name(color)


def board_to_chessboard_position(engine_board: ce.Board) -> dict[str, str]:
    position: dict[str, str] = {}
    for file in FILES:
        for rank in RANKS:
            square = f"{file}{rank}"
            pos = ce.Position.from_algebraic(square)
            if pos is None:
                continue
            piece = engine_board.piece_at(pos)
            if piece is None:
                continue
            notation = PIECE_NOTATION.get((piece.color, piece.type))
            if notation is not None:
                position[square] = notation
    return position


def build_move_response(*, success: bool, error: str | None = None) -> dict[str, Any]:
    turn = board.current_turn()
    state = board.game_state()
    payload: dict[str, Any] = {
        "success": success,
        "turn": turn_label(turn),
        "board": board.to_string(),
        "position": board_to_chessboard_position(board),
        "in_check": board.is_in_check(turn),
        "game_state": enum_name(state),
        "error": error,
    }
    if success:
        last_move = board.last_move()
        if last_move is not None:
            payload["last_move"] = last_move.to_algebraic()
    return payload


@app.get("/", response_class=HTMLResponse)
async def index() -> str:
    return TEMPLATE_PATH.read_text(encoding="utf-8")


@app.post("/api/move")
async def move_piece(request: MoveRequest) -> dict[str, Any]:
    from_pos = ce.Position.from_algebraic(request.from_sq)
    to_pos = ce.Position.from_algebraic(request.to)

    if from_pos is None or to_pos is None:
        return build_move_response(success=False, error="Invalid square notation.")

    move_error = board.try_move(from_pos, to_pos)
    if move_error is not None:
        return build_move_response(success=False, error=move_error)

    return build_move_response(success=True)
