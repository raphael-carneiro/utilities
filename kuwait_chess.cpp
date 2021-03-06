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
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <vector>
#include <csignal>
#include <bits/stdc++.h>

using namespace std;

bool goal_is_mate = false;
bool goal_is_draw = false;
bool goal_is_chessboard = false;
char const *initial_fen = "";
char const *final_fen = "";
game_state initial_game;
int  initial_side_to_move = WHITE;
char initial_chessboard[NUM_SQUARES];
char final_chessboard[NUM_SQUARES];
long variants_analyzed  = 0;
char last_variant_analyzed[4000] = {0};
int  min_move_count_mate = 9999;
int  min_move_count_draw = 9999;
int  min_move_count_chessboard = 9999;
int  max_full_move_count = 9999;
vector<string> mate_variant;
vector<string> draw_variant;
vector<string> chessboard_variant;
FILE *save_results = NULL;
char const *save_results_name = "kuwait_chess.txt";
int verbose = 1;

#define NORMAL				"\e[0m"
#define REVERSE				"\e[7m"
#define RESET_REVERSE		"\e[27m"
#define FG_DEFAULT			"\e[0;39m"
#define BG_DEFAULT			"\e[0;49m"
#define FG_LIGHT_RED		"\e[91m"
#define FG_LIGHT_YELLOW		"\e[93m"
#define FG_LIGHT_CYAN		"\e[96m"
#define FG_BOLD_LIGHT_RED	"\e[1;91m"
#define FG_BOLD_RED			"\e[1;31m"
#define FG_BOLD_YELLOW		"\e[1;33m"
#define FG_BOLD_CYAN		"\e[1;36m"
#define BG_BLACK			"\e[40m"
#define BG_WHITE			"\e[47m"


bool
no_restriction(char piece, int square)
{
	return false;
}

bool
no_restriction(piece_move *move)
{
	return false;
}

bool (*special_piece_restriction)(char piece, int square) = no_restriction;
bool (*special_move_restriction)(piece_move *move) = no_restriction;


int
error_message(int error_number, char const *error_text)
{
	if (verbose)
		fprintf(stderr, "Error #%d: %s\n", error_number, error_text);

	return error_number;
}


int
extra(int num_pieces, int default_num_pieces)
{
	int extra_num_pieces = (num_pieces > default_num_pieces) ? (num_pieces - default_num_pieces) : 0;

	return extra_num_pieces;
}


int
piece_count_validation(char *chessboard)
{
	int P = 0, N = 0, B = 0, R = 0, Q = 0, K = 0;
	int p = 0, n = 0, b = 0, r = 0, q = 0, k = 0;
	int promoted_P = 0, promoted_p = 0;
	int result = 0;

	for (int square = 0; square < NUM_SQUARES; square++)
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
			case WHITE_BISHOP:	B++;
				break;
			case BLACK_BISHOP:	b++;
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

	if (K != NUM_KINGS || k != NUM_KINGS)
		result |= 1 << error_message(1, "Invalid number of kings");

	if (Q > (NUM_QUEENS  + NUM_PAWNS) || q > (NUM_QUEENS  + NUM_PAWNS))
		result |= 1 << error_message(2, "Invalid number of queens");

	if (R > (NUM_ROOKS   + NUM_PAWNS) || r > (NUM_ROOKS   + NUM_PAWNS))
		result |= 1 << error_message(3, "Invalid number of rooks");

	if (B > (NUM_BISHOPS + NUM_PAWNS) || b > (NUM_BISHOPS + NUM_PAWNS))
		result |= 1 << error_message(4, "Invalid number of bishops");

	if (N > (NUM_KNIGHTS + NUM_PAWNS) || n > (NUM_KNIGHTS + NUM_PAWNS))
		result |= 1 << error_message(5, "Invalid number of knights");

	if (P > NUM_PAWNS || p > NUM_PAWNS)
		result |= 1 << error_message(6, "Invalid number of pawns");

	promoted_P = extra(Q, NUM_QUEENS) + extra(R, NUM_ROOKS) + extra(B, NUM_BISHOPS) + extra(N, NUM_KNIGHTS);
	promoted_p = extra(q, NUM_QUEENS) + extra(r, NUM_ROOKS) + extra(b, NUM_BISHOPS) + extra(n, NUM_KNIGHTS);

	if ((P + promoted_P) > NUM_PAWNS || (p + promoted_p) > NUM_PAWNS)
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
		fprintf(stderr, "%s%c%s%s\n", REVERSE, *fen_char, RESET_REVERSE, (fen_char + 1));
	}

	return error_message(error_number, error_text);
}


int
fen_piece_placement(char *chessboard, char const *fen_text)
{
	// https://www.chessprogramming.org/Forsyth-Edwards_Notation
	int square = 0, file_count = 0, rank_count = 1;
	char working_chessboard[NUM_SQUARES];
	memset(working_chessboard, EMPTY, NUM_SQUARES);

	for (char const *c = fen_text; *c != 0 && *c != ' '; c++)
	{
		if (square >= NUM_SQUARES)
			return error_fen(2, "FEN text too long", fen_text, c);

		if (*c >= '1' && *c <= (NUM_FILES + '0'))
		{
			if (file_count > 0 && isdigit(*(c - 1)))
				return error_fen(1, "Invalid FEN syntax", fen_text, c);

			square += (*c - '0');
			file_count += (*c - '0');
			if (file_count > NUM_FILES)
				return error_fen(4, "Too many files in a rank", fen_text, c);
		}
		else if (IS_PIECE(*c))
		{
			working_chessboard[square] = *c;
			square++;
			file_count++;
			if (file_count > NUM_FILES)
				return error_fen(4, "Too many files in a rank", fen_text, c);

			if (IS_PAWN(*c) && (rank_count == 1 || rank_count == NUM_RANKS))
				return error_fen(7, "Invalid pawn location", fen_text, c);
		}
		else if (*c == RANK_SEPARATOR)
		{
			if (file_count != NUM_FILES)
				return error_fen(5, "Too few files in a rank", fen_text, c);

			file_count = 0, rank_count++;
		}
		else
			return error_fen(1, "Invalid FEN syntax", fen_text, c);
	}

	if (square != NUM_SQUARES)
		return error_fen(3, "FEN text too short", fen_text, (fen_text + strlen(fen_text)));

	if (piece_count_validation(working_chessboard))
		return error_fen(6, "Invalid number of pieces", fen_text, (fen_text + strlen(fen_text)));

	memcpy(chessboard, working_chessboard, NUM_SQUARES);
	return 0;
}


