// timemanager.cpp

#include "timemanager.hpp"

#include "defs.hpp"       // types, constants
#include "searchinfo.hpp" // searchinfo
#include "board.hpp"      // board

using namespace MPChess::Types;


namespace MPChess {

Milliseconds calculate_search_time(const Board& board,
                                   Milliseconds white_time,
                                   Milliseconds black_time,
                                   Milliseconds white_inc,
                                   Milliseconds black_inc,
                                   std::size_t  moves_to_go)
{
    const uint remaining_moves_estimate = std::min(static_cast<std::size_t>(20), moves_to_go);
    const Milliseconds remaining_time = (board.get_side_to_move() == Color::WHITE) ? white_time + white_inc
                                                                                   : black_time + black_inc;

    return remaining_time / remaining_moves_estimate;
}

Milliseconds calculate_search_time(const Board& board, const SearchInfo& search_info)
{
    const auto& white_time  = search_info.white_time;
    const auto& black_time  = search_info.black_time;
    const auto& white_inc   = search_info.white_inc;
    const auto& black_inc   = search_info.black_inc;
    const auto& moves_to_go = search_info.moves_to_go;

    return calculate_search_time(board, white_time, black_time, white_inc, black_inc, moves_to_go);
}

} // MPChess namespace