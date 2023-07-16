// board.cpp

#include "board.hpp"

#include "defs.hpp"    // types, constants
#include "utils.hpp"   // char_to_int, operators, etc.

#include "zobrist.hpp" // zobrist hashes

#include <string>      // string

#include <iostream>    // ostream, cout
#include <sstream>     // istringstream
#include <stdexcept>   // exceptions
#include <cmath>       // ceil

using namespace MPChess::Types;
using namespace MPChess::Constants;


namespace MPChess {

// constructor

Board::Board() {
    this->set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

Board::Board(std::string&& fen) {
    this->set_fen(std::move(fen));
}


// set/get fen

void Board::set_fen(std::string&& fen) {

    // string parser
    std::istringstream stream(std::move(fen));
    std::string chunk;


    // 1. read board/pieces

    // clear board/pieces
    std::fill(this->piece_bbs.begin(), this->piece_bbs.end(), EMPTY);
    this->occupancy_bbs = {EMPTY, EMPTY, UNIVERSE}; // white, black, unoccupied
    std::fill(this->pieces.begin(), this->pieces.end(), Piece::NO_PIECE);

    stream >> chunk;
    Square sq = ALL_SQUARES_PRINT_ORDER[0];
    for (const char& c : chunk) {

        // end of rank char
        if (c == '/') {
            sq = shift(sq, -15);
            continue;
        }

        // empty squares char
        else if (std::size_t skip = RANK_LABELS.find(c); skip != RANK_LABELS.npos) {
            sq = shift(sq, skip);
        }

        // piece char
        else if (std::size_t piece_index = PIECE_LABELS.find(c); piece_index != PIECE_LABELS.npos) {
            
            // add piece to board
            this->piece_bbs[piece_index] |= square_to_bitboard(sq);
            this->pieces[sq] = static_cast<Piece>(piece_index); 
        }

        // invalid char
        else {
            assert(false);
        }
    
        // if not h file
        if (file_index(sq) != 7) {
            sq = shift(sq, 1);
        }
    }

    // update occupancy bitboards
    for (const Piece& piece : ALL_PIECES) {
        const Color color = piece_color(piece);
        this->occupancy_bbs[color] ^= this->piece_bbs[piece];
        this->occupancy_bbs[NO_COLOR]  ^= this->piece_bbs[piece];
    }


    // 2. read color to move
    stream >> chunk;
    if      (chunk == "w") {this->side_to_move = Color::WHITE;}
    else if (chunk == "b") {this->side_to_move = Color::BLACK;}
    else                   {assert(false);}


    // 3. read castling rights
    stream >> chunk;
    this->castling_rights = CastlingRights::NONE;
    for (const char& c : chunk) {
        if (c == '-') {break;} // no castle rights

        else if (c == 'K') {this->castling_rights |= CastlingRights::W_SHORT;}
        else if (c == 'Q') {this->castling_rights |= CastlingRights::W_LONG;}
        else if (c == 'k') {this->castling_rights |= CastlingRights::B_SHORT;}
        else if (c == 'q') {this->castling_rights |= CastlingRights::B_LONG;}

        else {assert(false);}
    }


    // 4. enpassant square
    stream >> chunk;
    this->enpassant_square = Square::NO_SQUARE;
    if (chunk != "-") {
        this->enpassant_square = notation_to_square(chunk);
    }


    // 5. half (50-move rule) move clock
    if (stream.eof()) {
        this->ply_clock = 0;
    }
    else {
        stream >> chunk;
        this->ply_clock = std::stoi(chunk);
    }


    // 6. full move number
    if (stream.eof()) {
        this->ply_move_number = 1;
    }
    else {
        stream >> chunk;
        this->ply_move_number = full_to_ply(std::stoi(chunk), this->side_to_move);
    }

    // reset moves played
    this->ply_played = 0;
    this->move_list.shrink(0);

    // generate zobrist key
    this->generate_key();
}

std::string Board::get_fen() const {

    std::string fen{""};

    // get position
    std::size_t empty_count = 0;
    for (const Square& sq : ALL_SQUARES_PRINT_ORDER) {

        // empty square
        if (this->pieces[sq] == Piece::NO_PIECE) {
            ++empty_count;
        }

        // non-empty square
        else {

            // add empty squares to fen before adding piece
            if (empty_count) {
                fen += int_to_char(empty_count);
                empty_count = 0;
            }

            // add piece to fen
            fen += PIECE_LABELS[this->pieces[sq]];
        }


        // end of rank
        if (file_index(sq) == 7) {
            
            // print empty squares
            if (empty_count) {
                fen += int_to_char(empty_count);
                empty_count = 0;
            }

            // if not last rank, print new rank char
            if (sq != Square::H1) {
                fen += '/';
            }
        }
    }
    fen += ' ';


    // color to move
    fen += COLOR_LABELS[this->side_to_move];
    fen += ' ';

    // castling rights
    fen += castle_to_string(this->castling_rights);
    fen += ' ';


    // enpassant square
    fen += square_to_notation(this->enpassant_square);
    fen += ' ';


    // move counters
    fen += int_to_char(this->ply_clock);
    fen += ' ';
    fen += int_to_char(this->ply_move_number);

    return fen;
}


// board getters

Piece Board::get_square_piece(Square sq) const {
    return this->pieces[sq];
}

Bitboard Board::get_occupation_bb(Color c) const {
    return this->occupancy_bbs[c];
}

Bitboard Board::get_piece_bb(Piece p) const {
    if (p == Piece::NO_PIECE) {
        return this->get_occupation_bb(Color::NO_COLOR);
    }
    else {
        return this->piece_bbs[p];
    }
}

Bitboard Board::get_piece_bb(Color c, PieceType pt) const {
    return this->get_piece_bb(color_type_to_piece(c, pt));
}

Bitboard Board::get_piece_type_bb(PieceType pt) const {
    return this->get_piece_bb(color_type_to_piece(Color::WHITE, pt))
         | this->get_piece_bb(color_type_to_piece(Color::BLACK, pt));
}

Color Board::get_side_to_move() const {
    return this->side_to_move;
}

Square Board::get_enpassant_square() const {
    return this->enpassant_square;
}

Castle Board::get_castling_rights() const {
    return this->castling_rights;
}

std::size_t Board::get_ply_clock() const {
    return this->ply_clock;
}

std::size_t Board::get_ply_played() const {
    return this->ply_played;
}

std::size_t Board::get_ply_move_number() const {
    return this->ply_move_number;
}

std::size_t Board::get_full_move_number() const {
    return ply_to_full(this->ply_move_number);
}

Key Board::get_zobrist_key() const {
    return this->zobrist_key;
}

const RegularMoveList& Board::get_move_list() const {
    return this->move_list;
}


// make/unmake move

void Board::make_move(Move move) {
#ifndef NDEBUG
    this->validate();
#endif

    // get all move info
    const Color& color_moved    = this->side_to_move;
    const Piece  piece_captured = this->captured_piece(move);
    const Square from           = move.get_from_square();
    const Square to             = move.get_to_square(); 

    // add state to history
    this->state_history[this->ply_played] = {
        .zobrist_key      = this->zobrist_key,
        .ply_clock        = this->ply_clock,
        .enpassant_square = this->enpassant_square,
        .castling_rights  = this->castling_rights,

        .piece_captured   = piece_captured
    };
    this->move_list.add_move(move);

    // castle: move castle rook
    if (move.is_castle()) {

        const Castle castle_color_mask = (color_moved == Color::WHITE) ? CastlingRights::W_BOTH
                                                                       : CastlingRights::B_BOTH;

        const auto [rook_from, rook_to] = castle_rook_from_to(move.get_castle() & castle_color_mask);

        // move rook
        this->move_piece(rook_from, rook_to);

        // king is moved below
    }

    // capture: removed captured piece
    if (move.is_capture()) {
        const Square square_captured = this->captured_square(move);
        this->remove_piece(square_captured);
    }

    // move piece (non-promote)
    if (!move.is_promote()) {
        this->move_piece(from, to);
    }


    // pawn promotion
    else {
        this->remove_piece(from);

        const Piece piece_promote = color_type_to_piece(color_moved, move.get_promote_piece_type());
        this->add_piece(to, piece_promote);
    }

    // update enpassant
    this->update_enpassant_square(move);

    // update castle state (if any remaining)
    this->update_castling_rights(move);

    // update color
    this->side_to_move = ~(this->side_to_move);
    this->zobrist_key ^= Zobrist::get_color_key();

    // update move counters
    if (move.is_capture() || piece_type(piece_captured) == PieceType::PAWN) {
        this->ply_clock = 0;
    }
    else {
    ++(this->ply_clock);
    }

    ++(this->ply_played);
    ++(this->ply_move_number);

#ifndef NDEBUG
    this->validate();
#endif
}

void Board::unmake_move() {

#ifndef NDEBUG
    this->validate();
#endif

    // get previous move info
    const Move&      prev_move  = this->move_list[this->ply_played - 1];
    const StateInfo& prev_state = this->state_history[this->ply_played - 1];
    
    const Color& color_moved = ~this->side_to_move;
    const Square from        = prev_move.get_from_square();
    const Square to          = prev_move.get_to_square();


    // castle : move rook back
    if (prev_move.is_castle()) {

        const Castle castle_color_mask = (color_moved == Color::WHITE) ? CastlingRights::W_BOTH
                                                                       : CastlingRights::B_BOTH;
        const auto [rook_from, rook_to] = castle_rook_from_to(prev_move.get_castle() & castle_color_mask);

        // move rook back
        this->move_piece(rook_to, rook_from);

        // king moved back below
    }

    // move piece back (non-promote)
    if (!prev_move.is_promote()) {
        this->move_piece(to, from);
    }

    // move piece back (promote)
    else {
        this->remove_piece(to);
        this->add_piece(from, color_type_to_piece(color_moved, PieceType::PAWN));
    }

    // capture : place captured piece back
    if (prev_move.is_capture()) {
        
        // enpassant capture
        if (prev_move.is_enpassant()) {
            
            const Square square_captured = (color_moved == Color::WHITE) ? step<StepType::S>(to)
                                                                         : step<StepType::N>(to);

            this->add_piece(square_captured, prev_state.piece_captured);
        }

        // regular capture
        else {
            this->add_piece(to, prev_state.piece_captured);
        }        
    }

    // restore irreversible state info
    this->zobrist_key      = prev_state.zobrist_key;
    this->enpassant_square = prev_state.enpassant_square;
    this->castling_rights  = prev_state.castling_rights;
    this->ply_clock        = prev_state.ply_clock;

    // move number
    --(this->ply_move_number);
    --(this->ply_played);

    // color to move
    this->side_to_move = color_moved;

    // movelist update
    this->move_list.shrink(this->move_list.get_size() - 1);

#ifndef NDEBUG
    this->validate();
#endif
}

void Board::update_enpassant_square(Move move) {

    // get move info
    const Square to          = move.get_to_square();
    const Color  color_moved = this->side_to_move;

    // remove old enpassant square
    if (!is_empty(this->enpassant_square)) {
        this->zobrist_key      ^= Zobrist::get_enpassant_key(this->enpassant_square);
        this->enpassant_square  = Square::NO_SQUARE;
    }

    // double pawn push (check for new enpassant square)
    if (move.is_double_pawn_push()) {
        const Bitboard adjacent_sqs = step<StepType::E>(to)
                                    | step<StepType::W>(to);

        const Bitboard& enemy_pawns = this->piece_bbs[color_type_to_piece(~color_moved, PieceType::PAWN)];

        // adjacent to enemy pawn
        if (!is_empty(adjacent_sqs & enemy_pawns)) {
            this->enpassant_square = (color_moved == Color::WHITE) ? step<StepType::S>(to)
                                                                   : step<StepType::N>(to);
        }
    }
}

void Board::update_castling_rights(Move move) {

    // no castling rights remaining
    if (this->castling_rights == CastlingRights::NONE) {return;}

    // unhash key for current castling rights
    this->zobrist_key ^= Zobrist::get_castle_key(this->castling_rights);

    // get move info
    const Piece&    piece_moved      = this->pieces[move.get_to_square()];
    const PieceType piece_type_moved = piece_type(piece_moved);
    const Color     color_moved      = piece_color(piece_moved);
    const Color     color_enemy      = ~color_moved;

    const Piece&    piece_captured      = this->state_history[this->ply_played].piece_captured; // note history has been updated but move counters have not
    const PieceType piece_type_captured = piece_type(piece_captured);

    const Castle castle_color_mask = (color_moved == Color::WHITE) ? CastlingRights::W_BOTH
                                                                   : CastlingRights::B_BOTH;

    // king move : remove all castle rights for color moved
    if (piece_type_moved == PieceType::KING) {
        this->castling_rights &= ~(castle_color_mask);
    }

    // rook move : remove castle rights for rook side for color moved
    if (piece_type_moved == PieceType::ROOK) {

        const Square rook_from_long_castle  = (color_moved == Color::WHITE) ? Square::A1
                                                                            : Square::A8;
        const Square rook_from_short_castle = (color_moved == Color::WHITE) ? Square::H1
                                                                            : Square::H8;

        // moved queen side rook: remove long castle
        if (rook_from_long_castle == move.get_from_square()) {
            this->castling_rights &= ~(castle_color_mask & CastlingRights::W_LONG_B_LONG);
        }

        // moved king side rook : remove short castle
        else if (rook_from_short_castle == move.get_from_square()) {
            this->castling_rights &= ~(castle_color_mask & CastlingRights::W_SHORT_B_SHORT);
        }
    }

    // rook captured : remove castle rights for rook side for color captured
    if (piece_type_captured == PieceType::ROOK) {

        const Square rook_from_long_castle  = (color_enemy == Color::WHITE) ? Square::A1
                                                                            : Square::A8;
        const Square rook_from_short_castle = (color_enemy == Color::WHITE) ? Square::H1
                                                                            : Square::H8;

        // note capture square is always to square, because cannot enpassant capture on first/last rank

        // captured queen side rook : remove long castle
        if (rook_from_long_castle == move.get_to_square()) {
            this->castling_rights &= ~(~castle_color_mask & CastlingRights::W_LONG_B_LONG);
        }

        // captured king side rook : remove short castle
        else if (rook_from_short_castle == move.get_to_square()) {
            this->castling_rights &= ~(~castle_color_mask & CastlingRights::W_SHORT_B_SHORT);
        }
    }

    // update zobrist with new castling rights
    this->zobrist_key ^= Zobrist::get_castle_key(this->castling_rights);
}

void Board::remove_piece(Square sq) {
    const Piece captured_piece = this->pieces[sq];
    const Color captured_color = piece_color(captured_piece);

    // remove piece from bitboards
    this->pieces[sq]                      = Piece::NO_PIECE;
    this->piece_bbs[captured_piece]      ^= square_to_bitboard(sq);
    this->occupancy_bbs[captured_color]  ^= square_to_bitboard(sq);
    this->occupancy_bbs[Color::NO_COLOR] |= square_to_bitboard(sq);

    // update zobrist key
    this->zobrist_key ^= Zobrist::get_piece_square_key(captured_piece, sq);
}

void Board::add_piece(Square sq, Piece p) {
    const Color color_piece = piece_color(p);

    // add piece to bitboards
    this->pieces[sq]    = p;
    this->piece_bbs[p]                   |= square_to_bitboard(sq);
    this->occupancy_bbs[color_piece]     |= square_to_bitboard(sq);
    this->occupancy_bbs[Color::NO_COLOR] ^= square_to_bitboard(sq);

    // update zobrist key
    this->zobrist_key ^= Zobrist::get_piece_square_key(p, sq);
}

void Board::move_piece(Square from, Square to) {
    const Piece piece_moved = this->pieces[from];
    const Color color_moved = piece_color(piece_moved);

    // move piece
    this->pieces[from] = Piece::NO_PIECE;
    this->pieces[to]   = piece_moved;

    this->piece_bbs[piece_moved]         ^= (from | to);
    this->occupancy_bbs[color_moved]     ^= (from | to);
    this->occupancy_bbs[Color::NO_COLOR] ^= (from | to);

    // update zobrist key
    this->zobrist_key ^= Zobrist::get_piece_square_key(piece_moved, from)
                      ^  Zobrist::get_piece_square_key(piece_moved, to);
}

Square Board::captured_square(Move move) const {
    if (!move.is_capture()) {return Square::NO_SQUARE;}

    // enpassant capture
    if (move.is_enpassant()) {
        if (this->side_to_move == Color::WHITE) {
            return step<StepType::S>(move.get_to_square());
        }
        else if (this->side_to_move == Color::BLACK) {
            return step<StepType::N>(move.get_to_square());
        }
        else {
            assert(false);
        }
    }

    // regular capture
    else {
        return move.get_to_square();
    }
}

Piece Board::captured_piece(Move move) const {
    return this->pieces[this->captured_square(move)];
}


// validate board

Bitboard Board::is_double_occupied() const {

    Bitboard occupied = EMPTY;

    for (const Piece& p : ALL_PIECES) {

        const Bitboard& piece_bb = this->piece_bbs[p];

        if (is_empty(occupied & piece_bb)) {occupied |= piece_bb;}
        else                               {return occupied & piece_bb;}
    }

    return EMPTY;
}

Bitboard Board::is_occupation_mismatch() const {

    Bitboard occupied = EMPTY;

    for (const Color& c : ALL_COLORS) {

        Bitboard color_bb = EMPTY;

        for (const PieceType& pt : ALL_PIECE_TYPES) {
            color_bb |= this->piece_bbs[color_type_to_piece(c, pt)];
        }

        if (color_bb != this->occupancy_bbs[c]) {return color_bb ^ this->occupancy_bbs[c];}
    
        occupied |= color_bb;
    }

    if (~occupied != this->occupancy_bbs[Color::NO_COLOR]) {return !occupied ^ this->occupancy_bbs[Color::NO_COLOR];}

    return EMPTY;
}

void Board::validate() const {

    Bitboard double_occupation   = this->is_double_occupied();
    Bitboard occupation_mismatch = this->is_occupation_mismatch();

    if (!is_empty(double_occupation)) {
        std::cout << *this << '\n';
        print_bitboard(double_occupation);
        throw std::logic_error("Board has double occupation!");
    }

    if (!is_empty(occupation_mismatch)) {
        std::cout << *this << '\n';
        print_bitboard(occupation_mismatch);
        throw std::logic_error("Board has occupation mismatch!");
    }
}

bool Board::is_repetition() const {
    if (this->ply_clock <= 3) {return false;}

    for (auto p_state  = this->state_history.rbegin() + 1;
              p_state != this->state_history.rbegin() + 1 + this->ply_clock;
            ++p_state)
    {
        if (p_state->zobrist_key == this->zobrist_key) {
            return true;
        }
    }

    return false;
}


// zobrist key methods

void Board::generate_key() {

    // reset key
    this->zobrist_key = 0;

    // piece square
    for (const Square& sq : ALL_SQUARES) {
         const Piece& p = this->pieces[sq];

        if (p != Piece::NO_PIECE) {
            this->zobrist_key ^= Zobrist::get_piece_square_key(p, sq);
        }
    }

    // enpassant square
    if (!is_empty(this->enpassant_square)) {
        this->zobrist_key ^= Zobrist::get_enpassant_key(this->enpassant_square);
    }

    // castling rights
    this->zobrist_key ^= Zobrist::get_castle_key(this->castling_rights);

    // color to move
    if (this->side_to_move == BLACK) {
        this->zobrist_key ^= Zobrist::get_color_key();
    }
}


// utils

void Board::print(std::ostream& os) const {

    // print move counters and color to move
    os << "Move #" << ply_to_full(this->ply_move_number);
    os << "\n50 Move Counter (Ply): " << this->ply_clock;
    os << "\nColor to move: " << COLOR_LABELS[this->side_to_move]; 

    // line break
    os << "\n  " + std::string(19, '=') << '\n';

    // print board
    for (const Square& sq : ALL_SQUARES_PRINT_ORDER) {

        // if start of rank, print rank label
        if (file_index(sq) == 0) {
            os << RANK_LABELS[rank_index(sq)] << " | ";
        }

        // print content of square
        const Piece& sq_piece = this->pieces[sq];
        if (sq_piece != Piece::NO_PIECE) {
            os << PIECE_LABELS[sq_piece] << ' ';
        }
        else {
            os << ". ";
        }

        // if end of rank, go to next line
        if (file_index(sq) == 7) {
            os << "|\n";
        }
    }


    // print file labels below board
    os << "  ";
    os << std::string(19, '=') << '\n';
    os << "   A B C D E F G H\n";


    // print fen
    os << "FEN: \"" << this->get_fen() << "\"\n";


    // print zobrist key
    // os << "Zobrist Key: " << this->key << '\n';


    // print enpassant square
    os << "Enpassant square: " << square_to_notation(this->enpassant_square) << '\n';


    // print castling rights
    os << "Castling rights: " << castle_to_string(this->castling_rights);

    // print previous move
    if (this->ply_played > 0) {
        os << "\nPrevious Move: ";
        print_move(this->move_list[this->ply_played - 1]);
    }
}


// non-member operators

std::ostream& operator<< (std::ostream& os, const Board& board) {
    board.print(os);
    return os;
}


} // MPChess namespace
