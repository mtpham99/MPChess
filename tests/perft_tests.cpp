// perft_tests.cpp

#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>

#include "perft.hpp"
#include "board.hpp"

using namespace MPChess;
using namespace MPChess::Types;


TEST_CASE("FEN #1 \"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\" @ Depth=6", "[depth6]")
{
    uint depth = 6;
    PerftInfo perft_info;
    Board board;

    auto node_count = perft(depth, board, &perft_info);

    REQUIRE(node_count                == 119060324ull);
    CHECK(perft_info.captures         == 2812008ull);
    CHECK(perft_info.enpassants       == 5248ull);
    CHECK(perft_info.checks           == 809099ull);
    CHECK(perft_info.castles          == 0ull);
    CHECK(perft_info.promotions       == 0ull);
}

TEST_CASE("FEN #2 \"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -\" @ Depth=5", "[depth5]")
{
    uint depth = 5;
    PerftInfo perft_info;
    Board board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");

    auto node_count = perft(depth, board, &perft_info);

    REQUIRE(node_count                == 193690690ull);
    CHECK(perft_info.captures         == 35043416ull);
    CHECK(perft_info.enpassants       == 73365ull);
    CHECK(perft_info.checks           == 3309887ull);
    CHECK(perft_info.castles          == 4993637ull);
    CHECK(perft_info.promotions       == 8392ull);
}

TEST_CASE("FEN #3 \"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -\" @ Depth=7", "[depth7]")
{
    uint depth = 7;
    PerftInfo perft_info;
    Board board("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -");

    auto node_count = perft(depth, board, &perft_info);

    REQUIRE(node_count                == 178633661ull);
    CHECK(perft_info.captures         == 14519036ull);
    CHECK(perft_info.enpassants       == 294874ull);
    CHECK(perft_info.checks           == 12797406ull);
    CHECK(perft_info.castles          == 0ull);
    CHECK(perft_info.promotions       == 140024ull);
}

TEST_CASE("FEN #4 \"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1\" @ Depth=5", "[depth5]")
{
    uint depth = 5;
    PerftInfo perft_info;
    Board board("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");

    auto node_count = perft(depth, board, &perft_info);

    REQUIRE(node_count                == 15833292ull);
    CHECK(perft_info.captures         == 2046173ull);
    CHECK(perft_info.enpassants       == 6512ull);
    CHECK(perft_info.checks           == 200568ull);
    CHECK(perft_info.castles          == 0ull);
    CHECK(perft_info.promotions       == 329464ull);
}

TEST_CASE("FEN #5 \"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8\" @ Depth=5", "[depth5]")
{
    uint depth = 5;
    Board board("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");

    auto node_count = perft(depth, board);

    REQUIRE(node_count == 89941194ull);
}

TEST_CASE("FEN #6 \"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10\" @ Depth=5", "[depth5]")
{
    uint depth = 5;
    Board board("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10");

    auto node_count = perft(depth, board);

    REQUIRE(node_count == 164075551ull);
}