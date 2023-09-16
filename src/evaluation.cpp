// evaluation.cpp

#include "evaluation.hpp"

#include "defs.hpp"  // types, constants
#include "utils.hpp" // pop_count
#include "board.hpp" // board

using namespace MPChess::Types;
using namespace MPChess::Constants;

namespace MPChess {

Eval evaluate_material(const Board& board) {
    Eval score = 0;

    for (const PieceType& pt : ALL_PIECE_TYPES) {
        const int white_piece_count = pop_count(board.get_piece_bb(color_type_to_piece(Color::WHITE, pt)));
        const int black_piece_count = pop_count(board.get_piece_bb(color_type_to_piece(Color::BLACK, pt)));
        score += PIECE_SCORES[pt] * (white_piece_count - black_piece_count);
    }



    // bishop pair
    const int bishop_pair = (pop_count(board.get_piece_bb(color_type_to_piece(Color::WHITE, PieceType::BISHOP))) >= 2)
                          - (pop_count(board.get_piece_bb(color_type_to_piece(Color::BLACK, PieceType::BISHOP))) >= 2);
    score += BISHOP_PAIR_SCORE * bishop_pair;

    return score;
}

Eval evaluate_piece_square(const Board& board) {
    Eval score = 0;

    for (const Square& sq : ALL_SQUARES) {
        const Piece     p  = board.get_square_piece(sq);
        if (p == Piece::NO_PIECE) {continue;}        

        const PieceType pt = piece_type(p);
        const Color     c  = piece_color(p);

        score += (c == Color::WHITE) ?  PIECE_SQUARE_EVAL_TABLE[pt][sq]
                                     : -PIECE_SQUARE_EVAL_TABLE[pt][flip<FlipType::VERTICAL>(sq)];
    }

    return score;
}

Eval evaluate(const Board& board) {
    Eval score = 0;
    score += evaluate_material(board);
    score += evaluate_piece_square(board);

    return (board.get_side_to_move() == Color::WHITE) ?  score 
                                                      : -score;
}

} // MPChess namespace