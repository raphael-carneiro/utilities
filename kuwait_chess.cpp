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
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char final_chessboard[64];
int  global_variants_analyzed = 0;
int  global_successful_results = 0;
int  max_full_move_count = 34;
bool verbose = true;


int
error_message(int error_number, char const *error_text)
{
	if (verbose)
		fprintf(stderr, "Error #%d: %s\n", error_number, error_text);

	return error_number;
}


int
piece_count_validation(char *chessboard)
{
	int P = 0, N = 0, B_w = 0, B_b = 0, R = 0, Q = 0, K = 0;
	int p = 0, n = 0, b_w = 0, b_b = 0, r = 0, q = 0, k = 0;
	int promoted_w = 0, promoted_b = 0;
	int result = 0;

	for (int square = 0; square < 64; square++)
	{
		switch(chessboard[square])
		{
			case WHITE_PAWN:	P++;
				break;
			case BLACK_PAWN:	p++;
				break;
			case WHITE_KNIGHT:	N++;
				break;
			case BLACK_KNIGHT:	n++;
				break;
			case WHITE_BISHOP:	B_w += IS_WHITE_SQUARE(square);
								B_b += IS_BLACK_SQUARE(square);
				break;
			case BLACK_BISHOP:	b_w += IS_WHITE_SQUARE(square);
								b_b += IS_BLACK_SQUARE(square);
				break;
			case WHITE_ROOK:	R++;
				break;
			case BLACK_ROOK:	r++;
				break;
			case WHITE_QUEEN:	Q++;
				break;
			case BLACK_QUEEN:	q++;
				break;
			case WHITE_KING:	K++;
				break;
			case BLACK_KING:	k++;
				break;
		}
	}

	if (K != 1 || k != 1)
		result |= 1 << error_message(1, "Invalid number of kings");

	if (Q > 9 || q > 9)
		result |= 1 << error_message(2, "Invalid number of queens");

	if (R > 10 || r > 10)
		result |= 1 << error_message(3, "Invalid number of rooks");

	if (B_w > 5 || B_b > 5 || b_w > 5 || b_b > 5)
		result |= 1 << error_message(4, "Invalid number of bishops");

	if (N > 10 || n > 10)
		result |= 1 << error_message(5, "Invalid number of knights");

	if (P > 8 || p > 8)
		result |= 1 << error_message(6, "Invalid number of pawns");

	promoted_w = (Q > 1) * (Q - 1) + (R > 2) * (R - 2) + (B_w > 1) * (B_w - 1) + (B_b > 1) * (B_b - 1) + (N > 2) * (N - 2);
	promoted_b = (q > 1) * (q - 1) + (r > 2) * (r - 2) + (b_w > 1) * (b_w - 1) + (b_b > 1) * (b_b - 1) + (n > 2) * (n - 2);

	if ((P + promoted_w) > 8 || (p + promoted_b) > 8)
		result |= 1 << error_message(7, "Invalid number of promoted pieces");

	return result;
}


int
error_fen(int error_number, char const *error_text, char const *fen_text, char const *fen_char)
{
	if (verbose)
	{
		for (char const *c = fen_text; c != fen_char; c++)
			fprintf(stderr, "%c", *c);
		fprintf(stderr, "\e[91m%c\e[0m%s\n", *fen_char, (fen_char + 1));
	}

	return error_message(error_number, error_text);
}


int
fen_piece_placement(char *chessboard, char const *fen_text)
{
	// https://www.chessprogramming.org/Forsyth-Edwards_Notation
	int square = 0, file_count = 0;
	char working_chessboard[64];
	memset(working_chessboard, EMPTY, 64);

	for (char const *c = fen_text; *c != 0 && *c != ' '; c++)
	{
		if (square >= 64)
			return error_fen(2, "FEN text too long", fen_text, c);

		if (*c >= '1' && *c <= '8')
		{
			if (file_count > 0 && isdigit(*(c - 1)))
				return error_fen(1, "Invalid FEN syntax", fen_text, c);

			square += (*c - '0');
			file_count += (*c - '0');
			if (file_count > 8)
				return error_fen(4, "Too many files in a rank", fen_text, c);
		}
		else if (IS_PIECE(*c))
		{
			working_chessboard[square] = *c;
			square++;
			file_count++;
			if (file_count > 8)
				return error_fen(4, "Too many files in a rank", fen_text, c);
		}
		else if (*c == RANK_SEPARATOR)
		{
			if (file_count != 8)
				return error_fen(5, "Too few files in a rank", fen_text, c);

			file_count = 0;
		}
		else
			return error_fen(1, "Invalid FEN syntax", fen_text, c);
	}

	if (square != 64)
		return error_fen(3, "FEN text too short", fen_text, (fen_text + strlen(fen_text)));

	if (piece_count_validation(working_chessboard))
		return error_fen(6, "Invalid number of pieces", fen_text, (fen_text + strlen(fen_text)));

	memcpy(chessboard, working_chessboard, 64);
	return 0;
}


