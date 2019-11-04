/*
 * kuwait_chess.hpp
 *
 *  Created on: 02-Oct-2019
 *      Author: Raphael V. Carneiro
 */

#ifndef KUWAIT_CHESS_HPP_
#define KUWAIT_CHESS_HPP_

#define NUM_FILES		 8
#define NUM_RANKS		 8
#define NUM_SQUARES		64 // (NUM_FILES * NUM_RANKS)
#define NUM_PAWNS		 8
#define NUM_KNIGHTS		 2
#define NUM_BISHOPS		 2
#define NUM_ROOKS		 2
#define NUM_QUEENS		 1
#define NUM_KINGS		 1
#define MAX_LEGAL_MOVES	27 // Legal moves for a queen on a central square of an empty chessboard

#define WHITE			 0
#define BLACK			 1
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
#define WHITE_PIECES	"PNBRQK"
#define BLACK_PIECES	"pnbrqk"
#define RANK_SEPARATOR	'/'

#define COLOR(piece)		(islower(piece) != 0)
#define IS_WHITE(piece)		(strchr(WHITE_PIECES, (piece)) != NULL)
#define IS_BLACK(piece)		(strchr(BLACK_PIECES, (piece)) != NULL)
#define IS_PIECE(piece)		(IS_WHITE(piece) || IS_BLACK(piece))
#define IS_EMPTY(piece)		((piece) == EMPTY)
#define IS_PAWN(piece)		((piece) == WHITE_PAWN   || (piece) == BLACK_PAWN)
#define IS_KNIGHT(piece)	((piece) == WHITE_KNIGHT || (piece) == BLACK_KNIGHT)
#define IS_BISHOP(piece)	((piece) == WHITE_BISHOP || (piece) == BLACK_BISHOP)
#define IS_ROOK(piece)		((piece) == WHITE_ROOK   || (piece) == BLACK_ROOK)
#define IS_QUEEN(piece)		((piece) == WHITE_QUEEN  || (piece) == BLACK_QUEEN)
#define IS_KING(piece)		((piece) == WHITE_KING   || (piece) == BLACK_KING)

#define FILE(square)		('a' + ((square) % NUM_FILES)) // files are labeled left to right from 'a' to 'h'
#define RANK(square)		('8' - ((square) / NUM_FILES)) // ranks are numbered bottom-up from '1' to '8'
#define SQUARE(file, rank)	(((file) - 'a') + ('8' - (rank)) * NUM_FILES)

#define NO_SQUARE				-1
#define IS_BLACK_SQUARE(square)	((((square) / NUM_FILES) % 2) ^ (((square) % NUM_FILES) % 2)) // (rank even and file odd)  or (rank odd and file even)
#define IS_WHITE_SQUARE(square)	!IS_BLACK_SQUARE(square) 				  					  // (rank even and file even) or (rank odd and file odd)

#define SET_FINISH(bit)					( bit)
#define SET_MATE(bit)					((bit) <<  1)
#define SET_DRAW(bit)					((bit) <<  2)
#define SET_CHESSBOARD(bit)				((bit) <<  3)
#define SET_MOVE_COUNT_MATE(num)		((num) << 16)
#define SET_MOVE_COUNT_DRAW(num)		((num) << 32)
#define SET_MOVE_COUNT_CHESSBOARD(num)	((num) << 48)

#define FINISH(result)					( (result) & 0x01)
#define MATE(result)					(((result) & 0x02) >> 1)
#define DRAW(result)					(((result) & 0x04) >> 2)
#define CHESSBOARD(result)				(((result) & 0x08) >> 3)
#define MOVE_COUNT_MATE(result)			(((result) & 0xFFFF0000) >> 16)
#define MOVE_COUNT_DRAW(result)			(((result) & 0xFFFF00000000) >> 32)
#define MOVE_COUNT_CHESSBOARD(result)	(((result) & 0xFFFF000000000000) >> 48)

enum stats_type {PERIODIC, TEMPORARY, FINAL};

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
	char chessboard[NUM_SQUARES]; // squares are numbered from left to right top-down
	piece_move *last_move;
	int  side_to_move; // (next move) 0 = white, 1 = black
	bool white_castling_short_ability; // K and Rh have never moved and Rh was not captured
	bool white_castling_long_ability;  // K and Ra have never moved and Ra was not captured
	bool black_castling_short_ability;
	bool black_castling_long_ability;
	int  en_passant_target_square; // skipped square from the last move of pawn
	int  half_move_clock; // counter for 50 move draw rule (without capture or pawn move)
	int  full_move_counter; // (next move)
	game_state *previous;
};


#endif /* KUWAIT_CHESS_HPP_ */