void
print_chessboard(char *chessboard)
{
	printf("\n");

	for (int r = 0; r < NUM_RANKS; r++)
	{
		printf("%d ", (NUM_RANKS - r));
		for (int f = 0; f < NUM_FILES; f++)
		{
			int square = r * NUM_FILES + f;
			char const *fg_color = IS_WHITE(chessboard[square]) ? FG_BOLD_CYAN : FG_BOLD_LIGHT_RED;
			char const *bg_color = IS_WHITE_SQUARE(square) ? BG_WHITE : BG_DEFAULT;
			printf("%s%s%c %s%s", bg_color, fg_color, chessboard[square], BG_DEFAULT, FG_DEFAULT);
		}
		printf("\n");
	}

	printf("  ");
	for (int f = 0; f < NUM_FILES; f++)
		printf("%c ", 'a' + f);
	printf("\n\n");
}


void
default_move(piece_move *move, char piece, int square)
{
	move->moving_piece = piece;
	move->promoted_piece = EMPTY;
	move->from_square = square;
	move->to_square = NO_SQUARE;
	move->file_ambiguity = false;
	move->rank_ambiguity = false;
	move->capture = false;
	move->en_passant = false;
	move->check = false;
	move->mate = false;
	move->draw = false;
	move->next_valid_moves = 0;
}


void
get_moves(piece_move *moves, int *index, char *chessboard, char file, char rank, int file_step, int rank_step, int max_steps)
{
	int color = COLOR(chessboard[SQUARE(file, rank)]);
	int f = file + file_step;
	int r = rank + rank_step;

	for (int step = 0; step < max_steps; step++)
	{
		if (f < 'a' || f > ('a' + NUM_FILES - 1) || r < '1' || r > ('1' + NUM_RANKS - 1))
			break;

		int square = SQUARE(f, r);

		if (chessboard[square] != EMPTY)
		{
			if (COLOR(chessboard[square]) != color)
			{
				moves[*index].to_square = square;
				moves[*index].capture = true;
				(*index)++;
			}
			break;
		}

		moves[*index].to_square = square;
		(*index)++;
		f += file_step;
		r += rank_step;
	}
}


void
get_castling(piece_move *moves, int *index, char *chessboard, int color, char castling_side, bool castling_ability)
{
	if (!castling_ability)
		return;

	char file = 'e';
	char rank = (color == WHITE) ? '1' : ('1' + NUM_RANKS - 1);
	int first_empty_square = IS_KING(castling_side) ? SQUARE(file + 1, rank) : SQUARE(file - 3, rank);
	int last_empty_square  = IS_KING(castling_side) ? SQUARE(file + 2, rank) : SQUARE(file - 1, rank);

	for (int square = first_empty_square; square <= last_empty_square; square++)
		if (chessboard[square] != EMPTY)
			return;

	moves[*index].to_square = IS_KING(castling_side) ? SQUARE(file + 2, rank) : SQUARE(file - 2, rank);
	(*index)++;
}


void
get_move_pawn_forward(piece_move *moves, int index_base, int *index, char *chessboard, char file, char rank, int step = 1)
{
	int color = COLOR(chessboard[SQUARE(file, rank)]);
	int rank_step = (color == WHITE) ? step : -step;
	char initial_rank = (color == WHITE) ? ('1' + 1) : ('1' + NUM_RANKS - 2);
	char r = rank + rank_step;

	int square = SQUARE(file, r);
	if (chessboard[square] == EMPTY)
	{
		moves[index_base + (*index)].to_square = square;
		(*index)++;

		if (rank == initial_rank && step == 1)
			get_move_pawn_forward(moves, index_base, index, chessboard, file, rank, 2);
	}
}


void
get_move_pawn_capture(piece_move *moves, int index_base, int *index, char *chessboard, char file, char rank, int file_step, int en_passant_target_square)
{
	int color = COLOR(chessboard[SQUARE(file, rank)]);
	int rank_step = (color == WHITE) ? 1 : -1;
	char f = file + file_step;
	char r = rank + rank_step;
	if (f < 'a' || f > ('a' + NUM_FILES - 1))
		return;

	int square = SQUARE(f, r);
	if ((chessboard[square] != EMPTY && COLOR(chessboard[square]) != color) || (square == en_passant_target_square))
	{
		moves[index_base + (*index)].to_square = square;
		moves[index_base + (*index)].capture = true;
		moves[index_base + (*index)].en_passant = (square == en_passant_target_square);
		(*index)++;
	}
}


