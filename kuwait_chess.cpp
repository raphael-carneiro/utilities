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

char final_chessboard[CHESSBOARD_SQUARES];
int  variants_analyzed = 0;
int  successful_results = 0;
int  min_full_move_count = 34;
int  max_full_move_count = 34;
FILE *save_results = NULL;
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

	for (int square = 0; square < CHESSBOARD_SQUARES; square++)
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
	char working_chessboard[CHESSBOARD_SQUARES];
	memset(working_chessboard, EMPTY, CHESSBOARD_SQUARES);

	for (char const *c = fen_text; *c != 0 && *c != ' '; c++)
	{
		if (square >= CHESSBOARD_SQUARES)
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

	if (square != CHESSBOARD_SQUARES)
		return error_fen(3, "FEN text too short", fen_text, (fen_text + strlen(fen_text)));

	if (piece_count_validation(working_chessboard))
		return error_fen(6, "Invalid number of pieces", fen_text, (fen_text + strlen(fen_text)));

	memcpy(chessboard, working_chessboard, CHESSBOARD_SQUARES);
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


void
get_moves(piece_move *moves, int *index, char *chessboard, char file, char rank, int file_step, int rank_step, int max_steps)
{
	int color = COLOR(chessboard[SQUARE(file, rank)]);
	int f = file + file_step;
	int r = rank + rank_step;

	for (int step = 0; step < max_steps; step++)
	{
		if (f < 'a' || f > 'h' || r < '1' || r > '8')
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
	char rank = (color == WHITE) ? '1' : '8';
	int first_empty_square = IS_KING(castling_side) ? SQUARE(file + 1, rank) : SQUARE(file - 3, rank);
	int last_empty_square  = IS_KING(castling_side) ? SQUARE(file + 2, rank) : SQUARE(file - 1, rank);

	for (int square = first_empty_square; square <= last_empty_square; square++)
		if (chessboard[square] != EMPTY)
			return;

	moves[*index].to_square = IS_KING(castling_side) ? SQUARE(file + 2, rank) : SQUARE(file - 2, rank);
	(*index)++;
}


void
get_move_pawn_forward(piece_move *moves, int index_base, int *index, char *chessboard, char file, char rank)
{
	int color = COLOR(chessboard[SQUARE(file, rank)]);
	int rank_step = (color == WHITE) ? 1 : -1;
	char first_rank = (color == WHITE) ? '2' : '7';

	int square = SQUARE(file, rank + rank_step);
	if (chessboard[square] == EMPTY)
	{
		moves[index_base + (*index)].to_square = square;
		(*index)++;

		square = SQUARE(file, rank + rank_step * 2);
		if (chessboard[square] == EMPTY && rank == first_rank)
		{
			moves[index_base + (*index)].to_square = square;
			(*index)++;
		}
	}
}


void
get_move_pawn_capture(piece_move *moves, int index_base, int *index, char *chessboard, char file, char rank)
{
	int color = COLOR(chessboard[SQUARE(file, rank)]);
	int rank_step = (color == WHITE) ? 1 : -1;

	int square = SQUARE(file + 1, rank + rank_step);
	if (COLOR(chessboard[square]) != color)
	{
		moves[index_base + (*index)].to_square = square;
		moves[index_base + (*index)].capture = true;
		(*index)++;
	}
	square = SQUARE(file - 1, rank + rank_step);
	if (COLOR(chessboard[square]) != color)
	{
		moves[index_base + (*index)].to_square = square;
		moves[index_base + (*index)].capture = true;
		(*index)++;
	}
}


void
get_move_pawn_en_passant(piece_move *moves, int index_base, int *index, int color, int en_passant_target_square, char file, char rank)
{
	int rank_step = (color == WHITE) ? 1 : -1;
	int square = SQUARE(file + 1, rank + rank_step);
	if (square == en_passant_target_square)
	{
		moves[index_base + (*index)].to_square = square;
		moves[index_base + (*index)].capture = true;
		moves[index_base + (*index)].en_passant = true;
		(*index)++;
	}
	square = SQUARE(file - 1, rank + rank_step);
	if (square == en_passant_target_square)
	{
		moves[index_base + (*index)].to_square = square;
		moves[index_base + (*index)].capture = true;
		moves[index_base + (*index)].en_passant = true;
		(*index)++;
	}
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

	int i = 0;
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
			get_moves(legal_moves, &i, cb, file, rank,  0,  1, 7);
			get_moves(legal_moves, &i, cb, file, rank,  1,  1, 7);
			get_moves(legal_moves, &i, cb, file, rank,  1,  0, 7);
			get_moves(legal_moves, &i, cb, file, rank,  1, -1, 7);
			get_moves(legal_moves, &i, cb, file, rank,  0, -1, 7);
			get_moves(legal_moves, &i, cb, file, rank, -1, -1, 7);
			get_moves(legal_moves, &i, cb, file, rank, -1,  0, 7);
			get_moves(legal_moves, &i, cb, file, rank, -1,  1, 7);
			break;

		case WHITE_ROOK:   case BLACK_ROOK:
			get_moves(legal_moves, &i, cb, file, rank,  0,  1, 7);
			get_moves(legal_moves, &i, cb, file, rank,  1,  0, 7);
			get_moves(legal_moves, &i, cb, file, rank,  0, -1, 7);
			get_moves(legal_moves, &i, cb, file, rank, -1,  0, 7);
			break;

		case WHITE_BISHOP: case BLACK_BISHOP:
			get_moves(legal_moves, &i, cb, file, rank,  1,  1, 7);
			get_moves(legal_moves, &i, cb, file, rank,  1, -1, 7);
			get_moves(legal_moves, &i, cb, file, rank, -1, -1, 7);
			get_moves(legal_moves, &i, cb, file, rank, -1,  1, 7);
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
			int j = 0;
			get_move_pawn_forward(legal_moves, i, &j, cb, file, rank);
			get_move_pawn_capture(legal_moves, i, &j, cb, file, rank);
			get_move_pawn_en_passant(legal_moves, i, &j, color, game->en_passant_target_square, file, rank);

			if ((color == WHITE && rank == '7') || (color == BLACK && rank == '2')) // pawn promotion
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
	memcpy(chessboard, previous_chessboard, CHESSBOARD_SQUARES);

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
	if (game->chessboard[SQUARE('e','1')] != WHITE_KING)
		game->white_castling_short_ability = game->white_castling_long_ability = false;
	if (game->chessboard[SQUARE('h','1')] != WHITE_ROOK)
		game->white_castling_short_ability = false;
	if (game->chessboard[SQUARE('a','1')] != WHITE_ROOK)
		game->white_castling_long_ability = false;
	if (game->chessboard[SQUARE('e','8')] != BLACK_KING)
		game->black_castling_short_ability = game->black_castling_long_ability = false;
	if (game->chessboard[SQUARE('h','8')] != BLACK_ROOK)
		game->black_castling_short_ability = false;
	if (game->chessboard[SQUARE('a','8')] != BLACK_ROOK)
		game->black_castling_long_ability = false;

	if (IS_PAWN(game->last_move->moving_piece))
		if (RANK(game->last_move->from_square) == '2' && RANK(game->last_move->to_square) == '4')
			game->en_passant_target_square = SQUARE(FILE(game->last_move->from_square), '3');
		else if (RANK(game->last_move->from_square) == '7' && RANK(game->last_move->to_square) == '5')
			game->en_passant_target_square = SQUARE(FILE(game->last_move->from_square), '6');

	game->half_move_clock++;
	if (IS_PAWN(game->last_move->moving_piece) || game->last_move->capture)
		game->half_move_clock = 0;
}

//			can_reach(chessboard, from_file, to_file, from_rank, to_rank, 0, 1, 7)

bool
can_reach(char *chessboard, char from_file, char from_rank, int to_square, int file_step, int rank_step, int max_steps)
{
	int f = from_file + file_step;
	int r = from_rank + rank_step;

	for (int step = 0; step < max_steps; step++)
	{
		if (f < 'a' || f > 'h' || r < '1' || r > '8')
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
	for (int attacking_square = 0; attacking_square < CHESSBOARD_SQUARES; attacking_square++)
	{
		char piece = chessboard[attacking_square];
		if ((color == WHITE && !IS_BLACK(piece)) || (color == BLACK && !IS_WHITE(piece)))
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
	if (move->from_square != king_initial_square)
		return false;

	int file_step = (move->to_square - move->from_square);
	if (abs(file_step) != 2)
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
	int square;

	for (square = 0; square < CHESSBOARD_SQUARES; square++)
		if (IS_KING(chessboard[square]) && COLOR(chessboard[square]) == color)
			break;

	return square_is_attacked(chessboard, color, square);
}


bool
forced_draw(game_state *game)
{
	// Fifty moves by each player without capture of any piece, or the movement of a pawn
	if (game->half_move_clock >= 100)
		return true;

	// Insufficient mating material: a single bishop or a single knight
	bool insufficient = true;
	int white_bishops = 0, white_knights = 0, black_bishops = 0, black_knights = 0;
	for (int square = 0; square < CHESSBOARD_SQUARES; square++)
	{
		char piece = game->chessboard[square];
		if (IS_EMPTY(piece) || IS_KING(piece))
			continue;

		if (IS_QUEEN(piece) || IS_ROOK(piece) || IS_PAWN(piece))
		{
			insufficient = false;
			break;
		}

		white_bishops += (IS_BISHOP(piece) && COLOR(piece) == WHITE);
		white_knights += (IS_KNIGHT(piece) && COLOR(piece) == WHITE);
		black_bishops += (IS_BISHOP(piece) && COLOR(piece) == BLACK);
		black_knights += (IS_KNIGHT(piece) && COLOR(piece) == BLACK);
	}
	if (insufficient && (white_bishops + white_knights) <= 1 && (black_bishops + black_knights) <= 1)
		return true;

	// Threefold repetition: the same position is reached three times with the same player to move
	char *chessboard = game->chessboard;
	int side_to_move = game->side_to_move;
	int repetitions = 0;
	game_state *old_game = game->previous;
	while(old_game != NULL && old_game->half_move_clock > 0)
	{
		if (old_game->side_to_move == side_to_move && strncmp(old_game->chessboard, chessboard, CHESSBOARD_SQUARES) == 0)
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
move_disambiguation(char *chessboard, piece_move *move)
{
	char piece = move->moving_piece;

	if (IS_KING(piece) || IS_PAWN(piece))
		return;

	char file = FILE(move->from_square);
	char rank = RANK(move->from_square);

	for (int square = 0; square < CHESSBOARD_SQUARES; square++)
	{
		if ((chessboard[square] != piece) || (square == move->to_square))
			continue;

		if (square_is_attacked_by_piece(chessboard, square, move->to_square, piece))
		{
			if (FILE(square) == file)
				move->rank_ambiguity = true;
			if (RANK(square) == rank)
				move->file_ambiguity = true;
			if (move->file_ambiguity && move->rank_ambiguity)
				return;
		}
	}
}


void
get_move_list(char *move_list, game_state *game)
{
	if (game == NULL || game->last_move == NULL)
		return;

	get_move_list(move_list, game->previous);

	piece_move move = *(game->last_move);
	char piece = move.moving_piece;
	char text[20] = {0};

	if (game->side_to_move == BLACK)
		sprintf(text, "%d. ", game->full_move_counter);
	if (game->side_to_move == WHITE && move_list[0] == 0)
		sprintf(text, "%d... ", game->full_move_counter - 1);

	if (IS_KING(piece) && FILE(move.from_square) == 'e' && FILE(move.to_square) == 'g')
		sprintf(text, "%sO-O", text);
	else if (IS_KING(piece) && FILE(move.from_square) == 'e' && FILE(move.to_square) == 'c')
		sprintf(text, "%sO-O-O", text);
	else
	{
		if (!IS_PAWN(piece))
			sprintf(text, "%s%c", text, toupper(piece));
		if (move.file_ambiguity || (IS_PAWN(piece) && move.capture))
			sprintf(text, "%s%c", text, FILE(move.from_square));
		if (move.rank_ambiguity)
			sprintf(text, "%s%c", text, RANK(move.from_square));
		if (move.capture)
			sprintf(text, "%sx", text);
		sprintf(text, "%s%c%c", text, FILE(move.to_square), RANK(move.to_square));
		if (move.en_passant)
			sprintf(text, "%se.p.", text);
		if (IS_PIECE(move.promoted_piece))
			sprintf(text, "%s=%c", text, toupper(move.promoted_piece));
	}

	if (move.mate)
		sprintf(text, "%s#", text);
	else if (move.check)
		sprintf(text, "%s+", text);

	if (move.draw)
		sprintf(text, "%s(=)", text);

	sprintf(move_list, "%s %s", move_list, text);
}


bool
goal_achieved(char *chessboard, char *goal, game_state *game)
{
	if (strncmp(chessboard, goal, CHESSBOARD_SQUARES) == 0)
	{
		char move_list[4000];
		int move_count = game->full_move_counter;
		if (game->side_to_move == WHITE)
			move_count--;
		if (move_count < min_full_move_count)
			min_full_move_count = move_count;
		get_move_list(move_list, game);
		save_results = fopen("kuwait_chess.txt", "a");
		fprintf(save_results, "(%d) %s\n", move_count, move_list);
		fclose(save_results);
		successful_results++;
		return true;
	}

	return false;
}


void
make_move(game_state *previous)
{
	piece_move current_move, legal_moves[MAX_LEGAL_MOVES];
	game_state current = *previous;
	current.last_move = &current_move;
	current.side_to_move = previous->side_to_move == WHITE ? BLACK : WHITE;
	current.full_move_counter += previous->side_to_move == WHITE ? 0 : 1;
	current.previous = previous;

	int color = previous->side_to_move;
	int valid_moves = 0;
	for (int square = 0; square < CHESSBOARD_SQUARES; square++)
	{
		char piece = previous->chessboard[square];
		if ((color == WHITE && !IS_WHITE(piece)) || (color == BLACK && !IS_BLACK(piece)))
			continue;

		// Rule #1 for the Kuwaiti problem: K, Ra1 and Pa never move
		if ((piece == WHITE_KING) || (piece == WHITE_ROOK && square == SQUARE('a','1')) || (piece == WHITE_PAWN && FILE(square) == 'a'))
			continue;

		int legal_move_count = get_legal_moves(legal_moves, previous, piece, square);
		for (int i = 0; i < legal_move_count; i++)
		{
			current_move = legal_moves[i];

			// Rule #2 for the Kuwaiti problem: Pb...Pf cannot move ahead and can only capture pieces to the left side
			if (current_move.moving_piece == WHITE_PAWN && FILE(current_move.from_square) <= 'f' &&
				FILE(current_move.from_square) <= FILE(current_move.to_square))
				continue;

			// Rule #3 for the Kuwaiti problem: Every other white piece but Pb...Pf cannot capture anything
			if (color == WHITE && !(current_move.moving_piece == WHITE_PAWN && FILE(current_move.from_square) <= 'f') &&
				current_move.capture)
				continue;

			update_chessboard(current.chessboard, previous->chessboard, &current_move);

			// Illegal move: let own king in check
			if (king_in_check(current.chessboard, color))
				continue;

			// Illegal move: castling under attack
			if (castling_under_attack(current.chessboard, &current_move))
				continue;

			update_state(&current);
			valid_moves++;
			move_disambiguation(current.chessboard, &current_move);
			if (king_in_check(current.chessboard, current.side_to_move))
				current_move.check = true;

			bool finish_variant = (color == BLACK && goal_achieved(current.chessboard, final_chessboard, &current));
			finish_variant |= (color == BLACK && previous->full_move_counter >= max_full_move_count);
			finish_variant |= forced_draw(&current);
			if (finish_variant)
			{
				variants_analyzed++;
				if (variants_analyzed % 1000000 == 0)
					printf("Variants analyzed: %d   Solutions: %d   Minimum move count: %d   (See kuwait_chess.txt)\n",
							variants_analyzed, successful_results, min_full_move_count);
			}
			else
				make_move(&current);
		}
	}

	if (valid_moves == 0)
	{
		if (king_in_check(previous->chessboard, previous->side_to_move))
			previous->last_move->mate = true;
		else
			previous->last_move->draw = true;  // Stalemate
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

	printf("Variants analyzed: %d   Solutions: %d   Minimum move count: %d   (See kuwait_chess.txt)\n",
			variants_analyzed, successful_results, min_full_move_count);
	return 0;
}
