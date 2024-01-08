# MPChess
My first chess engine written in C++, as I read through [The Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)

## Features
- Bitboards
- Magic Bitboards
- Simple Late Move Reductions
- Simple Move Extensions
    - Checks
- Null Move Pruning
- Killer & History Heuristic
- Transposition Table
- Aspiration Windows
- Quiescence Search

## TODO List
- Move Time
- Multi-Threaded Search & Transposition Table
- Opening Book
- Tablebases
- Multi PV
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