int
get_legal_moves(piece_move *legal_moves, game_state *game, char piece, int square)
{
	char file = FILE(square);
	char rank = RANK(square);
	char *cb = game->chessboard;
	int color = COLOR(cb[square]);
	bool castling_short_ability, castling_long_ability;

	for (int i = 0; i < MAX_LEGAL_MOVES; i++)
		default_move(&legal_moves[i], piece, square);

	int i = 0, j = 0;
	switch (piece)
	{
		case WHITE_KING:   case BLACK_KING:
			get_moves(legal_moves, &i, cb, file, rank,  0,  1, 1);
			get_moves(legal_moves, &i, cb, file, rank,  1,  1, 1);
			get_moves(legal_moves, &i, cb, file, rank,  1,  0, 1);
			get_moves(legal_moves, &i, cb, file, rank,  1, -1, 1);
			get_moves(legal_moves, &i, cb, file, rank,  0, -1, 1);
			get_moves(legal_moves, &i, cb, file, rank, -1, -1, 1);
			get_moves(legal_moves, &i, cb, file, rank, -1,  0, 1);
			get_moves(legal_moves, &i, cb, file, rank, -1,  1, 1);

			castling_short_ability = (color == WHITE && game->white_castling_short_ability) || (color == BLACK && game->black_castling_short_ability);
			castling_long_ability  = (color == WHITE && game->white_castling_long_ability)  || (color == BLACK && game->black_castling_long_ability);
			get_castling(legal_moves, &i, cb, color, 'K', castling_short_ability);
			get_castling(legal_moves, &i, cb, color, 'Q', castling_long_ability);
			break;

		case WHITE_QUEEN:  case BLACK_QUEEN:
			get_moves(legal_moves, &i, cb, file, rank,  0,  1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank,  1,  1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank,  1,  0, NUM_FILES - 1);
			get_moves(legal_moves, &i, cb, file, rank,  1, -1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank,  0, -1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank, -1, -1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank, -1,  0, NUM_FILES - 1);
			get_moves(legal_moves, &i, cb, file, rank, -1,  1, NUM_RANKS - 1);
			break;

		case WHITE_ROOK:   case BLACK_ROOK:
			get_moves(legal_moves, &i, cb, file, rank,  0,  1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank,  1,  0, NUM_FILES - 1);
			get_moves(legal_moves, &i, cb, file, rank,  0, -1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank, -1,  0, NUM_FILES - 1);
			break;

		case WHITE_BISHOP: case BLACK_BISHOP:
			get_moves(legal_moves, &i, cb, file, rank,  1,  1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank,  1, -1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank, -1, -1, NUM_RANKS - 1);
			get_moves(legal_moves, &i, cb, file, rank, -1,  1, NUM_RANKS - 1);
			break;

		case WHITE_KNIGHT: case BLACK_KNIGHT:
			get_moves(legal_moves, &i, cb, file, rank,  1,  2, 1);
			get_moves(legal_moves, &i, cb, file, rank, -1,  2, 1);
			get_moves(legal_moves, &i, cb, file, rank,  2,  1, 1);
			get_moves(legal_moves, &i, cb, file, rank,  2, -1, 1);
			get_moves(legal_moves, &i, cb, file, rank,  1, -2, 1);
			get_moves(legal_moves, &i, cb, file, rank, -1, -2, 1);
			get_moves(legal_moves, &i, cb, file, rank, -2,  1, 1);
			get_moves(legal_moves, &i, cb, file, rank, -2, -1, 1);
			break;

		case WHITE_PAWN:   case BLACK_PAWN:
			get_move_pawn_forward(legal_moves, i, &j, cb, file, rank);
			get_move_pawn_capture(legal_moves, i, &j, cb, file, rank,  1, game->en_passant_target_square);
			get_move_pawn_capture(legal_moves, i, &j, cb, file, rank, -1, game->en_passant_target_square);

			if ((color == WHITE && rank == ('1' + NUM_RANKS - 2)) || (color == BLACK && rank == '2')) // pawn promotion
			{
				for (int k = 0; k < j; k++)
				{
					legal_moves[i + k +   j] = legal_moves[i + k];
					legal_moves[i + k + 2*j] = legal_moves[i + k];
					legal_moves[i + k + 3*j] = legal_moves[i + k];
					legal_moves[i + k      ].promoted_piece = (color == WHITE) ? WHITE_QUEEN  : BLACK_QUEEN;
					legal_moves[i + k +   j].promoted_piece = (color == WHITE) ? WHITE_ROOK   : BLACK_ROOK;
					legal_moves[i + k + 2*j].promoted_piece = (color == WHITE) ? WHITE_BISHOP : BLACK_BISHOP;
					legal_moves[i + k + 3*j].promoted_piece = (color == WHITE) ? WHITE_KNIGHT : BLACK_KNIGHT;
				}
				j *= 4;
			}
			i += j;
			break;
	}

	return i;
}


void
update_chessboard(char *chessboard, char *previous_chessboard, piece_move *move)
{
	memcpy(chessboard, previous_chessboard, NUM_SQUARES);

	chessboard[move->from_square] = EMPTY;
	chessboard[move->to_square] = move->moving_piece;

	if (IS_KING(move->moving_piece) && FILE(move->from_square) == 'e') // Castling
	{
		if (move->to_square == SQUARE('g','1'))
		{
			chessboard[SQUARE('h','1')] = EMPTY;
			chessboard[SQUARE('f','1')] = WHITE_ROOK;
		}
		else if (move->to_square == SQUARE('c','1'))
		{
			chessboard[SQUARE('a','1')] = EMPTY;
			chessboard[SQUARE('d','1')] = WHITE_ROOK;
		}
		else if (move->to_square == SQUARE('g','8'))
		{
			chessboard[SQUARE('h','8')] = EMPTY;
			chessboard[SQUARE('f','8')] = BLACK_ROOK;
		}
		else if (move->to_square == SQUARE('c','8'))
		{
			chessboard[SQUARE('a','8')] = EMPTY;
			chessboard[SQUARE('d','8')] = BLACK_ROOK;
		}
	}

	if (move->en_passant)
		chessboard[SQUARE(FILE(move->to_square), RANK(move->from_square))] = EMPTY;

	if (IS_PIECE(move->promoted_piece))
		chessboard[move->to_square] = move->promoted_piece;
}


void
update_state(game_state *game)
{
	game->white_castling_short_ability  =
	game->white_castling_long_ability  &= (game->chessboard[SQUARE('e','1')] == WHITE_KING);
	game->white_castling_short_ability &= (game->chessboard[SQUARE('h','1')] == WHITE_ROOK);
	game->white_castling_long_ability  &= (game->chessboard[SQUARE('a','1')] == WHITE_ROOK);
	game->black_castling_short_ability  =
	game->black_castling_long_ability  &= (game->chessboard[SQUARE('e', ('1' + NUM_RANKS - 1))] == BLACK_KING);
	game->black_castling_short_ability &= (game->chessboard[SQUARE('h', ('1' + NUM_RANKS - 1))] == BLACK_ROOK);
	game->black_castling_long_ability  &= (game->chessboard[SQUARE('a', ('1' + NUM_RANKS - 1))] == BLACK_ROOK);

	if (game->last_move)
	{
		game->en_passant_target_square = NO_SQUARE;
		if (IS_PAWN(game->last_move->moving_piece))
			if (RANK(game->last_move->from_square) == '2' && RANK(game->last_move->to_square) == '4')
				game->en_passant_target_square = SQUARE(FILE(game->last_move->from_square), '3');
			else if (RANK(game->last_move->from_square) == ('1' + NUM_RANKS - 2) && RANK(game->last_move->to_square) == ('1' + NUM_RANKS - 4))
				game->en_passant_target_square = SQUARE(FILE(game->last_move->from_square), ('1' + NUM_RANKS - 3));

		game->half_move_clock++;
		if (IS_PAWN(game->last_move->moving_piece) || game->last_move->capture)
			game->half_move_clock = 0;
	}
}


void
set_game_state(game_state *game, char *chessboard = initial_chessboard, int side_to_move = WHITE, int full_move_counter = 1)
{
	memcpy(game->chessboard, chessboard, NUM_SQUARES);
	game->last_move = NULL;
	game->side_to_move = side_to_move;
	game->white_castling_short_ability = true;
	game->white_castling_long_ability  = true;
	game->black_castling_short_ability = true;
	game->black_castling_long_ability  = true;
	game->en_passant_target_square = NO_SQUARE;
	game->half_move_clock = 0;
	game->full_move_counter = full_move_counter;
	game->previous = NULL;

	update_state(game);
}


