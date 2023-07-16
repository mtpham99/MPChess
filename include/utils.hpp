// utils.hpp

#pragma once

#include "defs.hpp"    // types, constants

#include <string>      // string
#include <string_view> // string_view
#include <array>       // array
#include <utility>     // pair, to_underlying

#include <iostream>    // cout, ostream

#include <cassert>     // assert


namespace MPChess {

// square/bitboard utils

template<Concepts::bitboard_like BB_t>
constexpr bool is_empty(BB_t bb) {
    if constexpr (std::same_as<Types::Bitboard, BB_t>) {
        return bb == Constants::EMPTY;
    }
    else if constexpr (std::same_as<Types::Square, BB_t>) {
        return bb == Types::Square::NO_SQUARE;
    }
}

constexpr uint rank_index(Types::Square sq) {
    assert(!is_empty(sq));
    assert(sq / Constants::RANK_SIZE < Constants::NUM_RANKS);
    return (sq / Constants::RANK_SIZE);
}

constexpr uint file_index(Types::Square sq) {
    assert(!is_empty(sq));
    assert(sq % Constants::FILE_SIZE < Constants::NUM_FILES);
    return (sq % Constants::FILE_SIZE);
}

constexpr Types::Square file_rank_to_square(int file_index, int rank_index) {
    assert(file_index < Constants::NUM_FILES &&
           file_index >= 0        &&
           rank_index < Constants::NUM_RANKS &&
           rank_index >= 0);

    return static_cast<Types::Square>(file_index + rank_index*Constants::RANK_SIZE);
}

constexpr Types::Bitboard square_to_bitboard(Types::Square sq) {
    if (is_empty(sq)) {return Constants::EMPTY;}
    // return (1ull << sq);
    return Constants::SQUARE_BITBOARDS[sq];
}

constexpr Types::Bitboard rank_bitboard(Types::Square sq) {
    if (is_empty(sq)) {return Constants::EMPTY;}
    return Constants::RANK_1 << rank_index(sq)*Constants::RANK_SIZE;
}

constexpr Types::Bitboard file_bitboard(Types::Square sq) {
    if (sq == Types::Square::NO_SQUARE) {return Constants::EMPTY;}
    return Constants::FILE_A << file_index(sq);
}

constexpr void print_bitboard(Types::Bitboard bb, std::ostream& os=std::cout) {
    for (const Types::Square& sq : Constants::ALL_SQUARES_PRINT_ORDER) {
        if (is_empty(bb & square_to_bitboard(sq))) {os << '0';}
        else                                       {os << '1';}

        if (is_empty(Constants::FILE_H & square_to_bitboard(sq))) {os << ' ';}
        else                                                      {os << '\n';}
    }
}

constexpr void print_square(Types::Square sq, std::ostream& os=std::cout) {
    os << Constants::FILE_LABELS[file_index(sq)]
       << Constants::RANK_LABELS[rank_index(sq)]
       << '\n';
}


// bit manipulation utils
// TODO : compiler options

constexpr int pop_count(unsigned long long bb) {
    return __builtin_popcountll(bb);
}

constexpr int clz(unsigned long long bb) {
    return __builtin_clzll(bb);
}

constexpr int ctz(unsigned long long bb) {
    return __builtin_ctzll(bb);
}

constexpr Types::Square lsb(Types::Bitboard bb) {
    if (is_empty(bb)) {return Types::Square::NO_SQUARE;}
    return static_cast<Types::Square>(__builtin_ffsll(bb) - 1);
}

constexpr Types::Square msb(Types::Bitboard bb) {
    if (is_empty(bb)) {return Types::Square::NO_SQUARE;}
    return static_cast<Types::Square>((UINT64_WIDTH - 1) - clz(bb));
}

constexpr Types::Square pop_lsb(Types::Bitboard& bb) {
    if (is_empty(bb)) {return Types::Square::NO_SQUARE;}

    const auto lsb_sq = lsb(bb);
    bb ^= square_to_bitboard(lsb_sq);

    return lsb_sq;
}

constexpr Types::Square pop_msb(Types::Bitboard& bb) {
    if (bb == Constants::EMPTY) {return Types::Square::NO_SQUARE;}

    const auto msb_sq = msb(bb);
    bb ^= square_to_bitboard(msb_sq);

    return msb_sq;
}

// distance/flip/rotate/step/shift

inline constexpr std::array<std::array<uint, Constants::NUM_SQUARES>, Constants::NUM_SQUARES> SQUARE_DISTANCES = [] consteval {
    std::array<std::array<uint, Constants::NUM_SQUARES>, Constants::NUM_SQUARES> distances;

    for (const Types::Square& sq_1 : Constants::ALL_SQUARES) {
        for (const Types::Square& sq_2 : Constants::ALL_SQUARES) {
            
            const uint rank_dist = std::abs( static_cast<int>(rank_index(sq_1) - rank_index(sq_2)) );
            const uint file_dist = std::abs( static_cast<int>(file_index(sq_1) - file_index(sq_2)) );
            const uint dist      = std::max(rank_dist, file_dist);

            distances[sq_1][sq_2] = dist;
        }
    }

    return distances;
}();

constexpr uint distance(Types::Square sq_1, Types::Square sq_2) {
    return SQUARE_DISTANCES[sq_1][sq_2];
}

template<Types::FlipType flip_type, Concepts::bitboard_like BB_t>
constexpr BB_t flip(BB_t bb) {
    if constexpr (flip_type == Types::FlipType::NO_FLIP) {return bb;}
    if (is_empty<BB_t>(bb))                              {return bb;}

    // square input
    if constexpr (std::same_as<Types::Square, BB_t>) {

        if      constexpr (flip_type == Types::FlipType::VERTICAL) {
            return static_cast<Types::Square>(bb ^ 56);
        }
        else if constexpr (flip_type == Types::FlipType::HORIZONTAL) {
            return static_cast<Types::Square>(bb ^ 7);
        }
        else if constexpr (flip_type == Types::FlipType::DIAG) {
            return static_cast<Types::Square>(((bb >> 3) | (bb << 3)) & 63);
        }
        else if constexpr (flip_type == Types::FlipType::ANTIDIAG) {
            return static_cast<Types::Square>((((bb >> 3) | (bb << 3)) & 63) ^ 63);
        }
    }

    // bitboard input
    // TODO : compiler/cpu arch options
    else if constexpr (std::same_as<Types::Bitboard, BB_t>) {

        if      constexpr (flip_type == Types::FlipType::VERTICAL) {
            return _byteswap_uint64(bb);
        }
        else if constexpr (flip_type == Types::FlipType::HORIZONTAL) {
            constexpr uint64_t i = 0x5555555555555555ull;
            constexpr uint64_t j = 0x3333333333333333ull;
            constexpr uint64_t k = 0x0f0f0f0f0f0f0f0full;

            bb = ((bb >> 1) & i) +  2*(bb & i);
            bb = ((bb >> 2) & j) +  4*(bb & j);
            bb = ((bb >> 4) & k) + 16*(bb & k);

            return bb;
        }
        else if constexpr (flip_type == Types::FlipType::DIAG) {
            constexpr uint64_t i = 0x5500550055005500ull;
            constexpr uint64_t j = 0x3333000033330000ull;
            constexpr uint64_t k = 0x0f0f0f0f00000000ull;

            uint64_t temp;
            temp = k & (bb ^ (bb << 28));
            bb  ^= temp ^ (temp >> 28) ;
            temp = j & (bb ^ (bb << 14));
            bb  ^= temp ^ (temp >> 14) ;
            temp = i & (bb ^ (bb << 7));
            bb  ^= temp ^ (temp >> 7) ;
            return bb;
        }
        else if constexpr (flip_type == Types::FlipType::ANTIDIAG) {
            constexpr uint64_t i = 0xaa00aa00aa00aa00ull;
            constexpr uint64_t j = 0xcccc0000cccc0000ull;
            constexpr uint64_t k = 0xf0f0f0f00f0f0f0full;

            uint64_t temp;

            temp = bb ^ (bb << 36) ;
            bb  ^= k & (temp ^ (bb >> 36));
            temp = j & (bb ^ (bb << 18));
            bb  ^= temp ^ (temp >> 18) ;
            temp = i & (bb ^ (bb << 9));
            bb  ^= temp ^ (temp >> 9) ;
            return bb;
        }
    }
}

template<Types::RotateType rotate_type, Concepts::bitboard_like BB_t>
constexpr BB_t rotate(BB_t bb) {
    if constexpr (rotate_type == Types::RotateType::NO_ROTATE) {return bb;}
    if (is_empty<BB_t>(bb))                                    {return bb;}

    // square input
    if constexpr (std::same_as<Types::Square, BB_t>) {
        
        if constexpr (rotate_type == Types::RotateType::CW90) {
            return static_cast<Types::Square>((((bb >> 3) | (bb << 3)) & 63) ^ 56);
        }
        else if constexpr (rotate_type == Types::RotateType::CW180) {
            return static_cast<Types::Square>(bb ^ 63);
        }
        else if constexpr (rotate_type == Types::RotateType::CCW90) {
            return static_cast<Types::Square>((((bb >> 3) | (bb << 3)) & 63) ^ 7);
        }
    }

    // bitboard input
    else if constexpr (std::same_as<Types::Bitboard, BB_t>) {

        if constexpr (rotate_type == Types::RotateType::CW90) {
            return flip<Types::FlipType::VERTICAL>(flip<Types::FlipType::ANTIDIAG>(bb));
        }
        else if constexpr (rotate_type == Types::RotateType::CW180) {
            return flip<Types::FlipType::HORIZONTAL>(flip<Types::FlipType::VERTICAL>(bb));
        }
        else if constexpr (rotate_type == Types::RotateType::CCW90) {
            return flip<Types::FlipType::VERTICAL>(flip<Types::FlipType::DIAG>(bb));
        }
    }
}

template<Concepts::bitboard_like BB_t, Concepts::shift_like shift_t>
constexpr BB_t shift(BB_t bb, shift_t shift) {
    if (is_empty<BB_t>(bb)) {return bb;}
    if (shift == 0)         {return bb;}

    // square input
    if constexpr (std::same_as<Types::Square, BB_t>) {

        const int new_sq_ind = static_cast<int>(bb) + static_cast<int>(shift);

        if (new_sq_ind < 0 || new_sq_ind >= Constants::NUM_SQUARES) {
            return Types::Square::NO_SQUARE;
        }

        return static_cast<Types::Square>(new_sq_ind);
    }

    // bitboard input
    else if constexpr (std::same_as<Types::Bitboard, BB_t>) {

        if (shift > 0) {return bb <<  shift;}
        else           {return bb >> -shift;} 
    }
}

template<Types::StepType step_type, Concepts::bitboard_like BB_t>
constexpr BB_t step(BB_t bb) {
    if constexpr (step_type == Types::StepType::NO_STEP) {return bb;}
    if (is_empty<BB_t>(bb))                              {return bb;}

    // square input
    if constexpr (std::same_as<Types::Square, BB_t>) {

        Types::Square new_sq = shift(bb, step_type);
        if (is_empty(new_sq)) {return Types::Square::NO_SQUARE;}

        if (distance(new_sq, bb) > 2) {return Types::Square::NO_SQUARE;}
        else                          {return new_sq;}
    }

    // bitboard input
    else if constexpr (std::same_as<Types::Bitboard, BB_t>) {

        if constexpr (step_type == Types::StepType::N) {
            bb &= ~Constants::RANK_8;
        }
        else if constexpr (step_type == Types::StepType::S) {
            bb &= ~Constants::RANK_1;
        }
        else if constexpr (step_type == Types::StepType::E) {
            bb &= ~Constants::FILE_H;
        }
        else if constexpr (step_type == Types::StepType::W) {
            bb &= ~Constants::FILE_A;
        }
        else if constexpr (step_type == Types::StepType::NW) {
            bb &= ~(Constants::RANK_8 | Constants::FILE_A);
        }
        else if constexpr (step_type == Types::StepType::NE) {
            bb &= ~(Constants::RANK_8 | Constants::FILE_H);
        }
        else if constexpr (step_type == Types::StepType::SW) {
            bb &= ~(Constants::RANK_1 | Constants::FILE_A);
        }
        else if constexpr (step_type == Types::StepType::SE) {
            bb &= ~(Constants::RANK_1 | Constants::FILE_H);
        }
        else if constexpr (step_type == Types::StepType::NWW) {
            bb &= ~(Constants::RANK_8 | Constants::FILE_A | Constants::FILE_B);
        }
        else if constexpr (step_type == Types::StepType::NNW) {
            bb &= ~(Constants::RANK_8 | Constants::RANK_7 | Constants::FILE_A);
        }
        else if constexpr (step_type == Types::StepType::NNE) {
            bb &= ~(Constants::RANK_8 | Constants::RANK_7 | Constants::FILE_H);
        }
        else if constexpr (step_type == Types::StepType::NEE) {
            bb &= ~(Constants::RANK_8 | Constants::FILE_H | Constants::FILE_G);
        }
        else if constexpr (step_type == Types::StepType::SEE) {
            bb &= ~(Constants::RANK_1 | Constants::FILE_H | Constants::FILE_G);
        }
        else if constexpr (step_type == Types::StepType::SSE) {
            bb &= ~(Constants::RANK_1 | Constants::RANK_2 | Constants::FILE_H);
        }
        else if constexpr (step_type == Types::StepType::SSW) {
            bb &= ~(Constants::RANK_1 | Constants::RANK_2 | Constants::FILE_A);
        }
        else if constexpr (step_type == Types::StepType::SWW) {
            bb &= ~(Constants::RANK_1 | Constants::FILE_A | Constants::FILE_B);
        }
        else if constexpr (step_type == Types::StepType::NN) {
            bb &= ~(Constants::RANK_8 | Constants::RANK_7);
        }
        else if constexpr (step_type == Types::StepType::SS) {
            bb &= ~(Constants::RANK_2 | Constants::RANK_1);
        }

        if (step_type > 0) {return bb <<  step_type;}
        else               {return bb >> -step_type;}
    }
}


constexpr std::string square_to_notation(Types::Square sq) {
    if (sq == Types::Square::NO_SQUARE) {return "-";}
    return std::string{Constants::FILE_LABELS[file_index(sq)], Constants::RANK_LABELS[rank_index(sq)]};
}

constexpr Types::Square notation_to_square(std::string_view notation) {
    assert(notation.size() == 2);

    const auto file_index = Constants::FILE_LABELS.find(notation[0]);
    const auto rank_index = Constants::RANK_LABELS.find(notation[1]);

    if (file_index == Constants::FILE_LABELS.npos ||
        rank_index == Constants::RANK_LABELS.npos)
    {
        return Types::Square::NO_SQUARE;
    }

    return file_rank_to_square(file_index, rank_index);
}

constexpr Types::Square bitboard_to_square(Types::Bitboard bb) {
    assert(pop_count(bb) <= 1);
    return lsb(bb);
}


// square operators

constexpr Types::Square& operator++ (Types::Square& sq) {
    if (sq == Types::Square::NO_SQUARE || sq == Types::Square::H8) {
        sq = Types::Square::NO_SQUARE;
    }
    else {
        sq = static_cast<Types::Square>(sq + 1);
    }

    return sq;
}

constexpr Types::Square& operator-- (Types::Square& sq) {
    if (sq == Types::Square::NO_SQUARE || sq == Types::Square::A1) {
        sq = Types::Square::NO_SQUARE;
    }
    else {
        sq = static_cast<Types::Square>(sq - 1);
    }

    return sq;
}

constexpr Types::Square operator++ (Types::Square& sq, int) {
    if (sq == Types::Square::NO_SQUARE) {return Types::Square::NO_SQUARE;}
    
    const auto tmp_sq     = sq;
    const auto new_sq_ind = sq + 1;
    sq = (new_sq_ind < Constants::NUM_SQUARES) ? static_cast<Types::Square>(new_sq_ind)
                                    : Types::Square::NO_SQUARE;

    return tmp_sq;
}

constexpr Types::Square operator-- (Types::Square& sq, int) {
    if (sq == Types::Square::NO_SQUARE) {return Types::Square::NO_SQUARE;}
    
    const auto tmp_sq     = sq;
    const auto new_sq_ind = sq - 1 ;
    sq = (new_sq_ind > 0) ? static_cast<Types::Square>(new_sq_ind)
                          : Types::Square::NO_SQUARE;

    return tmp_sq;
}

template<Concepts::bitboard_like BB_t>
constexpr Types::Bitboard convert_to_bitboard(BB_t bb) {
    if      constexpr (std::same_as<Types::Bitboard, BB_t>) {
        return bb;
    }
    else if constexpr (std::same_as<Types::Square, BB_t>) {
        return square_to_bitboard(bb);
    }
}

template<Concepts::bitboard_like BB_t_1, Concepts::bitboard_like BB_t_2>
constexpr Types::Bitboard operator| (BB_t_1 lhs, BB_t_2 rhs) {
    return convert_to_bitboard(lhs) | convert_to_bitboard(rhs);
}

template<Concepts::bitboard_like BB_t_1, Concepts::bitboard_like BB_t_2>
constexpr Types::Bitboard operator& (BB_t_1 lhs, BB_t_2 rhs) {
    return convert_to_bitboard(lhs) & convert_to_bitboard(rhs);
}


// square/bitboard output

constexpr std::string bitboard_to_string(Types::Bitboard bb) {

    std::string bb_str{""};

    for (const Types::Square& sq : Constants::ALL_SQUARES_PRINT_ORDER) {
        if (square_to_bitboard(sq) & bb) {bb_str += '1';}
        else                             {bb_str += '0';}

        if (file_index(sq) == 7) {bb_str += '\n';}
        else                     {bb_str += ' ';}
    }
    
    return bb_str;
}

// piece utils

constexpr Types::Color piece_color(Types::Piece p) {
    if (p == Types::Piece::NO_PIECE) {return Types::Color::NO_COLOR;}
    return static_cast<Types::Color>(p / Constants::NUM_PIECE_TYPES);
}

constexpr Types::PieceType piece_type(Types::Piece p) {
    if (p == Types::Piece::NO_PIECE) {return Types::PieceType::NO_PIECE_TYPE;}
    return static_cast<Types::PieceType>(p % Constants::NUM_PIECE_TYPES);
}

constexpr Types::Piece color_type_to_piece(Types::Color c, Types::PieceType pt) {
    if (c == Types::Color::NO_COLOR || pt == Types::PieceType::NO_PIECE_TYPE) {return Types::Piece::NO_PIECE;}
    return static_cast<Types::Piece>(pt + c*Constants::NUM_PIECE_TYPES);
}


// color operator

constexpr Types::Color operator~ (Types::Color c) {
    assert(c != Types::Color::NO_COLOR);
    if      (c == Types::Color::WHITE) {return Types::Color::BLACK;}
    else                               {return Types::Color::WHITE;}
}


// castle utils

constexpr std::string castle_to_string(Types::Castle c) {
    if (c == Constants::CastlingRights::NONE) {return std::string{"-"};}

    std::string castle_string{""};

    if (c & Constants::CastlingRights::W_SHORT) {
        castle_string += 'K';
    }
    if (c & Constants::CastlingRights::W_LONG) {
        castle_string += 'Q';
    }
    if (c & Constants::CastlingRights::B_SHORT) {
        castle_string += 'k';
    }
    if (c & Constants::CastlingRights::B_LONG) {
        castle_string += 'q';
    }

    return castle_string;
}

constexpr std::pair<Types::Square, Types::Square> castle_rook_from_to(Types::Castle castle) {
    if      (castle == Constants::CastlingRights::W_SHORT) {
        return std::make_pair(Types::Square::H1, Types::Square::F1);
    }
    else if (castle == Constants::CastlingRights::B_SHORT) {
        return std::make_pair(Types::Square::H8, Types::Square::F8);
    }
    else if (castle == Constants::CastlingRights::W_LONG) {
        return std::make_pair(Types::Square::A1, Types::Square::D1);
    }
    else if (castle == Constants::CastlingRights::B_LONG) {
        return std::make_pair(Types::Square::A8, Types::Square::D8);
    }

    else {
        assert(false);
    }
}

constexpr std::pair<Types::Square, Types::Square> castle_king_from_to(Types::Castle castle) {
    if      (castle == Constants::CastlingRights::W_SHORT) {
        return std::make_pair(Types::Square::E1, Types::Square::G1);
    }
    else if (castle == Constants::CastlingRights::B_SHORT) {
        return std::make_pair(Types::Square::E8, Types::Square::G8);
    }
    else if (castle == Constants::CastlingRights::W_LONG) {
        return std::make_pair(Types::Square::E1, Types::Square::C1);
    }
    else if (castle == Constants::CastlingRights::B_LONG) {
        return std::make_pair(Types::Square::E8, Types::Square::C8);
    }

    else {
        assert(false);
    }
}


// misc utils

template<std::integral T = int>
constexpr int char_to_int(char c) {
    return static_cast<T>(c - '0');
}

template<std::integral T = int>
constexpr int int_to_char(T i) {
    return static_cast<char>('0' + i);
}

template<std::integral T = std::size_t>
constexpr T ply_to_full(T ply) {
    return (ply % 2 == 1) ? ply / 2 + 1 // white
                          : ply / 2;    // black
}

template<std::integral T = std::size_t>
constexpr T full_to_ply(T full, Types::Color side_to_move) {
    return (side_to_move == Types::Color::WHITE) ? 2 * full - 1
                                                 : 2 * full;
}

template<typename T=std::chrono::milliseconds>
constexpr std::chrono::time_point<std::chrono::high_resolution_clock, T> current_time() {
    const auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::time_point_cast<T>(now);
}

} // MPChess namespace