/*
 * kuwait_chess.hpp
 *
 *  Created on: 02-Oct-2019
 *      Author: Raphael V. Carneiro
 */

#ifndef KUWAIT_CHESS_HPP_
#define KUWAIT_CHESS_HPP_

#define	EMPTY		NULL
#define PAWN		'P'
#define KNIGHT		'N'
#define BISHOP		'B'
#define ROOK		'R'
#define QUEEN		'Q'
#define KING		'K'

#define WHITE(x)		toupper(x)
#define BLACK(x)		tolower(x)
#define IS_WHITE(x)		isupper(x)
#define IS_BLACK(x)		islower(x)
#define IS_EMPTY(x)		(x == EMPTY)
#define IS_PAWN(x)		(toupper(x) == PAWN)
#define IS_KNIGHT(x)	(toupper(x) == KNIGHT)
#define IS_BISHOP(x)	(toupper(x) == BISHOP)
#define IS_ROOK(x)		(toupper(x) == ROOK)
#define IS_QUEEN(x)		(toupper(x) == QUEEN)
#define IS_KING(x)		(toupper(x) == KING)

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

#define FILE_CHAR(x)		('a' + (x % 8))
#define RANK_DIGIT(x)		('8' - (x / 8))






#endif /* KUWAIT_CHESS_HPP_ */