bool
can_reach(char *chessboard, char from_file, char from_rank, int to_square, int file_step, int rank_step, int max_steps)
{
	int f = from_file + file_step;
	int r = from_rank + rank_step;

	for (int step = 0; step < max_steps; step++)
	{
		if (f < 'a' || f > ('a' + NUM_FILES - 1) || r < '1' || r > ('1' + NUM_RANKS - 1))
			break;

		int square = SQUARE(f, r);

		if (square == to_square)
			return true;

		if (chessboard[square] != EMPTY)
			break;

		f += file_step;
		r += rank_step;
	}

	return false;
}


bool
square_is_attacked_by_piece(char *chessboard, int from_square, int to_square, char piece)
{
	char file = FILE(from_square);
	char rank = RANK(from_square);
	int color = COLOR(chessboard[from_square]);

	switch (piece)
	{
		case WHITE_KING:   case BLACK_KING:
			if (can_reach(chessboard, file, rank, to_square,  0,  1, 1) ||
			    can_reach(chessboard, file, rank, to_square,  1,  1, 1) ||
				can_reach(chessboard, file, rank, to_square,  1,  0, 1) ||
				can_reach(chessboard, file, rank, to_square,  1, -1, 1) ||
				can_reach(chessboard, file, rank, to_square,  0, -1, 1) ||
				can_reach(chessboard, file, rank, to_square, -1, -1, 1) ||
				can_reach(chessboard, file, rank, to_square, -1,  0, 1) ||
				can_reach(chessboard, file, rank, to_square, -1,  1, 1))
				return true;
			break;

		case WHITE_QUEEN:  case BLACK_QUEEN:
			if (can_reach(chessboard, file, rank, to_square,  0,  1, 7) ||
			    can_reach(chessboard, file, rank, to_square,  1,  1, 7) ||
				can_reach(chessboard, file, rank, to_square,  1,  0, 7) ||
				can_reach(chessboard, file, rank, to_square,  1, -1, 7) ||
				can_reach(chessboard, file, rank, to_square,  0, -1, 7) ||
				can_reach(chessboard, file, rank, to_square, -1, -1, 7) ||
				can_reach(chessboard, file, rank, to_square, -1,  0, 7) ||
				can_reach(chessboard, file, rank, to_square, -1,  1, 7))
				return true;
			break;

		case WHITE_ROOK:   case BLACK_ROOK:
			if (can_reach(chessboard, file, rank, to_square,  0,  1, 7) ||
				can_reach(chessboard, file, rank, to_square,  1,  0, 7) ||
				can_reach(chessboard, file, rank, to_square,  0, -1, 7) ||
				can_reach(chessboard, file, rank, to_square, -1,  0, 7))
				return true;
			break;

		case WHITE_BISHOP: case BLACK_BISHOP:
			if (can_reach(chessboard, file, rank, to_square,  1,  1, 7) ||
				can_reach(chessboard, file, rank, to_square,  1, -1, 7) ||
				can_reach(chessboard, file, rank, to_square, -1, -1, 7) ||
				can_reach(chessboard, file, rank, to_square, -1,  1, 7))
				return true;
			break;

		case WHITE_KNIGHT: case BLACK_KNIGHT:
			if (can_reach(chessboard, file, rank, to_square,  1,  2, 1) ||
				can_reach(chessboard, file, rank, to_square, -1,  2, 1) ||
				can_reach(chessboard, file, rank, to_square,  2,  1, 1) ||
				can_reach(chessboard, file, rank, to_square,  2, -1, 1) ||
				can_reach(chessboard, file, rank, to_square,  1, -2, 1) ||
				can_reach(chessboard, file, rank, to_square, -1, -2, 1) ||
				can_reach(chessboard, file, rank, to_square, -2,  1, 1) ||
				can_reach(chessboard, file, rank, to_square, -2, -1, 1))
				return true;
			break;

		case WHITE_PAWN:
			if (can_reach(chessboard, file, rank, to_square,  1,  1, 1) ||
				can_reach(chessboard, file, rank, to_square, -1,  1, 1))
				return true;
			break;

		case BLACK_PAWN:
			if (can_reach(chessboard, file, rank, to_square,  1, -1, 1) ||
				can_reach(chessboard, file, rank, to_square, -1, -1, 1))
				return true;
			break;
	}

	return false;
}


bool
square_is_attacked(char *chessboard, int color, int square)
{
	for (int attacking_square = 0; attacking_square < NUM_SQUARES; attacking_square++)
	{
		char piece = chessboard[attacking_square];

		if (IS_EMPTY(piece) || (color == COLOR(piece)))
			continue;

		if (square_is_attacked_by_piece(chessboard, attacking_square, square, piece))
			return true;
	}

	return false;
}


bool
castling_under_attack(char *chessboard, piece_move *move)
{
	if (!IS_KING(move->moving_piece))
		return false;

	int color = COLOR(chessboard[move->to_square]);
	char file = 'e';
	char rank = (color == WHITE) ? '1' : '8';
	int king_initial_square = SQUARE(file, rank);
	int file_step = (move->to_square - move->from_square);
	if ((move->from_square != king_initial_square) || (abs(file_step) != 2))
		return false;

	if (square_is_attacked(chessboard, color, king_initial_square))
		return true;

	int king_middle_square = SQUARE(file + (file_step / 2), rank);
	if (square_is_attacked(chessboard, color, king_middle_square))
		return true;

	return false;
}


bool
king_in_check(char *chessboard, int color)
{
	char king = (color == WHITE) ? WHITE_KING : BLACK_KING;
	int square = strchr(chessboard, king) - chessboard;
	bool attacked = square_is_attacked(chessboard, color, square);

	return attacked;
}


