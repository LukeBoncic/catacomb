#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include "bitboard.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

extern Key SideKey;
extern Key CastleKeys[16];
extern Key PassantKeys[64];
extern Key PieceKeys[16][64];

typedef struct {
  int ply;
  int rule;
  int count[16];
  int castling;
  int side;
  int passant;
  int lists[16][10];
  Bitboard occupied[2];
  Bitboard pawns[2];
  Key key;
} Position;

void position_init();
void update_key(Position *pos);
void pos_pretty(Position *pos);
void reset_pos(Position *pos);
void parse_fen(Position *pos, char *fen);
void do_move(Position *pos, int move);
bool move_attacked(Position *pos, int from, int to, int color);
bool sq_attacked(Position *pos, int sq, int color);
int move_pinned(Position *pos, int from, int to, int color);
int sq_pinned(Position *pos, int sq, int color);

#endif