void
print_chessboard(char *chessboard)
{
	for (int i = 0; i < 8; i++)
	{
		printf("%d ", (8 - i));
		for (int j =0; j < 8; j++)
		{
			int square = i * 8 + j;
			char const *fg_color = IS_WHITE(chessboard[square]) ? "\e[39m" : "\e[39m";
			char const *bg_color = IS_WHITE_SQUARE(square) ? "\e[7m" : "\e[49m";
			printf("%s%s%c \e[0m", bg_color, fg_color, chessboard[square]);
		}
		printf("\n");
	}
	printf("  a b c d e f g h\n");
}


int
get_legal_moves(piece_move *legal_moves, char *chessboard, char piece, int square)
{
	int count = 0;

	return count;
}


void
update_chessboard(char *chessboard, piece_move *move)
{

}


void
update_state(game_state *current)
{

}


bool
king_in_check(int color, char *chessboard)
{
	return false;
}


bool
goal_achieved(char *chessboard, char *goal)
{
	if (strncmp(chessboard, goal, 64) == 0)
	{
		global_variants_analyzed++;
		global_successful_results++;
//		save(move_count / 2, move_list);

		return true;
	}

	return false;
}


void
make_move(game_state *previous)
{
	piece_move current_move, legal_moves[27];
	game_state current = *previous;
	current.last_move = &current_move;
	current.side_to_move = previous->side_to_move == WHITE ? BLACK : WHITE;
	current.full_move_counter += previous->side_to_move == WHITE ? 0 : 1;
	current.previous = previous;

	int color = previous->side_to_move;
	for (int square = 0; square < 64; square++)
	{
		char piece = previous->chessboard[square];
		if ((color == WHITE && !IS_WHITE(piece)) || (color == BLACK && !IS_BLACK(piece)))
			continue;

		// Rule #1 for the Kuwaiti problem: K, Ra1 and Pa never move
		if ((piece == WHITE_KING) || (piece == WHITE_ROOK && square == SQUARE('a','1')) || (piece == WHITE_PAWN && FILE(square) == 'a'))
			continue;

		int legal_move_count = get_legal_moves(legal_moves, previous->chessboard, piece, square);
		for (int i = 0; i < legal_move_count; i++)
		{
			current_move = legal_moves[i];

			// Rule #2 for the Kuwaiti problem: Pb...Pf can only capture pieces to the left side
			if (current_move.moving_piece == WHITE_PAWN && FILE(current_move.from_square) <= 'f' &&
				FILE(current_move.from_square) <= FILE(current_move.to_square))
				continue;

			// Rule #3 for the Kuwaiti problem: Every other white piece but Pb..Pf cannot capture anything
			if (color == WHITE && !(current_move.moving_piece == WHITE_PAWN && FILE(current_move.from_square) <= 'f') &&
				current_move.capture)
				continue;

			// Illegal move: own king in check
			update_chessboard(current.chessboard, &current_move);
			if (king_in_check(color, current.chessboard))
				continue;

			update_state(&current);
			if (color == BLACK && goal_achieved(current.chessboard, final_chessboard))
				return;


			make_move(&current);
		}
	}
}


int
main(int argc, char **argv)
{
	game_state initial;
	fen_piece_placement(initial.chessboard, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	fen_piece_placement(final_chessboard,   "k7/P7/P7/P7/P7/P7/P7/R3K3");
	initial.last_move = NULL;
	initial.side_to_move = WHITE;
	initial.white_castling_short_ability = true;
	initial.white_castling_long_ability  = true;
	initial.black_castling_short_ability = true;
	initial.black_castling_long_ability  = true;
	initial.en_passant_target_square = NO_SQUARE;
	initial.half_move_clock = 0;
	initial.full_move_counter = 1;
	initial.previous = NULL;

	make_move(&initial);



//	char fen_text[2000];
//	while (true)
//	{
//		printf("Insert FEN: ");
//		fgets(fen_text, 2000, stdin);
//		if (fen_text[strlen(fen_text)-1]=='\n') fen_text[strlen(fen_text)-1]=0;
//		if (!fen_piece_placement(final_chessboard, fen_text))
//		{
//			printf("No errors!\n");
//			print_chessboard(final_chessboard);
//		}
//		printf("Insert FEN: ");
//	}

	return 0;
}
