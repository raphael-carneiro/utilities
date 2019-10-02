/*
 * kuwait_chess.cpp
 *
 *  Created on: 02-Oct-2019
 *      Author: Raphael V. Carneiro
 *
 *  This program is related to the challenge made by a Kuwaiti chess player:
 *      How to get chessboard position k7/P7/P7/P7/P7/P7/P7/R3K3 after black move, with the least number of legal moves?
 *
 *  Known answer: 34 moves
 *      1. g4 e5 2. Nh3 Ba3 3. bxa3 h5 4. Bb2 hxg4 5. Bc3 Rh4 6. Bd4 exd4 7. Nc3 dxc3 8. dxc3 g3 9. Qd3 Rb4 10. Nf4 g5
 *      11. h4 f5 12. h5 d5 13. h6 Bd7 14. h7 g2 15. h8=B g1=R 16. Bd4 Ba4 17. Rh4 Rg3 18. Bg2 gxf4 19. Be3 fxe3 20. Be4 fxe4
 *      21. fxe3 exd3 22. exd3 c5 23. Rc4 dxc4 24. dxc4 b5 25. cxb4 Qa5 26. cxb5 Na6 27. bxa5 O-O-O 28. bxa6 Rd4 29. exd4 Rb3
 *      30. cxb3 Ne7 31. bxa4 Nd5 32. dxc5 Nb6 33. cxb6 Kb8 34. bxa7 Ka8
 *
 *  Reference: YouTube Channel Xadrez Brasil https://www.youtube.com/watch?v=5W-w31_95As
 */

#include "kuwait_chess.hpp"
#include <stdio.h>
#include <ctype.h>

char initial_chessboard[64];
char target_chessboard[64];


bool white_castling_short_able = false; // K and Rh have never moved and Rh was not captured
bool white_castling_long_able = false;  // K and Ra have never moved and Ra was not captured
bool black_castling_short_able = false;
bool black_castling_long_able = false;

int en_passant_target_square = -1;
int last_move_with_capture_or_pawn = 0;

int
main(int argc, char **argv)
{

	return 0;
}
