# MPChess
Making my first UCI (universal chess interface) chess engine written in C++, as I read through [The Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)

# Build Dependencies
- (Optional/Testing) Catch2 v2.x

# Build
Build from source using cmake:
```
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=yes ..
make -j$(nproc)
```
Built binaries will be in the "bin" directory.

# Run Perft Tests
Perft tests are designed around the 6 positions from [The Chess Programming Wiki: Perft results](https://www.chessprogramming.org/Perft_Results).

Tests were made using [Catch2 (v2.x)](https://github.com/catchorg/Catch2/tree/v2.x) and integrated using cmake.

To run the tests, make sure MPChess was built with the "BUILD_TESTING" set. The run the following:
```
cd build
ctest -j6 # Run all 6 tests in parallel
```

# How to Use
Below is a link to the UCI (Universal Chess Interface):

[UCI Specification](https://www.wbec-ridderkerk.nl/html/UCIProtocol.html)

## Features
- [Bitboards](https://www.chessprogramming.org/Bitboards)
- [Magic Bitboards](https://www.chessprogramming.org/Magic_Bitboards)
- [Quiescence Search](https://www.chessprogramming.org/Quiescence_Search)
- [Transposition Table](https://www.chessprogramming.org/Transposition_Table)
- [Killer Heuristic](https://www.chessprogramming.org/Killer_Heuristic)
- [History Heuristic](https://www.chessprogramming.org/History_Heuristic)
- [Null Move Pruning](https://www.chessprogramming.org/Null_Move_Pruning)
- [Aspiration Windows](https://www.chessprogramming.org/Aspiration_Windows)
- [Simple Late Move Reductions](https://www.chessprogramming.org/Late_Move_Reductions)
- [Simple Move Extensions](https://www.chessprogramming.org/Extensions)
    - Checks

## TODO List
- Move Time
- Multi-Threaded Search/Multi PV
- Opening Book
- Tablebases
- Mate Search
- [Lichess Bot Client](https://github.com/lichess-bot-devs/lichess-bot)


## Resources
1. [__***The Resource***__ ](https://www.chessprogramming.org/Main_Page)
2. [Good explanation on Magic Bitboard](https://analog-hors.github.io/site/magic-bitboards/)
3. [Late Move Reductions](https://web.archive.org/web/20070820072632/http://www.glaurungchess.com/lmr.html)
4. [Perft Results for Checking](https://www.chessprogramming.org/Perft_Results)
5. [QPerft](https://home.hccnet.nl/h.g.muller/dwnldpage.html)
    - Very fast perft command line tool for checking your implementation
    - Also has a branch count feature to help trackdown perft/movegen errors
6. Other Engines I Used as Guides:
    1. [List of Chess Engines](https://computerchess.org.uk/ccrl/4040/index.html)
    2. [Goldfish](https://github.com/bsamseth/Goldfish)
    3. [Renegade](https://github.com/pkrisz99/Renegade)
    4. [Stockfish](https://github.com/official-stockfish/Stockfish)
7. Playlists:
    1. [Chess Programming's Bitboard Chess Engine in C](https://www.youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs)
    2. [Bluefever Software's Chess Engine in C](https://www.youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg)
