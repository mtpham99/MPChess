// defs.hpp

#pragma once

#include <cstdint>     // fixed width types
#include <string_view> // string_view
#include <array>       // array
#include <chrono>      // time


namespace MPChess {

namespace Types {

using Bitboard = uint64_t;

using Castle = uint8_t;

using Key = uint64_t;

using TimePoint    = std::chrono::time_point<std::chrono::high_resolution_clock, std::chrono::milliseconds>;
using TimeDuration = std::chrono::milliseconds;
using Milliseconds = std::chrono::milliseconds;

using MoveFlag  = uint8_t;
using MoveMask  = uint16_t;
using MoveData  = uint16_t;
using MoveScore = uint32_t;

using Eval  = int16_t;
using Depth = uint16_t;

enum Square : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    NO_SQUARE
};

enum Color : int {
    WHITE, BLACK, NO_COLOR
};

enum PieceType : int {
    PAWN, KNIGHT, BISHOP,
    ROOK, QUEEN, KING,
    NO_PIECE_TYPE
};

enum Piece : int {
    W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    NO_PIECE
};


enum class FlipType : int {
    VERTICAL, HORIZONTAL, DIAG, ANTIDIAG, NO_FLIP
};

enum class RotateType : int {
    CW90, CW180, CCW90, NO_ROTATE
};

enum StepType : int {
    N   =     8, E   =     1, W   =    -1, S   =    -8,
    NN  =   N+N,                           SS  =   S+S,
    NW  =   N+W, NE  =   N+E, SW  =   S+W, SE  =   S+E,
    NWW = N+W+W, NNW = N+N+W, NNE = N+N+E, NEE = N+E+E,
    SEE = S+E+E, SSE = S+S+E, SSW = S+S+W, SWW = S+W+W,
    NO_STEP = 0
};

enum class MoveGenType : int {
    QUIET,
    CAPTURE,
    PSEUDOLEGAL
};

enum class NodeType : uint8_t {
    NULL_NODE,
    PV_NODE,
    ALL_NODE,
    CUT_NODE
};

struct StateInfo {
    Key         zobrist_key;
    std::size_t ply_clock;
    Square      enpassant_square;
    Castle      castling_rights;

    Piece       piece_captured;
};

} // Types namespace


