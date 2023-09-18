// movepicker.hpp

#pragma once

#include "defs.hpp"

#include "move.hpp"     // move
#include "movelist.hpp" // movelist
#include "board.hpp"    // board
#include "movegen.hpp"  // movegen
#include "engine.hpp"   // engine tables (tt, killer, history)

#include <array>  // array
#include <limits> // max value


namespace MPChess {


namespace Constants {

    inline constexpr std::array<std::array<Types::MoveScore, NUM_PIECE_TYPES>, NUM_PIECE_TYPES> MVVLVA_SCORES = []() consteval {
        std::array<std::array<Types::MoveScore, NUM_PIECE_TYPES>, NUM_PIECE_TYPES> mmvlva_scores;
        std::array<Types::MoveScore, NUM_PIECES> victim_scores = {1,2,3,4,5,6};

        for (const Types::PieceType& victim : ALL_PIECE_TYPES) {
            for (const Types::PieceType& attacker : ALL_PIECE_TYPES) {
                mmvlva_scores[victim][attacker] = ((victim_scores[victim] + NUM_PIECE_TYPES) - victim_scores[attacker]);
            }
        }

        return mmvlva_scores;
    }();

    inline constexpr Types::MoveScore MIN_MVVLVA_SCORE  = []() consteval {
        Types::MoveScore min_score = std::numeric_limits<Types::MoveScore>::max();
        for (const auto& row : MVVLVA_SCORES) {
            for (const auto& score : row) {
                min_score = std::min(min_score, score);
            }
        }

        return min_score;
    }();

    inline constexpr Types::MoveScore MAX_MVVLVA_SCORE  = []() consteval {
        Types::MoveScore max_score = std::numeric_limits<Types::MoveScore>::min();
        for (const auto& row : MVVLVA_SCORES) {
            for (const auto& score : row) {
                max_score = std::max(max_score, score);
            }
        }

        return max_score;
    }();


    inline constexpr Types::MoveScore TT_MOVE_SCORE        = std::numeric_limits<Types::MoveScore>::max();
    inline constexpr Types::MoveScore CAPTURE_SCORE_OFFSET = TT_MOVE_SCORE - 1 - MAX_MVVLVA_SCORE;
    inline constexpr Types::MoveScore KILLER_SCORE_OFFSET  = CAPTURE_SCORE_OFFSET + MIN_MVVLVA_SCORE - 1;

    /*
    //
    // #    [movetype]         [movescore]
    // 1)   tt move         => TT_MOVE_SCORE
    // 2)   best capture    => TT_MOVE_SCORE - 1 = CAPTURE_SCORE_OFFSET + MAX_MVVLVA_SCORE
    // 3)   capture
    // ...
    // X)   worst capture   => CAPTURE_SCORE_OFFSET + MIN_MVVLVA_SCORE
    // X+1) 1st killer move => CAPTURE_SCORE_OFFSET + MIN_MVVLVA_SCORE - 1 = KILLER_SCORE_OFFSET
    // X+2) 2nd killer move => KILLER_SCORE_OFFSET
    // X+Y) Yth killer move => KILLER_SCORE_OFFSET
    //
    // A) hash move(s) is(are) scored as max
    // B) captures are scored as CAPTURE_SCORE_OFFSET + MVVLVA
    // C) killers are scored as CAPTURE_SCORE_OFFSET + MIN_MVVLVA - 1 (i.e. scored to be immediately after worst capture)
    // D) histories are scored as just the history score (starting at 0)
    // TODO : verify all histories are below killers
    //
    */

} // Constants namespace


// MovePicker

template<Types::MoveGenType gen_type>
class MovePicker {

private:

    const Board& position;

    std::size_t     iter = 0;
    OrderedMoveList move_list;

public:

    // Constructors

    MovePicker(const Board& pos) : position{pos}
    {
        // Generate all pseudolegal moves
        generate_moves<gen_type>(this->position, this->move_list);

        // Score all pseudolegal moves
        // TODO : - promotions?
        //        - make sure no overlap using these manual offsets

        // Get tt move for current position
        const Types::TTEntry tt_entry = Engine::tt.probe(this->position.get_zobrist_key());
        const Move           tt_move  = tt_entry.move;

        for (OrderedMove& move : this->move_list) {

            // 1. hash/tt move
            // TODO : multi pv?
            if (move == tt_move) {
                move.set_score(Constants::TT_MOVE_SCORE);
            }

            // 2. capture
            else if (move.is_capture()) {
                
                const Types::PieceType attacker = piece_type(this->position.moved_piece(move));
                const Types::PieceType victim   = (move.is_enpassant()) ? Types::PieceType::PAWN :
                                                                          piece_type(this->position.captured_piece(move));

                // score captures directly below tt moves
                const Types::MoveScore score = Constants::CAPTURE_SCORE_OFFSET + Constants::MVVLVA_SCORES[victim][attacker];
                move.set_score(score);
            }


            // 3. non-capture
            // TODO : setting score here makes child nodes not affect upper depth search order
            else {
                // killer move
                // placing directory below minimum capture score
                if (const auto& p_kmove = std::find(Engine::killer_table[this->position.get_ply_played()].begin(), 
                                                    Engine::killer_table[this->position.get_ply_played()].end(),
                                                    move);
                    p_kmove != Engine::killer_table[this->position.get_ply_played()].end())
                {
                    const Types::MoveScore score = Constants::KILLER_SCORE_OFFSET;
                    move.set_score(score);
                }
                // history move
                else {
                    const Types::Square to = move.get_to_square();
                    const Types::Piece  p  = this->position.moved_piece(move);

                    // TODO : make sure none of these scores are larger than any capture score
                    const Types::MoveScore score = Engine::history_table[p][to];
                    move.set_score(score);
                }
            }
        }

        // sort all moves
        // TODO : should this be select sort on each "next_move" call?
        //      : https://www.chessprogramming.org/Move_Ordering/#Using
        std::sort(this->move_list.begin(), this->move_list.end());
        std::reverse(this->move_list.begin(), this->move_list.end());
    }


    // Next Move

    Move next_move() {

        if (this->iter == this->move_list.get_size()) {
            return {}; // null move
        }

        return this->move_list[(this->iter)++];
    }
};

} // MPChess namespace