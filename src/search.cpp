// search.cpp

#include "search.hpp"

#include "defs.hpp"        // types, constants
#include "board.hpp"       // board
#include "movelist.hpp"    // movelist

#include "threads.hpp"     // thread
#include "engine.hpp"      // engine variables (tt, searchinfo)

#include "evaluation.hpp"  // evaluate
#include "movepicker.hpp"  // movepicker

#include "uci.hpp"         // uci

#include <cmath>           // pow

using namespace MPChess::Types;
using namespace MPChess::Constants;


namespace MPChess {

Eval quiescence(EngineThread& thread,
                Eval          alpha,
                Eval          beta)
{
    if (thread.status == EngineThreadStatus::IDLE) {return 0;}

    Board& board = thread.root_board;    

    Eval stand_pat = evaluate(board);
    if (stand_pat >= beta)  {return beta;}
    if (stand_pat >  alpha) {alpha = stand_pat;}

    // RegularMoveList pseudo_legal_captures;
    // generate_moves<MoveGenType::CAPTURE>(board, pseudo_legal_captures);
    MovePicker<MoveGenType::CAPTURE> move_picker(board);
    Move capture;
    while (!(capture = move_picker.next_move()).is_null()) {

        // make move
        board.make_move(capture);

        // if illegal
        if (board.is_check<false>()) {
            board.unmake_move();
            continue;
        }

        const Eval score = -quiescence(thread, -beta, -alpha);
        ++(Engine::search_info.depth_node_count); // total nodes for current iterative deepening iteration
        ++(thread.node_counter);                  // total nodes for current search
        board.unmake_move();

        if (score >= beta)  {
            return beta;
        }
        
        else if (score >  alpha) {
            alpha = score;
        }
    }

    return alpha;
}

Eval alpha_beta(EngineThread&    thread,
                Depth            depth,
                Eval             alpha,
                Eval             beta,
                bool             root,
                RegularMoveList& pv_parent)
{
    Board&              board = thread.root_board;
    TranspositionTable& tt    = Engine::tt;

    // check max_ply or repetition
    if (board.get_ply_played() >= MAX_PLY)                    {return evaluate(board);}
    if (board.is_repetition() || board.get_ply_clock() > 100) {return 0;}

    // check for stop signal
    if (thread.is_main_thread() && thread.check_stop()) {return 0;}
    if (thread.status == EngineThreadStatus::IDLE)      {return 0;}

    // probe hash entry
    const TTEntry   tt_entry     = tt.probe(board.get_zobrist_key());
    const Eval&     tt_eval      = tt_entry.eval;
    const NodeType& tt_node_type = tt_entry.node;
    if (!tt_entry.is_null() && tt_entry.depth > depth) {
        if (tt_node_type == NodeType::PV_NODE
            || (tt_node_type == NodeType::ALL_NODE && tt_eval <= alpha)
            || (tt_node_type == NodeType::CUT_NODE && tt_eval >= beta))
        {
            return tt_eval;
        }
    }

    // quiescence
    if (depth == 0) {
        return quiescence(thread, alpha, beta);
    }

    // alpha-beta
    Move            best_move;
    Eval            best_score = -Evals::INF;
    NodeType        node_type  =  NodeType::ALL_NODE;
    RegularMoveList pv_child;

    // null-move pruning
    const std::size_t R = 2; // depth reduction factor 
    if (depth >= R + 2 && !board.is_check<true>()) {

        board.make_null_move();
        Eval score = -alpha_beta(thread, depth - 1 - R, -beta, -beta + 1, false, pv_child); // TODO : no need for pv here
        board.unmake_null_move();

        if (score >= beta) {return beta;}
    }

    MovePicker<MoveGenType::PSEUDOLEGAL> move_picker(board);
    Move move;
    std::size_t legal_count = 0;
    while (!(move = move_picker.next_move()).is_null()) {

        // root moves
        if (root &&
            std::find(thread.root_moves.begin(),
                      thread.root_moves.end(),
                      move) == thread.root_moves.end())
        {
            // skip non-root move at root
            continue;
        } 

        // make move
        board.make_move(move);

        // if illegal
        if (board.is_check<false>()) {
            board.unmake_move();
            continue;
        }
        ++legal_count;
        if (root) {++(Engine::search_info.curr_move_number);}

        // uci update
        if (thread.is_main_thread()
            && depth == 1
            && (current_time() - Engine::prev_uci_update_time) > Engine::uci_update_frequency)
        {
            Engine::prev_uci_update_time        = current_time();
            const RegularMoveList& played_moves = board.get_move_list();

            UCI::sync_out << "info "
                          << "depth "          << board.get_ply_played()                     << " "
                          << "currmove "       << UCI::move_to_uci_notation(played_moves[0]) << " "
                          << "currmovenumber " << Engine::search_info.curr_move_number       << " "
                          << "currline ";
            for (const Move& move : played_moves)
            {
                UCI::sync_out << UCI::move_to_uci_notation(move) << " ";
            }
            UCI::sync_out << "\n\n";
            UCI::sync_out.emit();
        }

        const Eval score = -alpha_beta(thread, depth - 1, -beta, -alpha, false, pv_child);
        ++(Engine::search_info.depth_node_count); // total nodes for current iterative deepening iteration
        ++(thread.node_counter);                  // total nodes for current search
        board.unmake_move();

        if (score >= beta) {

            // store cutoff move in tt
            node_type = NodeType::CUT_NODE;
            tt.store(board.get_zobrist_key(), move, beta, depth, node_type);

            // killer move
            if (!move.is_capture()) {
                const auto p_kmove = std::find(Engine::killer_table[depth].begin(), Engine::killer_table[depth].end(), move);
                if (p_kmove == Engine::killer_table[depth].end()) {
                    for (std::size_t k_ind = NUM_KILLER_MOVES-1; k_ind > 0; --k_ind) {
                        Engine::killer_table[depth][k_ind] = Engine::killer_table[depth][k_ind-1];
                    }
                    Engine::killer_table[depth][0] = move;
                }
            }
            return beta;
        }
        else if (score > alpha) {
            alpha     = score;
            node_type = NodeType::PV_NODE;

            // update pv
            pv_parent.shrink(0);
            pv_parent.add_move(move);
            pv_parent.add_moves(pv_child);

            // history move
            if (!move.is_capture()) {
                Engine::history_table[board.moved_piece(move)][move.get_to_square()] += (1 << depth);
            }
        }

        if (score > best_score) {
            best_move  = move;
            best_score = score;
        }
    }

    // no legal moves (mate/stalemate)
    if (legal_count == 0) {
        
        // checkmate
        if (board.is_check<true>()) {
            return -Evals::MATE + board.get_ply_played();
        }
        // stalemate
        else {
            return 0;
        }
    }

    tt.store(board.get_zobrist_key(), best_move, best_score, depth, node_type);
    return alpha;
}

Eval search(EngineThread& thread) {


    Board&           root_board = thread.root_board;
    RegularMoveList& root_moves = thread.root_moves;
    root_board.set_fen(Engine::engine_board.get_fen());

    // iterative deepening loop
    Depth depth   = 1;
    Eval  alpha   = -Evals::INF;
    Eval  beta    =  Evals::INF;
    Eval  window  =  Constants::PAWN_SCORE / 2;
    while (Engine::thread_pool.is_running()
           && depth < MAX_PLY)
    {                                              
        // root moves
        if (Engine::search_info.root_moves.get_size() > 0) {
            root_moves = Engine::search_info.root_moves;
        }
        else {
            root_moves.shrink(0);
            generate_moves<MoveGenType::PSEUDOLEGAL>(root_board, root_moves);
        }

        // multipv loop
        RegularMoveList  temp_pv_line;
        std::size_t      num_pvs = std::min(root_moves.get_size(), Engine::options.num_pvs);
        for (std::size_t pv_ind  = 0; pv_ind < num_pvs; ++pv_ind) {

            // reset pv and search stats
            temp_pv_line.shrink(0);
            Engine::search_info.curr_move_number      = 0;
            Engine::search_info.depth_node_count_prev = Engine::search_info.depth_node_count;
            Engine::search_info.depth_node_count      = 0;
            
            Eval score = alpha_beta(thread, depth, alpha, beta, true, temp_pv_line);

            // adjust aspiration window
            if (score <= alpha || score >= beta) {
                alpha = -Evals::INF;
                beta  =  Evals::INF;

                temp_pv_line.shrink(0);
                score = alpha_beta(thread, depth, alpha, beta, true, temp_pv_line);
            }
            else {
                alpha = score - window;
                beta  = score + window;
            }

            if (Engine::thread_pool.is_running() && temp_pv_line.get_size() == depth)
            {
                Engine::pv_lines[pv_ind].set_moves(temp_pv_line);
                Engine::pv_lines[pv_ind].set_score(score); 
            }
            // ran out of time
            else {
                break;
            }

            // remove pv move from root moves and search another pv line
            root_moves.remove_move(Engine::pv_lines[pv_ind][0]);            
        } // pv loop

        // sort pvlines
        std::sort(Engine::pv_lines.begin(), Engine::pv_lines.end());

        // uci update
        if (thread.is_main_thread()) {
            const auto total_nodes      = Engine::thread_pool.sum_threads(&EngineThread::node_counter);
            const auto time_spent       = (current_time() - Engine::search_info.start_time).count();
            const auto nodes_per_second = 1000.0 * total_nodes / time_spent;

            for (std::size_t pv_ind=0; pv_ind<num_pvs; ++pv_ind) {
                UCI::sync_out << "info "
                              << "depth " << depth << " ";

                if (num_pvs > 1) {
                    UCI::sync_out << "multipv " << pv_ind << " ";
                }

                UCI::sync_out << "score cp " << Engine::pv_lines[pv_ind].get_score() << " "
                              << "nodes "    << total_nodes                          << " "
                              << "nps "      << nodes_per_second                     << " "
                              << "pv ";

                for (const Move& pv_move : Engine::pv_lines[pv_ind]) {
                    UCI::sync_out << UCI::move_to_uci_notation(pv_move) << " ";
                }
                UCI::sync_out << "\n";

                if (Engine::options.debug) {
                    UCI::sync_out << "info debug ";

                    if (depth >= 2) {
                        // mean branching factor
                        const auto mean_bf_1 = std::pow(Engine::search_info.depth_node_count, 1. / depth);
                        const auto mean_bf_2 = 1.0 * Engine::search_info.depth_node_count / Engine::search_info.depth_node_count_prev;
                        UCI::sync_out << "BF: " << mean_bf_1 << " " << mean_bf_2 << "\n";
                    }
                }

                UCI::sync_out << "\n";
                UCI::sync_out.emit();
            }
        }

        // iterative deepening depth increment
        ++depth;
    } // iterative deepening loop

    if (thread.is_main_thread()) {
        UCI::sync_out << "bestmove " << UCI::move_to_uci_notation(Engine::pv_lines[0][0]) << "\n";
        UCI::sync_out.emit();
    }
    return Engine::pv_lines[0].get_score();
}

} // MPChess namespace