namespace Constants {

// bitboard constants

inline constexpr Types::Bitboard RANK_1        = 0x00000000000000ffull;
inline constexpr Types::Bitboard RANK_2        = 0x000000000000ff00ull;
inline constexpr Types::Bitboard RANK_3        = 0x0000000000ff0000ull;
inline constexpr Types::Bitboard RANK_4        = 0x00000000ff000000ull;
inline constexpr Types::Bitboard RANK_5        = 0x000000ff00000000ull;
inline constexpr Types::Bitboard RANK_6        = 0x0000ff0000000000ull;
inline constexpr Types::Bitboard RANK_7        = 0x00ff000000000000ull;
inline constexpr Types::Bitboard RANK_8        = 0xff00000000000000ull;

inline constexpr Types::Bitboard FILE_A        = 0x0101010101010101ull;
inline constexpr Types::Bitboard FILE_B        = 0x0202020202020202ull;
inline constexpr Types::Bitboard FILE_C        = 0x0404040404040404ull;
inline constexpr Types::Bitboard FILE_D        = 0x0808080808080808ull;
inline constexpr Types::Bitboard FILE_E        = 0x1010101010101010ull;
inline constexpr Types::Bitboard FILE_F        = 0x2020202020202020ull;
inline constexpr Types::Bitboard FILE_G        = 0x4040404040404040ull;
inline constexpr Types::Bitboard FILE_H        = 0x8080808080808080ull;

inline constexpr Types::Bitboard DIAG_A1H8     = 0x8040201008040201ull;
inline constexpr Types::Bitboard DIAG_H1A8     = 0x0102040810204080ull;

inline constexpr Types::Bitboard BLACK_SQUARES = 0xaa55aa55aa55aa55ull;
inline constexpr Types::Bitboard WHITE_SQUARES = 0x55aa55aa55aa55aaull;

inline constexpr Types::Bitboard EMPTY         = 0x0000000000000000ull;
inline constexpr Types::Bitboard UNIVERSE      = 0xffffffffffffffffull;


// size constants

inline constexpr std::size_t NUM_COLORS        =    2;
inline constexpr std::size_t NUM_PIECE_TYPES   =    6;
inline constexpr std::size_t NUM_PIECES        =   12;
inline constexpr std::size_t NUM_CASTLE_STATES =   16;

inline constexpr std::size_t NUM_SQUARES       =   64;
inline constexpr std::size_t NUM_RANKS         =    8;
inline constexpr std::size_t NUM_FILES         =    8;
inline constexpr std::size_t RANK_SIZE         =    8;
inline constexpr std::size_t FILE_SIZE         =    8;

inline constexpr std::size_t MAX_PLY           = 1024;

inline constexpr std::size_t NUM_KILLER_MOVES  =    3;


// eval constants

namespace Evals {

inline constexpr Types::Eval INF  = 30000;
inline constexpr Types::Eval MATE = 20000;

} // Eval namespace




// castle constants

namespace CastlingRights {

inline constexpr Types::Castle NONE            = 0b0000;
inline constexpr Types::Castle W_SHORT         = 0b0001;
inline constexpr Types::Castle W_LONG          = 0b0010;
inline constexpr Types::Castle W_BOTH          = 0b0011;
inline constexpr Types::Castle B_SHORT         = 0b0100;
inline constexpr Types::Castle W_SHORT_B_SHORT = 0b0101;
inline constexpr Types::Castle W_LONG_B_SHORT  = 0b0110;
inline constexpr Types::Castle W_BOTH_B_SHORT  = 0b0111;
inline constexpr Types::Castle B_LONG          = 0b1000;
inline constexpr Types::Castle W_SHORT_B_LONG  = 0b1001;
inline constexpr Types::Castle W_LONG_B_LONG   = 0b1010;
inline constexpr Types::Castle W_BOTH_B_LONG   = 0b1011;
inline constexpr Types::Castle B_BOTH          = 0b1100;
inline constexpr Types::Castle W_SHORT_B_BOTH  = 0b1101;
inline constexpr Types::Castle W_LONG_B_BOTH   = 0b1110;
inline constexpr Types::Castle ALL             = 0b1111;
} // Castle namespace


// fen constants

inline constexpr std::string_view STARTING_FEN{"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};


// label constants

inline constexpr std::string_view COLOR_LABELS{"wb"};
inline constexpr std::string_view PIECE_TYPE_LABELS{"pnbrqk"};
inline constexpr std::string_view PIECE_LABELS{"PNBRQKpnbrqk"};
inline constexpr std::string_view FILE_LABELS{"abcdefgh"};
inline constexpr std::string_view RANK_LABELS{"12345678"};


// array constants

inline constexpr std::array<Types::Color, NUM_COLORS> ALL_COLORS = {Types::WHITE, Types::BLACK};

inline constexpr std::array<Types::PieceType, NUM_PIECE_TYPES> ALL_PIECE_TYPES = {
    Types::PAWN, Types::KNIGHT, Types::BISHOP,
    Types::ROOK, Types::QUEEN,  Types::KING
};

inline constexpr std::array<Types::Piece, NUM_PIECES> ALL_PIECES = {
    Types::W_PAWN, Types::W_KNIGHT, Types::W_BISHOP,
    Types::W_ROOK, Types::W_QUEEN,  Types::W_KING,
    Types::B_PAWN, Types::B_KNIGHT, Types::B_BISHOP,
    Types::B_ROOK, Types::B_QUEEN,  Types::B_KING
};

inline constexpr std::array<Types::Square, NUM_SQUARES> ALL_SQUARES = {
    Types::A1, Types::B1, Types::C1, Types::D1, Types::E1, Types::F1, Types::G1, Types::H1,
    Types::A2, Types::B2, Types::C2, Types::D2, Types::E2, Types::F2, Types::G2, Types::H2,
    Types::A3, Types::B3, Types::C3, Types::D3, Types::E3, Types::F3, Types::G3, Types::H3,
    Types::A4, Types::B4, Types::C4, Types::D4, Types::E4, Types::F4, Types::G4, Types::H4,
    Types::A5, Types::B5, Types::C5, Types::D5, Types::E5, Types::F5, Types::G5, Types::H5,
    Types::A6, Types::B6, Types::C6, Types::D6, Types::E6, Types::F6, Types::G6, Types::H6,
    Types::A7, Types::B7, Types::C7, Types::D7, Types::E7, Types::F7, Types::G7, Types::H7,
    Types::A8, Types::B8, Types::C8, Types::D8, Types::E8, Types::F8, Types::G8, Types::H8,
};

inline constexpr std::array<Types::Bitboard, NUM_SQUARES> SQUARE_BITBOARDS = [] consteval {
    std::array<Types::Bitboard, NUM_SQUARES> sq_bbs;
    for (std::size_t sq_ind=0; sq_ind<NUM_SQUARES; ++sq_ind) {
        sq_bbs[sq_ind] = (1ull << sq_ind);
    }
    return sq_bbs;
}();

inline constexpr std::array<Types::Square, NUM_SQUARES> ALL_SQUARES_BIG_ENDIAN = [] consteval {
    std::array<Types::Square, NUM_SQUARES> all_sqs_big_endian;
    for (std::size_t sq_ind=0; sq_ind<NUM_SQUARES; ++sq_ind) {
        all_sqs_big_endian[sq_ind] = static_cast<Types::Square>(sq_ind ^ 7 ^ 56);
    }
    return all_sqs_big_endian;
}();

inline constexpr std::array<Types::Square, NUM_SQUARES> ALL_SQUARES_PRINT_ORDER = [] consteval {
    std::array<Types::Square, NUM_SQUARES> all_sqs_print_order;
    for (std::size_t sq_ind=0; sq_ind<NUM_SQUARES; ++sq_ind) {
        all_sqs_print_order[sq_ind] = static_cast<Types::Square>(sq_ind ^ 56);
    }
    return all_sqs_print_order;
}();

} // Constants namespace


namespace Concepts {

template<typename T>
concept bitboard_like = std::same_as<Types::Bitboard, T> || std::same_as<Types::Square, T>;

template<typename T>
concept shift_like = std::integral<T> || std::same_as<Types::StepType, T>;

} // Concepts namespace


} // MPChess namespace