bool
forced_draw(game_state *game)
{
	// Draw Rule #1: Fifty moves by each player without capture of any piece, or the movement of a pawn
	if (game->half_move_clock >= (50 + 50))
		return true;

	// Draw Rule #2: Insufficient mating material: a single bishop or a single knight
	bool minor_pieces_only = true;
	int white_bishops_w = 0, white_bishops_b = 0, white_knights = 0, black_bishops_w = 0, black_bishops_b = 0, black_knights = 0;
	for (int square = 0; square < NUM_SQUARES; square++)
	{
		char piece = game->chessboard[square];
		if (IS_EMPTY(piece) || IS_KING(piece))
			continue;

		if (IS_QUEEN(piece) || IS_ROOK(piece) || IS_PAWN(piece))
		{
			minor_pieces_only = false;
			break;
		}

		white_bishops_w |= (piece == WHITE_BISHOP && IS_WHITE_SQUARE(square));
		white_bishops_b |= (piece == WHITE_BISHOP && IS_BLACK_SQUARE(square));
		white_knights   += (piece == WHITE_KNIGHT);
		black_bishops_w |= (piece == BLACK_BISHOP && IS_WHITE_SQUARE(square));
		black_bishops_b |= (piece == BLACK_BISHOP && IS_BLACK_SQUARE(square));
		black_knights   += (piece == BLACK_KNIGHT);
	}
	if (minor_pieces_only && (white_bishops_w + white_bishops_b + white_knights) < 2 &&
							 (black_bishops_w + black_bishops_b + black_knights) < 2)
		return true;

	// Draw Rule #3: Threefold repetition: the same position is reached three times with the same player to move
	char *chessboard = game->chessboard;
	int side_to_move = game->side_to_move;
	int repetitions = 0;
	game_state *old_game = game->previous;
	while(old_game != NULL && old_game->half_move_clock > 0)
	{
		if (old_game->side_to_move == side_to_move && strncmp(old_game->chessboard, chessboard, NUM_SQUARES) == 0)
		{
			repetitions++;
			if (repetitions >= 3)
				return true;
		}
		old_game = old_game->previous;
	}

	return false;
}


void
move_disambiguation(piece_move *move, char *chessboard)
{
	char piece = move->moving_piece;

	if (IS_KING(piece) || IS_PAWN(piece))
		return;

	char file_from = FILE(move->from_square);
	char rank_from = RANK(move->from_square);

	for (int square = 0; square < NUM_SQUARES; square++)
	{
		if ((chessboard[square] != piece) || (square == move->from_square))
			continue;

		if (square_is_attacked_by_piece(chessboard, square, move->to_square, piece))
		{
			if (FILE(square) != file_from)
				move->file_ambiguity = true;
			else
				move->rank_ambiguity = true;

			if (move->file_ambiguity && move->rank_ambiguity)
				return;
		}
	}
}


int
get_move_count_text(char *move_count_text, game_state *game, bool start)
{
	int len = 0;

	if (game->side_to_move == BLACK)
		sprintf(move_count_text, "%d. %n", game->full_move_counter, &len);
	else if (game->side_to_move == WHITE && start)
		sprintf(move_count_text, "%d... %n", (game->full_move_counter - 1), &len);

	return len;
}


int
get_move_text(char *move_text, piece_move *move)
{
	int len = 0;
	char piece = move->moving_piece;
	char *text = move_text;

	if (IS_KING(piece) && FILE(move->from_square) == 'e' && FILE(move->to_square) == 'g')
		sprintf(text, "O-O%n", &len);
	else if (IS_KING(piece) && FILE(move->from_square) == 'e' && FILE(move->to_square) == 'c')
		sprintf(text, "O-O-O%n", &len);
	else
	{
		if (!IS_PAWN(piece))
			sprintf(text += len, "%c%n", toupper(piece), &len);
		if (move->file_ambiguity || (IS_PAWN(piece) && move->capture))
			sprintf(text += len, "%c%n", FILE(move->from_square), &len);
		if (move->rank_ambiguity)
			sprintf(text += len, "%c%n", RANK(move->from_square), &len);
		if (move->capture)
			sprintf(text += len, "x%n", &len);
		sprintf(text += len, "%c%c%n", FILE(move->to_square), RANK(move->to_square), &len);
		if (move->en_passant)
			sprintf(text += len, "e.p.%n", &len);
		if (IS_PIECE(move->promoted_piece))
			sprintf(text += len, "=%c%n", toupper(move->promoted_piece), &len);
	}

	if (move->mate)
		sprintf(text += len, "#%n", &len);
	else if (move->check)
		sprintf(text += len, "+%n", &len);

	if (move->draw)
		sprintf(text += len, "(=)%n", &len);

	len += (text - move_text);
	return len;
}


int
get_move_list(char *move_list, game_state *game, int start_move_counter = 1, int start_side_to_move = WHITE)
{
	if (game == NULL || game->last_move == NULL || game->full_move_counter < start_move_counter ||
		(game->full_move_counter == start_move_counter && game->side_to_move < start_side_to_move))
		return 0;

	int len = get_move_list(move_list, game->previous, start_move_counter, start_side_to_move);

	bool start = (game->full_move_counter == start_move_counter && game->side_to_move == start_side_to_move);
	move_list[len++] = ' ';
	len += get_move_count_text(move_list + len, game, start);
	len += get_move_text(move_list + len, game->last_move);

	return len;
}


bool
first_move(game_state *game)
{
	bool is_first_move = (game->previous == NULL);

	return is_first_move;
}


bool
goal_chessboard_achieved(game_state *game, char *goal_chessboard)
{
	bool chessboard_achieved = goal_is_chessboard && (strncmp(game->chessboard, goal_chessboard, NUM_SQUARES) == 0);

	return chessboard_achieved;
}


void
format_commas(char *formatted_number, long num)
{
	char str_num[80], *fn = formatted_number;
	int last = sprintf(str_num, "%ld", num) - 1;

	for (int i = 0; i <= last; i++)
	{
		*fn++ = str_num[i];
		if ((last - i) % 3 == 0)
			*fn++ = ',';
	}
	*(--fn) = 0;
}


void
print_stats(stats_type type)
{
	char stats_line[2000], variants_analyzed_str[80], mate_results_str[80], draw_results_str[80], chessboard_results_str[80];
	char *text = stats_line;
	int len = 0;

	if (type == TEMPORARY)
		sprintf(text += len, "\nTemporary results:\n%n", &len);
	else if (type == FINAL)
		sprintf(text += len, "\n%n", &len);

	format_commas(variants_analyzed_str, variants_analyzed);
	sprintf(text += len, "Variants analyzed: %s   %n", variants_analyzed_str, &len);

	if (goal_is_mate)
	{
		format_commas(mate_results_str, mate_variant.size());
		sprintf(text += len, "Mate solutions: %s   %n", mate_results_str, &len);
		if (mate_variant.size() > 0)
			sprintf(text += len, "Move count: %d   %n", min_move_count_mate, &len);
	}

	if (goal_is_draw)
	{
		format_commas(draw_results_str, draw_variant.size());
		sprintf(text += len, "Draw solutions: %s   %n", draw_results_str, &len);
		if (draw_variant.size() > 0)
			sprintf(text += len, "Move count: %d   %n", min_move_count_draw, &len);
	}

	if (goal_is_chessboard)
	{
		format_commas(chessboard_results_str, chessboard_variant.size());
		sprintf(text += len, "Chessboard solutions: %s   %n", chessboard_results_str, &len);
		if (chessboard_variant.size() > 0)
			sprintf(text += len, "Move count: %d   %n", min_move_count_chessboard, &len);
	}

	if (type == PERIODIC || type == TEMPORARY)
		sprintf(text += len, "Last variant: %s   %n", last_variant_analyzed, &len);

	if (type == FINAL && save_results_name && (mate_variant.size() + draw_variant.size() + chessboard_variant.size()) > 0)
		sprintf(text += len, "(See %s)%n", save_results_name, &len);

	sprintf(text += len, "\n%n", &len);

	if (type == TEMPORARY || type == FINAL)
		sprintf(text += len, "\n%n", &len);

	printf("%s", stats_line);

	if (type == FINAL && save_results_name)
	{
		save_results = fopen(save_results_name, "a");
		fprintf(save_results, "%s", stats_line);
		fclose(save_results);
	}
}


