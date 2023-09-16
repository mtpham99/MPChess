// uci.cpp

#include "uci.hpp"

#include "defs.hpp"     // types, constants
#include "utils.hpp"    // square_to_notation, current_time

#include "board.hpp"    // board
#include "move.hpp"     // move
#include "movelist.hpp" // movelist

#include "movegen.hpp"  // movegen
#include "engine.hpp"   // engine globals (searchinfo)

#include <string>       // string
#include <sstream>      // stringstream

using namespace MPChess::Types;
using namespace MPChess::Constants;


namespace MPChess {

namespace UCI {

// utils

std::string move_to_uci_notation(Move move) {
    if (move.is_null()) {return "0000";}

    const Square    from    = move.get_from_square();
    const Square    to      = move.get_to_square();
    const PieceType promote = move.get_promote_piece_type();

    std::string move_str = square_to_notation(from) + square_to_notation(to);

    if (move.is_promote()) {
        move_str += PIECE_TYPE_LABELS[promote];
    }

    return move_str;
}

Move uci_notation_to_move(std::string_view notation, const Board& board) {
    if (notation.size() < 2 || notation.size() > 5 || notation == "0000") {
        return {}; // null move
    }

    RegularMoveList pseudo_legal_moves;
    generate_moves<MoveGenType::PSEUDOLEGAL>(board, pseudo_legal_moves);

    for (const Move& move : pseudo_legal_moves) {
        if (move_to_uci_notation(move) == notation) {
            return move;
        }
    }

    return {}; // null move
}


// uci specification

void print_welcome() {
    sync_out << "Welcome to MPChess!\n\n";
    sync_out.emit();
}

void uci_loop() {

    std::string line, chunk;
    while(std::getline(std::cin, line)) {

        std::istringstream stream(line);
        chunk.clear();
        stream >> chunk;

        if (chunk == "uci") {
            sync_out << "id name MPChess\n"
                     << "id author Matthew Pham\n"
                     << "uciok\n\n";
            sync_out.emit();
        }

        else if (chunk == "isready") {
            sync_out << "readyok\n\n";
            sync_out.emit();
        }

        else if (chunk == "setoption") {
            // TODO
        }

        else if (chunk == "debug") {
            stream >> chunk;
            if (chunk == "y"   ||
                chunk == "yes" ||
                chunk == "on")
            {
                Engine::options.debug = true;
            }
            else {
                Engine::options.debug = false;
            }
        }

        else if (chunk == "position") {
            parse_position(stream);
        }

        else if (chunk == "go") {
            parse_go(stream);
        }

        else if (chunk == "stop") {
            Engine::thread_pool.stop_search();
        }

        else if (chunk == "ucinewgame") {
            Engine::thread_pool.stop_search();
            Engine::tt.reset();
        }

        else if (chunk == "isready") {
            sync_out << "readyok\n";
            sync_out.emit();
        }

        else if (chunk == "print" ||
                 chunk == "d")
        {
            sync_out << Engine::engine_board << "\n\n";
            sync_out.emit();
        }

        else if (chunk == "quit" ||
                 chunk == "q"    ||
                 chunk == "exit")
        {
            sync_out << "Quitting. Good Bye.\n\n";
            sync_out.emit();

            Engine::thread_pool.stop_search();

            break;
        }
    }
}

void parse_position(std::istringstream& stream) {

    Board parse_board;
    std::string chunk;
    stream >> chunk;

    // parse fen
    if (chunk == "startpos") {
        parse_board.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        stream >> chunk;
    }
    else if (chunk == "fen") {
        std::string fen;
        while (stream >> chunk && chunk != "moves") {
            fen += chunk + " ";
        }
        fen.pop_back();

        parse_board.set_fen(std::move(fen));
    }
    else {
        return;
    }

    // parse moves
    if (chunk == "moves") {
        Move move;
        while (stream >> chunk) {
            move = uci_notation_to_move(chunk, parse_board);
            if (move.is_null()) {break;}
            parse_board.make_move(move);
        }
    }

    std::string parsed_fen = parse_board.get_fen();
    Engine::engine_board.set_fen(std::move(parsed_fen));
}

void parse_go(std::istringstream& stream) {

    SearchInfo parse_search_info;

    // start time
    parse_search_info.start_time = current_time();

    std::string chunk;
    while (stream >> chunk) {

        // root moves
        if (chunk == "searchmoves") {
            while (stream >> chunk) {
                const Move root_move = uci_notation_to_move(chunk, Engine::engine_board);
                parse_search_info.root_moves.add_move(root_move);
            }
        }

        // ponder
        else if (chunk == "ponder") {
            // TODO
            parse_search_info.ponder = true;
        }

        // white time
        else if (chunk == "wtime") {
            stream >> chunk;
            parse_search_info.white_time = Milliseconds{std::stoull(chunk)};
        }

        // black time
        else if (chunk == "btime") {
            stream >> chunk;
            parse_search_info.black_time = Milliseconds{std::stoull(chunk)};
        }

        // white increment
        else if (chunk == "winc") {
            stream >> chunk;
            parse_search_info.white_inc = Milliseconds{std::stoull(chunk)};
        }

        // black increment
        else if (chunk == "binc") {
            stream >> chunk;
            parse_search_info.black_inc = Milliseconds{std::stoull(chunk)};
        }

        // moves until time control
        else if (chunk == "movestogo") {
            stream >> chunk;
            parse_search_info.moves_to_go = std::stoul(chunk);
        }

        // search until explicit "stop"
        else if (chunk == "infinite") {
            parse_search_info.infinite = true;
        }

        // search specific depth
        else if (chunk == "depth") {
            stream >> chunk;
            parse_search_info.max_depth = std::stoul(chunk);
        }

        // search specific number of nodes
        // DEBUG NOT WORKING
        else if (chunk == "nodes") {
            stream >> chunk;
            parse_search_info.max_nodes = std::stoul(chunk);
        }

        // search for mate in n
        else if (chunk == "mate") {
            stream >> chunk;
            // TODO
            parse_search_info.mate_in_n = std::stoul(chunk);
        }
        
        // search for specific amount of time (ms)
        else if (chunk == "movetime") {
            stream >> chunk;
            parse_search_info.max_time = Milliseconds{std::stoull(chunk)};
        }
    }

    Engine::thread_pool.start_search(std::move(parse_search_info));
}

} // UCI namesapce

} // MPChess namespace