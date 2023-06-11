// zobrist.hpp

#pragma once

#include "defs.hpp" // types, constants
#include "rng.hpp"  // xorshift rng

#include <array>    // array


namespace MPChess {

// forward declaration
class Board;


namespace Zobrist {

    // hash getters    
    Types::Key get_piece_square_key(Types::Piece p, Types::Square sq);
    Types::Key get_enpassant_key(Types::Square sq);
    Types::Key get_castle_key(Types::Castle c);
    Types::Key get_color_key();

    // hashes
    namespace Hashes {

    const std::array<Types::Key, Constants::NUM_PIECES*Constants::NUM_SQUARES> piece_square = Rng::main_rng.generate_N<Constants::NUM_PIECES*Constants::NUM_SQUARES>();
    const std::array<Types::Key, Constants::NUM_FILES>                         enpassant    = Rng::main_rng.generate_N<Constants::NUM_FILES>();
    const std::array<Types::Key, Constants::NUM_CASTLE_STATES>                 castle       = Rng::main_rng.generate_N<Constants::NUM_CASTLE_STATES>();
    const Types::Key                                                           color        = Rng::main_rng.generate();

    } // Hashes namespace

} // Zobrist namespace

} // MPChess namespace