char *
output_variant(char *print_line, int move_count, char const *move_list, char const *highlight = NULL)
{
	char const *color1 = (highlight ? highlight  : "");
	char const *color2 = (highlight ? FG_DEFAULT : "");

	sprintf(print_line, "%s(%d) %s%s", color1, move_count, move_list, color2);

	return print_line;
}


void
print_variant(game_state *game, int verbose = 0, char const *highlight = NULL)
{
	last_variant_analyzed[0] = 0;
	get_move_list(last_variant_analyzed, game);

	if (verbose)
	{
		char print_line[4000];
		int move_count = (game->side_to_move == WHITE) ? (game->full_move_counter - 1) : game->full_move_counter;
		output_variant(print_line, move_count, last_variant_analyzed, highlight);
		printf("%s\n", print_line);
	}
}


long
set_result(bool finish, bool mate, bool draw, bool chessboard, int move_count_mate, int move_count_draw, int move_count_chessboard)
{
	long result = SET_FINISH(finish) | SET_MATE(mate) | SET_DRAW(draw) | SET_CHESSBOARD(chessboard);

	if (mate)
		result |= SET_MOVE_COUNT_MATE((long) move_count_mate);

	if (draw)
		result |= SET_MOVE_COUNT_DRAW((long) move_count_draw);

	if (chessboard)
		result |= SET_MOVE_COUNT_CHESSBOARD((long) move_count_chessboard);

	return result;
}


long
push_result_forward(bool finish, bool mate, bool draw, bool chessboard, int move_count)
{
	long result = set_result(finish, mate, draw, chessboard, move_count, move_count, move_count);

	return result;
}


long
pull_result_backward(bool mate, bool draw, bool chessboard, int move_count_mate, int move_count_draw, int move_count_chessboard)
{
	long result = set_result(false, mate, draw, chessboard, move_count_mate, move_count_draw, move_count_chessboard);

	return result;
}


void
push_variant(vector<string> &variant_list, game_state *game)
{
	int move_count = (game->side_to_move == WHITE) ? (game->full_move_counter - 1) : game->full_move_counter;
	char move_list[4000], variant[4000];
	move_list[0] = 0;
	get_move_list(move_list, game);
	output_variant(variant, move_count, move_list, NULL);
	variant_list.push_back(string(variant));
}


long
finish_variant(game_state *game, bool print = true, bool mate = false, bool stalemate = false)
{
	bool draw = mate ? false : (stalemate || forced_draw(game));
	bool chessboard = (goal_chessboard_achieved(game, final_chessboard) && (game->side_to_move == initial_side_to_move));
	bool max_moves  = ((game->full_move_counter > max_full_move_count)  && (game->side_to_move == initial_side_to_move));
	bool finish = (mate || draw || chessboard || max_moves);
	int  move_count = (game->side_to_move == WHITE) ? (game->full_move_counter - 1) : game->full_move_counter;

	finish |= ((!goal_is_mate       || game->full_move_counter > min_move_count_mate) &&
			   (!goal_is_draw       || game->full_move_counter > min_move_count_draw) &&
			   (!goal_is_chessboard || game->full_move_counter > min_move_count_chessboard));

	if (print)
	{
		if (!finish && (verbose >= 3))
			print_variant(game, verbose);
		else
			print_variant(game);

		if (finish)
		{
			variants_analyzed++;
			if (variants_analyzed % 1000000 == 0)
				print_stats(PERIODIC);
		}
	}

	long result = push_result_forward(finish, mate, draw, chessboard, move_count);

	return result;
}


long
finish_mate_or_stalemate(game_state *game)
{
	bool mate = false, stalemate = false;

	if (king_in_check(game->chessboard, game->side_to_move))
		mate = true;
	else
		stalemate = true;

	if (game->last_move)
	{
		game->last_move->mate = mate;
		game->last_move->draw = stalemate;
	}

	long result = finish_variant(game, true, mate, stalemate);

	return result;
}


bool
is_valid_piece(char piece, int square, int color)
{
	if (IS_EMPTY(piece) || (color != COLOR(piece)))
		return false;

	if ((*special_piece_restriction)(piece, square)) // Problem specifics
		return false;

	return true;
}


bool
is_valid_move(piece_move *move, game_state *game)
{
	if ((*special_move_restriction)(move)) // Problem specifics
		return false;

	update_chessboard(game->chessboard, game->previous->chessboard, move);

	if (king_in_check(game->chessboard, game->previous->side_to_move)) // Illegal move: Let own king in check
		return false;

	if (castling_under_attack(game->chessboard, move)) // Illegal move: Castling under attack
		return false;

	return true;
}


int
get_valid_move_count(game_state *game)
{
	piece_move next_move, legal_moves[MAX_LEGAL_MOVES];
	game_state next = *game;
	next.last_move = &next_move;
	next.side_to_move = game->side_to_move == WHITE ? BLACK : WHITE;
	next.full_move_counter += game->side_to_move == WHITE ? 0 : 1;
	next.previous = game;

	int color = game->side_to_move;
	int valid_moves = 0;

	for (int square = 0; square < NUM_SQUARES; square++)
	{
		char piece = game->chessboard[square];
		if (is_valid_piece(piece, square, color))
		{
			int legal_move_count = get_legal_moves(legal_moves, game, piece, square);

			for (int i = 0; i < legal_move_count; i++)
			{
				next_move = legal_moves[i];
				if (is_valid_move(&next_move, &next))
					valid_moves++;
			}
		}
	}

	return valid_moves;
}


