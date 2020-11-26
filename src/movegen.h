#ifndef MOVEGEN_H_INCLUDED
#define MOVEGEN_H_INCLUDED

#include "position.h"

typedef struct {
  int moves[MAX_MOVES];
  int scores[MAX_MOVES];
  int count;
} Movelist;

void add_move(Position *pos, Movelist *list, bool check, int move);
void add_castling(Position *pos, Movelist *list, bool check, int move);
void add_pawn_move(Position *pos, Movelist *list, bool check, int from, int to);
void generate_all_moves(Position *pos, Movelist *list);
void movelist_pretty(Movelist *movelist);

#endif