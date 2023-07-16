// movegen.hpp

#pragma once

#include "defs.hpp"     // types, constants
#include "utils.hpp"    // pop_lsb, step

#include "board.hpp"    // board
#include "movelist.hpp" // movelist

#include "attacks.hpp"  // attacks


namespace MPChess {

template<Types::MoveGenType gen_type, Types::Color color_friend, Concepts::move_like move_t=Move>
requires (color_friend != Types::Color::NO_COLOR)
std::size_t generate_pawn_moves(const Board& board, MoveList<move_t>& move_list) {

    std::size_t initial_list_size = move_list.get_size();

    const Types::Bitboard unoccupied   = board.get_occupation_bb(Types::Color::NO_COLOR);

    const Types::Color    color_enemy  = ~color_friend;
    const Types::Bitboard enemy        =  board.get_occupation_bb(color_enemy)
                                       & ~board.get_piece_bb(color_enemy, Types::PieceType::KING);

    const Types::Bitboard second_rank  = (color_friend == Types::Color::WHITE) ? Constants::RANK_2 : Constants::RANK_7;
    const Types::Bitboard seventh_rank = (color_friend == Types::Color::WHITE) ? Constants::RANK_7 : Constants::RANK_2;

    const Types::Square   enpassant_sq = board.get_enpassant_square();

    const Types::StepType forward      = (color_friend == Types::Color::WHITE) ? Types::StepType::N : Types::StepType::S; 

    
    // capture moves: diagonal captures, enpassants, and promote captures
    if constexpr (gen_type != Types::MoveGenType::QUIET) {

        // get pawns
        Types::Bitboard pawns = board.get_piece_bb(color_friend, Types::PieceType::PAWN);

        // loop over pawns
        while (pawns) {

            // get square of current pawn
            const Types::Square from = pop_lsb(pawns);

            // get all pawn captures
            Types::Bitboard captures = Attacks::attacks<Types::PieceType::PAWN, color_friend>(from)
                                     & (enemy | enpassant_sq);

            // loop over captures
            while (captures) {

                const Types::Square to = pop_lsb(captures);

                // add enpassant capture
                if (to == enpassant_sq) {
                    move_list.add_move(move_t(from, to, Constants::Move::Flags::ENPASSANT));
                }

                // add promote capture
                else if (from & seventh_rank) {
                    for (const Types::MoveFlag promote_flag : {Constants::Move::Flags::PROMOTE_KNIGHT_CAPTURE,
                                                               Constants::Move::Flags::PROMOTE_BISHOP_CAPTURE,
                                                               Constants::Move::Flags::PROMOTE_ROOK_CAPTURE,
                                                               Constants::Move::Flags::PROMOTE_QUEEN_CAPTURE})
                    {
                        move_list.add_move(move_t(from, to, promote_flag));
                    }
                }

                // add regular capture
                else {
                    move_list.add_move(move_t(from, to, Constants::Move::Flags::CAPTURE));
                }
            }
        }
    }

    // quiet moves: single push, double push (no capture, no promote)
    if constexpr (gen_type != Types::MoveGenType::CAPTURE) {

        // get pawns
        Types::Bitboard pawns = board.get_piece_bb(color_friend, Types::PieceType::PAWN)
                              & ~seventh_rank;

        // loop over pawns
        while (pawns) {

            // get square of current pawn
            const Types::Square from  = pop_lsb(pawns);
            const Types::Square front = step<forward>(from);

            // add single pawn push
            if (front & unoccupied) {
                move_list.add_move(move_t(from, front, Constants::Move::Flags::QUIET));
            

                // add double pawn push
                const Types::Square front_front = step<forward>(front);
                if ((from & second_rank) && (front_front & unoccupied)) {
                    move_list.add_move(move_t(from, front_front, Constants::Move::Flags::DOUBLE_PAWN_PUSH));
                }
            }
        }
    }

    // non-quiet and non-capture moves: promote pawn push
    if constexpr (gen_type != Types::MoveGenType::QUIET &&
                  gen_type != Types::MoveGenType::CAPTURE)
    {

        // get pawns
        Types::Bitboard seventh_rank_pawns = board.get_piece_bb(color_friend, Types::PieceType::PAWN)
                                           & seventh_rank;

        // loop over pawns
        while (seventh_rank_pawns) {

            // get square of current pawn
            const Types::Square from = pop_lsb(seventh_rank_pawns);
            const Types::Square to   = step<forward>(from);

            // add promote push
            if (to & unoccupied) {
                for (const Types::MoveFlag promote_flag : {Constants::Move::Flags::PROMOTE_KNIGHT_QUIET,
                                                           Constants::Move::Flags::PROMOTE_BISHOP_QUIET,
                                                           Constants::Move::Flags::PROMOTE_ROOK_QUIET,
                                                           Constants::Move::Flags::PROMOTE_QUEEN_QUIET})
                {
                    move_list.add_move(move_t(from, to, promote_flag));
                }
            }
        }
    }

    const std::size_t num_pseudo_legal_generated = move_list.get_size() - initial_list_size;
    return num_pseudo_legal_generated;
}


template<Types::MoveGenType gen_type, Types::Color color_friend, Concepts::move_like move_t=Move>
requires (color_friend != Types::Color::NO_COLOR)
std::size_t generate_king_moves(const Board& board, MoveList<move_t>& move_list) {

    std::size_t initial_list_size = move_list.get_size();

    const Types::Bitboard unoccupied  = board.get_occupation_bb(Types::Color::NO_COLOR);
    const Types::Bitboard occupied    = ~unoccupied;

    const Types::Square   from        = bitboard_to_square(board.get_piece_bb(color_friend, Types::PieceType::KING));

    const Types::Bitboard king_moves  = Attacks::attacks<Types::PieceType::KING>(from);

    const Types::Color    color_enemy = ~color_friend;
    const Types::Bitboard enemy       =  board.get_occupation_bb(color_enemy)
                                      & ~board.get_piece_bb(color_enemy, Types::PieceType::KING);

    const Types::Castle color_castle_mask = (color_friend == Types::Color::WHITE) ? Constants::CastlingRights::W_BOTH : Constants::CastlingRights::B_BOTH;


    // capture moves
    if constexpr (gen_type != Types::MoveGenType::QUIET) {

        // get captures
        Types::Bitboard captures = king_moves & enemy;

        // loop over all captures
        while (captures) {

            // add capture
            const Types::Square to = pop_lsb(captures);
            move_list.add_move(move_t(from, to, Constants::Move::Flags::CAPTURE));
        }
    }

    // quiet moves (step/non-castle)
    if constexpr (gen_type != Types::MoveGenType::CAPTURE) {
        
        // get quiet steps
        Types::Bitboard quiet_steps = king_moves & unoccupied;

        // loop over all steps
        while (quiet_steps) {

            // add quiet step
            const Types::Square to = pop_lsb(quiet_steps);
            move_list.add_move(move_t(from, to, Constants::Move::Flags::QUIET));
        }
    }

    // castle moves
    if constexpr (gen_type != Types::MoveGenType::CAPTURE) {

        // check for existing castle rights and check
        const bool check = board.is_check<true>();
        const Types::Castle castle_rights = static_cast<Types::Castle>(board.get_castling_rights() & color_castle_mask);

        if (!check && castle_rights) {
            
            // loop over castle types
            for (Types::Castle castle_type : {Constants::CastlingRights::W_SHORT_B_SHORT, Constants::CastlingRights::W_LONG_B_LONG}) {

                const Types::MoveFlag castle_flag = (castle_type == Constants::CastlingRights::W_SHORT_B_SHORT) ? Constants::Move::Flags::SHORT_CASTLE
                                                                                                                : Constants::Move::Flags::LONG_CASTLE;
                castle_type &= castle_rights;

                // check if castle rights exist for current type
                if (castle_type) {

                    const auto [king_from, king_to] = castle_king_from_to(castle_type);
                    const auto [rook_from, rook_to] = castle_rook_from_to(castle_type);
                
                    const Types::Bitboard king_castle_path = Attacks::inbetween_squares(king_from, king_to);
                    const Types::Bitboard castle_squares   = Attacks::inbetween_squares(king_from, rook_from);

                    const bool safe_king_path       = is_empty(board.attacks_to(king_castle_path) & enemy);
                    const bool empty_castle_squares = is_empty(castle_squares & occupied);

                    // add castle move
                    if (safe_king_path && empty_castle_squares) {
                        move_list.add_move(move_t(king_from, king_to, castle_flag));
                    }
                }
            }
        }
    }

    const std::size_t num_pseudo_legal_generated = move_list.get_size() - initial_list_size;
    return num_pseudo_legal_generated;
}


template<Types::MoveGenType gen_type, Types::Color color_friend, Types::PieceType piece_type, Concepts::move_like move_t=Move>
requires (piece_type   != Types::PieceType::NO_PIECE_TYPE) &&
         (color_friend != Types::Color::NO_COLOR)
std::size_t generate_piece_moves(const Board& board, MoveList<move_t>& move_list) {

    std::size_t initial_list_size = move_list.get_size();

    if      constexpr (piece_type == Types::PieceType::PAWN) {
        std::size_t num_pseudo_legal_generated = generate_pawn_moves<gen_type, color_friend>(board, move_list);
        return num_pseudo_legal_generated;
    }

    else if constexpr (piece_type == Types::PieceType::KING) {
        std::size_t num_pseudo_legal_generated = generate_king_moves<gen_type, color_friend>(board, move_list);
        return num_pseudo_legal_generated;
    }

    else {

        const Types::Bitboard unoccupied = board.get_occupation_bb(Types::Color::NO_COLOR);
        const Types::Bitboard occupied   = ~unoccupied;

        const Types::Color color_enemy = ~color_friend;
        const Types::Bitboard enemy    =  board.get_occupation_bb(color_enemy)
                                       & ~board.get_piece_bb(color_enemy, Types::PieceType::KING);


        // get pieces
        Types::Bitboard pieces = board.get_piece_bb(color_friend, piece_type);

        // loop over pieces
        while (pieces) {

            // get square of current piece
            const Types::Square from = pop_lsb(pieces);

            // get all possible to squares of current piece
            const Types::Bitboard all_to_sqs = Attacks::attacks<piece_type>(from, occupied);


            // capture moves
            if constexpr (gen_type != Types::MoveGenType::QUIET) {
                
                // add captures
                Types::Bitboard capture_to_sqs = all_to_sqs & enemy;
                while (capture_to_sqs) {
                    
                    const Types::Square to = pop_lsb(capture_to_sqs);
                    move_list.add_move(move_t(from, to, Constants::Move::Flags::CAPTURE));
                }
            }

            // quiet moves
            if constexpr (gen_type != Types::MoveGenType::CAPTURE) {

                // add quiet moves
                Types::Bitboard quiet_to_sqs = all_to_sqs & unoccupied;
                while (quiet_to_sqs) {

                    const Types::Square to = pop_lsb(quiet_to_sqs);
                    move_list.add_move(move_t(from, to, Constants::Move::Flags::QUIET));
                }
            }
        }

        std::size_t num_pseudo_legal_generated = move_list.get_size() - initial_list_size;
        return num_pseudo_legal_generated;
    }
}


template<Types::MoveGenType gen_type, Types::Color color_friend, Concepts::move_like move_t>
requires (color_friend != Types::Color::NO_COLOR)
std::size_t generate_moves(const Board& board, MoveList<move_t>& move_list) {

    std::size_t move_count = 0;

    move_count += generate_pawn_moves<gen_type, color_friend>(board, move_list);
    move_count += generate_king_moves<gen_type, color_friend>(board, move_list);
    move_count += generate_piece_moves<gen_type, color_friend, Types::PieceType::KNIGHT>(board, move_list);
    move_count += generate_piece_moves<gen_type, color_friend, Types::PieceType::BISHOP>(board, move_list);
    move_count += generate_piece_moves<gen_type, color_friend, Types::PieceType::ROOK>(board, move_list);
    move_count += generate_piece_moves<gen_type, color_friend, Types::PieceType::QUEEN>(board, move_list);

    return move_count;
}


template<Types::MoveGenType gen_type, Concepts::move_like move_t>
std::size_t generate_moves(const Board& board, MoveList<move_t>& move_list) {
    if (board.get_side_to_move() == Types::Color::WHITE) {
        return generate_moves<gen_type, Types::Color::WHITE>(board, move_list);
    }
    else if (board.get_side_to_move() == Types::Color::BLACK) {
        return generate_moves<gen_type, Types::Color::BLACK>(board, move_list);
    }
    else {
        return 0;
    }
}

} // MPChess namespace