// main.cpp

#include <iostream>

#include "defs.hpp"
#include "utils.hpp"

#include "attacks.hpp" // generate tables
#include "board.hpp"

#include "perft.hpp"
#include "movegen.hpp"

using namespace MPChess;
using namespace MPChess::Types;


auto main() -> int {

    // Board board("rnbqkb1r/pppppppp/5n2/3P4/8/8/PPP1PPPP/RNBQKBNR b KQkq - 0 2");
    Board board("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    // Board board("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    // Board board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    // Board board;

    std::cout << board << "\n\n";

    PerftInfo info;
    auto nodes = perft(4, board, &info);

    std::cout << "Nodes: "
              << nodes
              << "\n";

    std::cout << "Captures: "
              << info.captures
              << "\n"
              << "Enpassants: "
              << info.enpassants
              << "\n"
              << "Castles: "
              << info.castles
              << "\n"
              << "Checks: "
              << info.checks
              << "\n"
              << "Promotes: "
              << info.promotions
              << "\n\n";

    RegularMoveList ml;

    auto pawn_count = generate_piece_moves<MoveGenType::PSEUDOLEGAL, WHITE, PAWN>(board, ml);
    ml.reset();

    auto knight_count = generate_piece_moves<MoveGenType::PSEUDOLEGAL, WHITE, KNIGHT>(board, ml);
    ml.reset();

    auto bishop_count = generate_piece_moves<MoveGenType::PSEUDOLEGAL, WHITE, BISHOP>(board, ml);
    ml.reset();

    auto rook_count = generate_piece_moves<MoveGenType::PSEUDOLEGAL, WHITE, ROOK>(board, ml);
    ml.reset();

    auto queen_count = generate_piece_moves<MoveGenType::PSEUDOLEGAL, WHITE, QUEEN>(board, ml);
    ml.reset();

    auto king_count = generate_piece_moves<MoveGenType::PSEUDOLEGAL, WHITE, KING>(board, ml);
    ml.reset();

    std::cout << "PAWN: "   << pawn_count   << "\n"
              << "KNIGHT: " << knight_count << "\n"
              << "BISHOP: " << bishop_count << "\n"
              << "ROOK: "   << rook_count   << "\n"
              << "QUEEN: "  << queen_count  << "\n"
              << "KING: "   << king_count   << "\n";

    return 0;
}