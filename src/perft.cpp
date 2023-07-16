// perft.cpp

#include "perft.hpp"

#include "movelist.hpp"
#include "movegen.hpp"

using namespace MPChess::Types;


namespace MPChess {

unsigned long long perft(unsigned int depth, Board& board, PerftInfo* p_perft_info) {

    RegularMoveList move_list;
    generate_moves<MoveGenType::PSEUDOLEGAL>(board, move_list);

    if (depth == 0) {return 1ull;}

    unsigned long long node_count = 0;
    for (const auto& move : move_list) {

        board.make_move(move);

        // check if legal move
        if (!board.is_check<false>()) {

            // save perft info if specified
            if (p_perft_info != nullptr) {

                if (depth == 1) {

                    // piece captured
                    if (move.is_capture()) {
                        ++(p_perft_info->captures);
                    }

                    // enpassant
                    if (move.is_enpassant()) {
                        ++(p_perft_info->enpassants);
                    }

                    // castle
                    if (move.is_castle()) {
                        ++(p_perft_info->castles);
                    }

                    // promotion
                    if (move.is_promote()) {
                        ++(p_perft_info->promotions);
                    }

                    // checks
                    if (board.is_check<true>()) {
                        ++(p_perft_info->checks);
                    }
                }
            }
        
            node_count += perft(depth - 1, board, p_perft_info);
        }

        board.unmake_move();
    }

    return node_count;
}

} // MPChess namespace