void
get_valid_moves(vector<piece_move> &valid_moves, game_state *game)
{
	piece_move next_move, legal_moves[MAX_LEGAL_MOVES];
	int color = game->previous->side_to_move;
	long result;

	for (int square = 0; square < NUM_SQUARES; square++)
	{
		char piece = game->previous->chessboard[square];
		if (is_valid_piece(piece, square, color))
		{
			int legal_move_count = get_legal_moves(legal_moves, game->previous, piece, square);

			for (int i = 0; i < legal_move_count; i++)
			{
				next_move = legal_moves[i];

				if (is_valid_move(&next_move, game))
				{
					move_disambiguation(&next_move, game->previous->chessboard);
					next_move.check = king_in_check(game->chessboard, game->side_to_move);
					next_move.next_valid_moves = get_valid_move_count(game);
					if (next_move.next_valid_moves == 0)
					{
						next_move.mate =  next_move.check;
						next_move.draw = !next_move.check;  // Stalemate
					}
					else
					{
						result = finish_variant(game, false);
						next_move.draw = DRAW(result);
						if (FINISH(result))
							next_move.next_valid_moves = 0;
					}
					valid_moves.push_back(next_move);
				}
			}
		}
	}
}


void
print_results(vector<string> &results, bool goal, char const *title, stats_type type)
{
	if (goal && results.size() > 0)
	{
		printf("\n%s results:\n\n", title);

		if (type == FINAL && save_results_name)
		{
			save_results = fopen(save_results_name, "a");
			fprintf(save_results, "\n%s results:\n\n", title);
		}

		for (int i = 0; i < results.size(); i++)
		{
			printf("%s%s%s\n", FG_BOLD_CYAN, results.at(i).c_str(), FG_DEFAULT);
			if (type == FINAL && save_results_name)
				fprintf(save_results, "%s\n", results.at(i).c_str());
		}

		if (type == FINAL && save_results_name)
			fclose(save_results);
	}
}


void
erase_bad_variants(vector<string> &variant, bool goal, bool candidate, size_t game_variants, size_t previous_variants,
				   int *min_move_count, int previous_min_move_count, int move_count, bool finish, bool erase_branch = true)
{
	if (goal)
	{
		if (finish && candidate && (*min_move_count > move_count))
		{	// This is a shorter variant, thus erase its siblings
			*min_move_count = move_count;
			if (previous_variants > game_variants)
				variant.erase(variant.begin() + game_variants, variant.begin() + previous_variants);
		}

		if (!finish && !candidate && erase_branch)
		{	// This is a bad variant, thus erase its whole branch
			*min_move_count = previous_min_move_count;
			if (variant.size() > previous_variants)
				variant.erase(variant.begin() + previous_variants, variant.end());
		}
	}
}


bool
order_by_ascending_next_valid_moves(piece_move move_1, piece_move move_2)
{
	bool order = (move_1.next_valid_moves < move_2.next_valid_moves);

    return order;
}


long
get_all_valid_moves_from_state(game_state *game)
{
	long result;
	bool game_mate_candidate = false, game_draw_candidate = false, game_chessboard_candidate = false;
	bool mate_candidate, draw_candidate, chessboard_candidate;
	size_t game_mate_variants = mate_variant.size();
	size_t game_draw_variants = draw_variant.size();
	size_t game_chessboard_variants = chessboard_variant.size();

	piece_move next_move;
	game_state next = *game;
	next.last_move = &next_move;
	next.side_to_move = game->side_to_move == WHITE ? BLACK : WHITE;
	next.full_move_counter += game->side_to_move == WHITE ? 0 : 1;
	next.previous = game;

	int color = game->side_to_move;
	vector<piece_move> valid_moves;
	get_valid_moves(valid_moves, &next);
	sort(valid_moves.begin(), valid_moves.end(), order_by_ascending_next_valid_moves);

	for (int i = 0; i < valid_moves.size(); i++)
	{
		next_move = valid_moves.at(i);
		update_chessboard(next.chessboard, game->chessboard, &next_move);
		update_state(&next);

		size_t previous_mate_variants = mate_variant.size();
		size_t previous_draw_variants = draw_variant.size();
		size_t previous_chessboard_variants = chessboard_variant.size();
		int previous_min_move_count_mate = min_move_count_mate;
		int previous_min_move_count_draw = min_move_count_draw;
		int previous_min_move_count_chessboard = min_move_count_chessboard;

		result = finish_variant(&next, true, next_move.mate, next_move.draw);
		if (!FINISH(result))
			result = get_all_valid_moves_from_state(&next); // Go recursively until variant reaches an end

		game_mate_candidate |= mate_candidate = (goal_is_mate && MATE(result) && ((color == initial_side_to_move) || !FINISH(result)));
		game_draw_candidate |= draw_candidate = (goal_is_draw && DRAW(result));
		game_chessboard_candidate |= chessboard_candidate = (CHESSBOARD(result) && ((color == initial_side_to_move) || !FINISH(result)));

		if (FINISH(result))
		{
			bool candidate = (mate_candidate || draw_candidate || chessboard_candidate);
			char const *highlight = candidate ? FG_BOLD_LIGHT_RED : NULL;

			if (mate_candidate)
				push_variant(mate_variant, &next);

			if (draw_candidate)
				push_variant(draw_variant, &next);

			if (chessboard_candidate)
				push_variant(chessboard_variant, &next);

			if ((candidate && verbose >= 1) || (verbose >= 2))
				print_variant(&next, verbose, highlight);
		}

		if (color == initial_side_to_move)
		{
			erase_bad_variants(mate_variant, goal_is_mate, mate_candidate, game_mate_variants, previous_mate_variants,
							   &min_move_count_mate, previous_min_move_count_mate, MOVE_COUNT_MATE(result), FINISH(result));

			erase_bad_variants(draw_variant, goal_is_draw, draw_candidate, game_draw_variants, previous_draw_variants,
							   &min_move_count_draw, previous_min_move_count_draw, MOVE_COUNT_DRAW(result), FINISH(result));

			erase_bad_variants(chessboard_variant, goal_is_chessboard, chessboard_candidate, game_chessboard_variants, previous_chessboard_variants,
							   &min_move_count_chessboard, previous_min_move_count_chessboard, MOVE_COUNT_CHESSBOARD(result), FINISH(result), false);
		}
		else if (!mate_candidate && !draw_candidate && !goal_is_chessboard)
			return 0; // Interrupt branch search if there is any variant that leads to a non-goal finish
	}

	if (valid_moves.size() == 0)
		result = finish_mate_or_stalemate(game);
	else
		result = pull_result_backward(game_mate_candidate, game_draw_candidate, game_chessboard_candidate,
									  min_move_count_mate, min_move_count_draw, min_move_count_chessboard);
	return result;
}


