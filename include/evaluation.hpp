// evaluation.hpp

#pragma once

#include "defs.hpp" // types, constants

#include <array>    // array


namespace MPChess {

// Forward declarations
class Board;

namespace Constants {

// Piece values
inline constexpr Types::Eval PAWN_SCORE     = 100;
inline constexpr Types::Eval KNIGHT_SCORE   = 350;
inline constexpr Types::Eval BISHOP_SCORE   = 350;
inline constexpr Types::Eval ROOK_SCORE     = 525;
inline constexpr Types::Eval QUEEN_SCORE    = 1000;
inline constexpr Types::Eval KING_SCORE     = 10000;
inline constexpr Types::Eval NO_PIECE_SCORE = 0;
inline constexpr std::array<Types::Eval, NUM_PIECE_TYPES> PIECE_SCORES = {
    PAWN_SCORE, KNIGHT_SCORE, BISHOP_SCORE,
    ROOK_SCORE, QUEEN_SCORE, KING_SCORE
};

// Square piece tables
inline constexpr std::array<std::array<int, NUM_SQUARES>, NUM_PIECE_TYPES-1> PIECE_SQUARE_EVAL_TABLE = {{

    // pawn
    {
        0,   0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,   5, 10, 25, 25, 10,  5,  5,
        0,   0,  0, 20, 20,  0,  0,  0,
        5,  -5,-10,  0,  0,-10, -5,  5,
        5,  10, 10,-20,-20, 10, 10,  5,
        0,   0,  0,  0,  0,  0,  0,  0
    },

    // knight
    {
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    },

    // bishop
    {
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20,
    },

    // rook 
    {
         0,  0,  0,  0,  0,  0,  0,  0,
         5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
         0,  0,  0,  5,  5,  0,  0,  0
    },

    // queen
    {
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
         -5,  0,  5,  5,  5,  5,  0, -5,
          0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    }
}};

} // Constants namespace


// Eval functions
Types::Eval evaluate_material(const Board& board);
Types::Eval evaluate_piece_square(const Board& board);
Types::Eval evaluate(const Board& board);

} // MPChess