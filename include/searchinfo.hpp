// searchinfo.hpp

#pragma once

#include "defs.hpp"     // types, constants

#include "movelist.hpp" // movelist

#include <limits>       // infinity


namespace MPChess {

struct SearchInfo {

    // start time
    Types::TimePoint start_time{};

    // root moves
    RegularMoveList root_moves{};

    // special search types
    bool ponder   = false;
    bool infinite = false;

    // explicit search limits
    std::size_t max_nodes = std::numeric_limits<std::size_t>::max();
    std::size_t max_depth = Constants::MAX_PLY;
    std::size_t mate_in_n = 0;
    Types::Milliseconds max_time = Types::Milliseconds::max();

    // time/clock info
    Types::Milliseconds white_time = Types::Milliseconds::max();
    Types::Milliseconds black_time = Types::Milliseconds::max();
    Types::Milliseconds white_inc  = Types::Milliseconds::max();
    Types::Milliseconds black_inc  = Types::Milliseconds::max();
    std::size_t moves_to_go = std::numeric_limits<std::size_t>::max();

    // search stats
    std::size_t curr_move_number      = 0;
    std::size_t depth_node_count      = 0;
    std::size_t depth_node_count_prev = 0;
};

} // MPChess namespace