void
signal_handler(int signum)
{
	if (signum == SIGQUIT)
	{
		print_results(mate_variant, goal_is_mate, "Mate", TEMPORARY);
		print_results(draw_variant, goal_is_draw, "Draw", TEMPORARY);
		print_stats(TEMPORARY);
	}
	else
		exit(signum);
}


void
secure_file(char const *file_name)
{
	FILE *file = fopen(file_name, "r");

	if (file == NULL)
		return;

	fclose(file);
	pid_t pid = getpid();
	char new_file_name[2000], file_type[80];
	strcpy(new_file_name, file_name);
	char *p = strrchr(new_file_name, '.');
	if (p == NULL)
		p = new_file_name + strlen(new_file_name);
	strcpy(file_type, p);
	(*p) = 0;
	sprintf(new_file_name, "%s_%d%s", new_file_name, pid, file_type);
	rename(file_name, new_file_name);
}


void
usage(int exit_code = 0, char const *error_msg = "", char const *error_msg2 = "")
{
	if (exit_code && (error_msg[0] || error_msg2[0]))
		fprintf(stderr, "\n%s%s\n", error_msg, error_msg2);

	fprintf(stderr, "\nUsage: kc [args]\n" " args:\n"
		"    -i <FEN>       : initial chessboard (Forsyth-Edwards Notation)\n"
		"    -f <FEN>       : search for final chessboard (Forsyth-Edwards Notation)\n"
		"    -m             : search for forced mate\n"
		"    -d             : search for forced draw\n"
		"    -n <moves>     : maximum number of moves\n"
		"    -r <file>      : results filename\n"
		"    -v <verbose>   : verbose level\n\n");

	if (exit_code)
		exit(exit_code);
}


void
read_parameters(int argc, char **argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-i") == 0)
		{
			if (i == argc - 1)
				usage(1, "Initial chessboard (FEN) expected after -i");
			i++;
			initial_fen = argv[i];
			if (fen_piece_placement(initial_chessboard, argv[i]) != 0)
				usage(2, "FEN syntax error after -i: ", argv[i]);
		}
		else if (strcmp(argv[i], "-f") == 0)
		{
			if (i == argc - 1)
				usage(3, "Final chessboard (FEN) expected after -f");
			i++;
			final_fen = argv[i];
			if (fen_piece_placement(final_chessboard, argv[i]) != 0)
				usage(4, "FEN syntax error after -f: ", argv[i]);
			goal_is_chessboard = true;
		}
		else if (strcmp(argv[i], "-m") == 0)
			goal_is_mate = true;
		else if (strcmp(argv[i], "-d") == 0)
			goal_is_draw = true;
		else if (strcmp(argv[i], "-n") == 0)
		{
			if (i == argc - 1)
				usage(5, "Number expected after -n");
			i++;
			char *p;
			max_full_move_count = strtol(argv[i], &p, 10);
			if (p != argv[i] + strlen(argv[i]) || max_full_move_count <= 0)
				usage(6, "Invalid number after -n: ", argv[i]);
		}
		else if (strcmp(argv[i], "-r") == 0)
		{
			if (i == argc - 1)
				usage(7, "File name expected after -r");
			i++;
			save_results_name = argv[i];
		}
		else if (strcmp(argv[i], "-v") == 0)
		{
			if (i == argc - 1 || argv[i + 1][0] == '-')
				verbose = 1;
			else
			{
				i++;
				char *p;
				verbose = strtol(argv[i], &p, 10);
				if (p != argv[i] + strlen(argv[i]) || verbose < 0)
					usage(8, "Invalid number after -v: ", argv[i]);
			}
		}
		else
			usage(9, "Invalid command line argument: ", argv[i]);
	}

	if (!(goal_is_mate || goal_is_draw || goal_is_chessboard))
		usage(10, "At least one search option (-f -m -d) must be set");

	print_chessboard(initial_chessboard);

	if (king_in_check(initial_chessboard, (initial_side_to_move == WHITE) ? BLACK : WHITE))
	{
		fprintf(stderr, "%s king is in check on initial chessboard: %s\n\n", (initial_side_to_move == WHITE ? "Black" : "White"), initial_fen);
		exit(11);
	}
}


bool
piece_restriction_K_Ra1_Pa(char piece, int square)
{
	// Rule #1 for the Kuwait problem: K, Ra1 and Pa never move
	if ((piece == WHITE_KING) || (piece == WHITE_ROOK && square == SQUARE('a','1')) || (piece == WHITE_PAWN && FILE(square) == 'a'))
		return true;

	return false;
}


bool
move_restriction_Pb_Pf_capture(piece_move *move)
{
	// Rule #2 for the Kuwait problem: Pb...Pf cannot move ahead and cannot capture pieces to the right side
	if (move->moving_piece == WHITE_PAWN && FILE(move->from_square) <= 'f' && FILE(move->from_square) <= FILE(move->to_square))
		return true;

	// Rule #3 for the Kuwait problem: Every other white piece but Pb...Pf cannot capture anything
	if (COLOR(move->moving_piece) == WHITE && !(move->moving_piece == WHITE_PAWN && FILE(move->from_square) <= 'f') && move->capture)
		return true;

	return false;
}


int
main(int argc, char **argv)
{
	if (argc == 1 || strcmp(argv[1], "-h") == 0)
		usage(-1);

	fen_piece_placement(initial_chessboard, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
	read_parameters(argc, argv);
	set_game_state(&initial_game, initial_chessboard, initial_side_to_move);
	secure_file(save_results_name);

	if (goal_is_chessboard && strcmp(final_fen, "k7/P7/P7/P7/P7/P7/P7/R3K3") == 0) // Kuwait chess problem specifics
	{
		special_piece_restriction = piece_restriction_K_Ra1_Pa;
		special_move_restriction  = move_restriction_Pb_Pf_capture;
		max_full_move_count = 34;
	}

	signal(SIGQUIT, signal_handler);
	printf("\nPress Ctrl+\\ to display temporary results\n");

	get_all_valid_moves_from_state(&initial_game);

	print_results(mate_variant, goal_is_mate, "Mate", FINAL);
	print_results(draw_variant, goal_is_draw, "Draw", FINAL);
	print_stats(FINAL);
	return 0;
}
