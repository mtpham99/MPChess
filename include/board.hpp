// board.hpp

#pragma once

#include "defs.hpp"      // types, constants

#include "movelist.hpp"  // movelist

#include "attacks.hpp"   // attacks

#include <array>         // array
#include <string_view>   // string_view

#include <iostream>      // ostream, cout


namespace MPChess {

class Board {
private:

    // board

    std::array<Types::Piece, Constants::NUM_SQUARES>     pieces;
    std::array<Types::Bitboard, Constants::NUM_PIECES>   piece_bbs;
    std::array<Types::Bitboard, Constants::NUM_COLORS+1> occupancy_bbs;


    // game state variables

    Types::Color  side_to_move;
    std::size_t   ply_clock;
    std::size_t   ply_played;
    std::size_t   ply_move_number;
    Types::Square enpassant_square;
    Types::Castle castling_rights;
    Types::Key    zobrist_key;


    // state/move history

    std::array<Types::StateInfo, Constants::MAX_PLY> state_history;
    RegularMoveList                                  move_list;


    // generate zobrist key

    void generate_key();

public:

    // constructors

    Board();
    Board(std::string&& fen);

    
    // no copying

    Board(const Board& board)             = delete;
    Board& operator= (const Board& board) = delete;


    // set/get fen

    void set_fen(std::string&& fen);
    std::string get_fen() const;


    // utils

    void print(std::ostream& os = std::cout) const;


    // board getters

    Types::Piece    get_square_piece(Types::Square sq)                const;
    Types::Bitboard get_occupation_bb(Types::Color c)                 const;
    Types::Bitboard get_piece_bb(Types::Piece p)                      const;
    Types::Bitboard get_piece_bb(Types::Color c, Types::PieceType pt) const;
    Types::Bitboard get_piece_type_bb(Types::PieceType pt)            const;

    Types::Color    get_side_to_move()     const; 
    Types::Square   get_enpassant_square() const;
    Types::Castle   get_castling_rights()  const;

    std::size_t     get_ply_clock()        const;
    std::size_t     get_ply_played()       const;
    std::size_t     get_ply_move_number()  const;
    std::size_t     get_full_move_number() const;
    Types::Key      get_zobrist_key()      const;

    const RegularMoveList& get_move_list() const;

    template<Types::Color side>
    requires (side != Types::Color::NO_COLOR)
    Types::Square get_king_square() const {
        const Types::Piece king = color_type_to_piece(side, Types::PieceType::KING);
        return bitboard_to_square(this->piece_bbs[king]);
    }


    // make/unmake move

    void make_move(Move move);
    void unmake_move();

    void update_enpassant_square(Move move);
    void update_castling_rights(Move move);

    Types::Piece  moved_piece(Move move)     const;
    Types::Square captured_square(Move move) const;
    Types::Piece  captured_piece(Move move)  const;

    void remove_piece(Types::Square sq);
    void add_piece(Types::Square sq, Types::Piece p);
    void move_piece(Types::Square from, Types::Square to);


    // validate board

    Types::Bitboard is_double_occupied()     const;
    Types::Bitboard is_occupation_mismatch() const;
    void            validate()               const;
    bool            is_repetition()          const;


    // attacks

    template<Concepts::bitboard_like BB_t>
    Types::Bitboard attacks_to(BB_t bb) const {

        if (is_empty(bb)) {return Constants::EMPTY;}

        const Types::Bitboard occupancy = ~(this->occupancy_bbs[Types::Color::NO_COLOR]);

        return (Attacks::attacks<Types::PieceType::PAWN, Types::Color::WHITE>(bb) & this->get_piece_bb(Types::Piece::B_PAWN))
             | (Attacks::attacks<Types::PieceType::PAWN, Types::Color::BLACK>(bb) & this->get_piece_bb(Types::Piece::W_PAWN))
             | (Attacks::attacks<Types::PieceType::KNIGHT>(bb) & this->get_piece_type_bb(Types::PieceType::KNIGHT))
             | (Attacks::attacks<Types::PieceType::KING>(bb)   & this->get_piece_type_bb(Types::PieceType::KING))
             | (Attacks::attacks<Types::PieceType::BISHOP>(bb, occupancy) & (this->get_piece_type_bb(Types::PieceType::BISHOP) | this->get_piece_type_bb(Types::PieceType::QUEEN)))
             | (Attacks::attacks<Types::PieceType::ROOK>(bb, occupancy)   & (this->get_piece_type_bb(Types::PieceType::ROOK)   | this->get_piece_type_bb(Types::PieceType::QUEEN)));
    }

    template<bool side_to_move>
    bool is_check() const {
        
        const Types::Color    side = (side_to_move) ?  this->side_to_move
                                                 : ~this->side_to_move;
        
        const Types::Square   king     = bitboard_to_square(this->get_piece_bb(side, Types::PieceType::KING));
        const Types::Bitboard enemy    = this->get_occupation_bb(~side);
        const Types::Bitboard checkers = this->attacks_to(king) & enemy;

        return !is_empty(checkers);
    }
};

// non-member operators
std::ostream& operator<< (std::ostream& os, const Board& board);


} // MPChess namespace