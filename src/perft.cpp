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

        const Color color_friend = board.get_side_to_move();
        const Color color_enemy  = ~color_friend;

        board.make_move(move);

        // check if legal move
        const bool check = (color_friend == Color::WHITE) ? board.is_check<Color::WHITE>()
                                                          : board.is_check<Color::BLACK>() ;
        if (!check) {

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
                    const bool enemy_check = (color_enemy == Color::WHITE) ? board.is_check<Color::WHITE>()
                                                                           : board.is_check<Color::BLACK>();
                    if (enemy_check) {
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