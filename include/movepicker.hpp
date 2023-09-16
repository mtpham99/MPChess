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
        const Move           pv_move  = tt_entry.move;

        for (OrderedMove& move : this->move_list) {

            // 1. hash/tt move
            // TODO : multi pv?
            if (move == pv_move) {
                move.set_score(std::numeric_limits<Types::MoveScore>::max());
            }

            // 2. capture
            else if (move.is_capture()) {
                
                const Types::PieceType attacker = piece_type(this->position.moved_piece(move));
                const Types::PieceType victim   = (move.is_enpassant()) ? Types::PieceType::PAWN :
                                                                          piece_type(this->position.captured_piece(move));

                // score captures directly below tt moves
                const Types::MoveScore score = Constants::MVVLVA_SCORES[attacker][victim]
                                             + std::numeric_limits<Types::MoveScore>::max() - 1
                                             - 11; // max mvvlva value
                move.set_score(score);
            }


            // 3. non-capture
            // TODO : setting score here makes child nodes not affect upper depth search order
            else {

                // history move
                // Note all non-captures (including killer moves) will get history score added
                const Types::Square to = move.get_to_square();
                const Types::Piece  p  = this->position.moved_piece(move);

                // TODO : make sure none of these scores are larger than any capture score
                Types::MoveScore score = Engine::history_table[p][to];
                

                // killer move
                // Add max history score to killer moves to prioritize over history moves
                if (const auto& p_kmove = std::find(Engine::killer_table[this->position.get_ply_played()].begin(), 
                                                    Engine::killer_table[this->position.get_ply_played()].end(),
                                                    move);
                    p_kmove != Engine::killer_table[this->position.get_ply_played()].end())
                {
                    const std::size_t k_index = std::distance(Engine::killer_table[this->position.get_ply_played()].begin(), p_kmove);
                    score += Constants::NUM_KILLER_MOVES - k_index;
                    move.set_score(score);
                } 
            }
        }

        // sort all moves
        // TODO : should this be select sort on each "next_move" call?
        //      : https://www.chessprogramming.org/Move_Ordering/#Using
        std::sort(this->move_list.begin(), this->move_list.end());
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