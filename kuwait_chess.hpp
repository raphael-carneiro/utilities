/*
 * kuwait_chess.hpp
 *
 *  Created on: 02-Oct-2019
 *      Author: Raphael V. Carneiro
 */

#ifndef KUWAIT_CHESS_HPP_
#define KUWAIT_CHESS_HPP_

#define CHESSBOARD_SQUARES	64
#define MAX_LEGAL_MOVES		27

#define WHITE			0
#define BLACK			1
#define	EMPTY			' '
#define WHITE_PAWN		'P'
#define WHITE_KNIGHT	'N'
#define WHITE_BISHOP	'B'
#define WHITE_ROOK		'R'
#define WHITE_QUEEN		'Q'
#define WHITE_KING		'K'
#define BLACK_PAWN		'p'
#define BLACK_KNIGHT	'n'
#define BLACK_BISHOP	'b'
#define BLACK_ROOK		'r'
#define BLACK_QUEEN		'q'
#define BLACK_KING		'k'
#define PIECES			"PNBRQKpnbrqk"
#define WHITE_PIECES	"PNBRQK"
#define BLACK_PIECES	"pnbrqk"
#define RANK_SEPARATOR	'/'

#define COLOR(piece)		(islower(piece))
#define IS_PIECE(piece)		(strchr(PIECES,       piece) != NULL)
#define IS_WHITE(piece)		(strchr(WHITE_PIECES, piece) != NULL)
#define IS_BLACK(piece)		(strchr(BLACK_PIECES, piece) != NULL)
#define IS_EMPTY(piece)		(piece == EMPTY)
#define IS_PAWN(piece)		(piece == WHITE_PAWN   || piece == BLACK_PAWN)
#define IS_KNIGHT(piece)	(piece == WHITE_KNIGHT || piece == BLACK_KNIGHT)
#define IS_BISHOP(piece)	(piece == WHITE_BISHOP || piece == BLACK_BISHOP)
#define IS_ROOK(piece)		(piece == WHITE_ROOK   || piece == BLACK_ROOK)
#define IS_QUEEN(piece)		(piece == WHITE_QUEEN  || piece == BLACK_QUEEN)
#define IS_KING(piece)		(piece == WHITE_KING   || piece == BLACK_KING)

#define FILE(square)		('a' + (square % 8)) // files are labeled from 'a' to 'h' left to right
#define RANK(square)		('8' - (square / 8)) // ranks are numbered bottom-up
#define SQUARE(file, rank)	((file - 'a') + ('8' - rank) * 8)

#define NO_SQUARE				-1
#define IS_BLACK_SQUARE(square)	(((square / 8) % 2) ^ ((square % 8) % 2)) // (rank even and file odd)  or (rank odd and file even)
#define IS_WHITE_SQUARE(square)	!IS_BLACK_SQUARE(square) 				  // (rank even and file even) or (rank odd and file odd)

struct piece_move
{
	char moving_piece;
	char promoted_piece;
	int  from_square;
	int  to_square;
	bool file_ambiguity;
	bool rank_ambiguity;
	bool capture;
	bool en_passant;
	bool check;
	bool mate;
	bool draw;
};

struct game_state
{
	char chessboard[64]; // squares are numbered from left to right top-down
	piece_move *last_move;
	int  side_to_move; // (next move) 0 = white, 1 = black
	bool white_castling_short_ability; // K and Rh have never moved and Rh was not captured
	bool white_castling_long_ability;  // K and Ra have never moved and Ra was not captured
	bool black_castling_short_ability;
	bool black_castling_long_ability;
	int  en_passant_target_square; // skipped square from the last move (pawn)
	int  half_move_clock; // counter for 50 move draw rule (without capture or pawn move)
	int  full_move_counter; // (next move)
	game_state *previous;
};


#endif /* KUWAIT_CHESS_HPP_ */
