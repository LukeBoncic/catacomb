#include <stdio.h>

#include "position.h"

#define RAND_64 ((Key)rand() | (Key)rand() << 15 | (Key)rand() << 30 | (Key)rand() << 45 | ((Key)rand() & 0xf) << 60 )

Key PieceKeys[16][64];
Key SideKey;
Key CastleKeys[16];
Key PassantKeys[64];

void position_init() {
	for (int a = 0; a < 16; ++a)
		for (int b = 0; b < 64; ++b)
			PieceKeys[a][b] = RAND_64;
	for (int c = 0; c < 16; ++c)
		CastleKeys[c] = RAND_64;
	for (int d = 0; d < 64; ++d)
		PassantKeys[d] = RAND_64;
	SideKey = RAND_64;
}

void update_key(Position *pos) {
	Key key = 0x0ULL;
	for (int piece = 0; piece < 16; ++piece)
    for (int count = 0; count < pos->count[piece]; ++count)
			key ^= PieceKeys[piece][pos->lists[piece][count]];
	if (pos->side == WHITE)
		key ^= SideKey;
	if (pos->passant != SQ_NONE)
		key ^= PassantKeys[pos->passant];
	key ^= CastleKeys[pos->castling];
	pos->key = key;
}

void pos_pretty(Position *pos) {
  int pieces[64];
  char *strings[16] = {
    "----", "K---", "-Q--", "KQ--",
    "--k-", "K-k-", "-Qk-", "KQk-",
    "---q", "K--q", "-Q-q", "KQ-q",
    "--kq", "K-kq", "-Qkq", "KQkq"
  };
  for (int s = 0; s < 64; ++s)
    pieces[s] = -1;
  for (int p = 0; p < 16; p++)
    for (int c = 0; c < pos->count[p]; c++)
      pieces[pos->lists[p][c]] = p;
  printf("+---+---+---+---+---+---+---+---+\n");
  for (int r = 7; r >= 0; r--) {
    for (int f = 0; f <= 7; f++) {
      if (pieces[make_square(f, r)] == W_PAWN)
        printf("| P ");
      else if (pieces[make_square(f, r)] == W_KNIGHT)
        printf("| N ");
      else if (pieces[make_square(f, r)] == W_BISHOP)
        printf("| B ");
      else if (pieces[make_square(f, r)] == W_ROOK)
        printf("| R ");
      else if (pieces[make_square(f, r)] == W_QUEEN)
        printf("| Q ");
      else if (pieces[make_square(f, r)] == W_KING)
        printf("| K ");
      else if (pieces[make_square(f, r)] == B_PAWN)
        printf("| p ");
      else if (pieces[make_square(f, r)] == B_KNIGHT)
        printf("| n ");
      else if (pieces[make_square(f, r)] == B_BISHOP)
        printf("| b ");
      else if (pieces[make_square(f, r)] == B_ROOK)
        printf("| r ");
      else if (pieces[make_square(f, r)] == B_QUEEN)
        printf("| q ");
      else if (pieces[make_square(f, r)] == B_KING)
        printf("| k ");
      else
        printf("|   ");
    }
    printf("| %d\n+---+---+---+---+---+---+---+---+\n", 1 + r);
  }
  printf("  a   b   c   d   e   f   g   h\n\n");
  if (pos->passant != SQ_NONE)
    printf("En Passant Square: %c%c\n",file_of(pos->passant)+'a',rank_of(pos->passant)+'1');
  else
    puts("No En Passant Square");
  printf("Castling: %s", strings[pos->castling]);
  printf("\nFifty Move Rule: %d\n", pos->rule);
  printf("Move Number: %d\n\n",pos->ply);
}

void reset_pos(Position *pos) {
  pos->occupied[0] = 0x0ULL;
  pos->occupied[1] = 0x0ULL;
  pos->passant = SQ_NONE;
  pos->castling = 0;
  pos->side = WHITE;
  pos->rule = 0;
  pos->ply = 0;
  pos->pawns[0] = 0x0ULL;
  pos->pawns[1] = 0x0ULL;
  for (int a = 0; a < 16; ++a) {
    pos->count[a] = 0;
    for (int b = 0; b < 10; ++b)
      pos->lists[a][b] = SQ_NONE;
  }
  update_key(pos);
}

void parse_fen(Position *pos, char *fen) {
  reset_pos(pos);
  int file = FILE_A;
  int rank = RANK_8;
  int piece;
  int count;
  while (rank >= RANK_1) {
    count = 1;
    switch (*fen) {
      case 'p': piece = B_PAWN; break;
      case 'r': piece = B_ROOK; break;
      case 'n': piece = B_KNIGHT; break;
      case 'b': piece = B_BISHOP; break;
      case 'k': piece = B_KING; break;
      case 'q': piece = B_QUEEN; break;
      case 'P': piece = W_PAWN; break;
      case 'R': piece = W_ROOK; break;
      case 'N': piece = W_KNIGHT; break;
      case 'B': piece = W_BISHOP; break;
      case 'K': piece = W_KING; break;
      case 'Q': piece = W_QUEEN; break;
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
        piece = -1;
        count = *fen - '0';
        break;
      case '/':
      case ' ':
        rank--;
        file = FILE_A;
        ++fen;
        continue;
    }
		for (int i = 0; i < count; i++) {			
      int sq = make_square(file, rank);
      if (piece != -1) {
        pos->lists[piece][pos->count[piece]] = sq;
        pos->occupied[color_of(piece)] |= SquareBB[sq];
        ++pos->count[piece];
        if (type_of_p(piece) == PAWN) {
          pos->pawns[color_of(piece)] |= SquareBB[sq];
        }
      }
			file++;
    }
		++fen;
  }
  pos->side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;
	for (int i = 0; i < 4; i++) {
    if (*fen == ' ') {
      break;
    }		
		switch (*fen) {
			case 'K': pos->castling |= WHITE_OO; break;
			case 'Q': pos->castling |= WHITE_OOO; break;
			case 'k': pos->castling |= BLACK_OO; break;
			case 'q': pos->castling |= BLACK_OOO; break;
    }
		++fen;
	}
	++fen;
	if (*fen != '-') {        
		file = *fen - 'a';
    ++fen;
		rank = *fen - '1';
    ++fen;
		pos->passant = make_square(file, rank);
    --fen;
  }
  fen += 2;
  pos->rule = *fen - '0';
  ++fen;
  if (*fen != ' ') {
    pos->rule *= 10;
    pos->rule += *fen - '0';
    ++fen;
  }
  ++fen;
  pos->ply = *fen - '0';
  ++fen;
  while (*fen) {
    pos->ply *= 10;
    pos->ply += *fen - '0';
    ++fen;
  }
  pos->ply *= 2;
  pos->ply -= 2;
  pos->ply += pos->side;
  update_key(pos);
}

void do_move(Position *pos, int move) {
  int to = to_sq(move);
  int from = from_sq(move);
  int type = type_of_m(move);
  int promote = make_piece(pos->side, promotion_type(move));
  for (int t = PAWN; t <= KING; ++t) {
    int piece = make_piece(pos->side, t);
    int enemy = make_piece(pos->side == WHITE ? BLACK : WHITE, t);
    for (int c = 0; c < pos->count[piece]; ++c) {
      if (pos->lists[piece][c] == from) {
        pos->lists[piece][c] = to;
        if (t == PAWN)
          pos->rule = -1;
        for (int p = PAWN; p <= KING; ++p) {
          for (int i = 0; i < pos->count[make_piece(pos->side == WHITE ? BLACK : WHITE, p)]; ++i) {
            int sq = pos->lists[make_piece((pos->side == WHITE ? BLACK : WHITE), p)][i];
            if (sq == to) {
              pos->lists[make_piece((pos->side == WHITE ? BLACK : WHITE), p)][i] = pos->count[make_piece(pos->side == WHITE ? BLACK : WHITE, p)];
              pos->rule = -1;
            }
          }
        }
        if (type == ENPASSANT && to == pos->passant) {
          for (int c = 0; c < pos->count[piece]; ++c)
            if (pos->lists[enemy][c] == (pos->side == WHITE ? pos->passant - 8 : pos->passant + 8)) {
              pos->lists[enemy][c] = pos->lists[enemy][pos->count[enemy]-1];
              pos->lists[enemy][c] = SQ_NONE;
              break;
            }
        }
        if (type == PROMOTION) {
          pos->lists[piece][c] = pos->lists[piece][pos->count[piece]-1];
          pos->lists[piece][pos->count[piece]-1] = SQ_NONE;
          --pos->count[piece];
          pos->lists[promote][pos->count[promote]] = to;
          ++pos->count[promote];
        }
        if (t == PAWN && to - from == 16)
          pos->passant = from + 8;
        else if (t == PAWN && from - to == 16)
          pos->passant = to + 8;
        else
          pos->passant = SQ_NONE;
        if (t == KING && from == SQ_E1 && to == SQ_G1 && type == CASTLING)
          for (int c = 0; c < pos->count[W_ROOK]; ++c)
            if (pos->lists[W_ROOK][c] == SQ_H1)
              pos->lists[W_ROOK][c] = SQ_F1;
        if (t == KING && from == SQ_E1 && to == SQ_C1 && type == CASTLING)
          for (int c = 0; c < pos->count[W_ROOK]; ++c)
            if (pos->lists[W_ROOK][c] == SQ_A1)
              pos->lists[W_ROOK][c] = SQ_D1;
        if (t == KING && from == SQ_E8 && to == SQ_G8 && type == CASTLING)
          for (int c = 0; c < pos->count[B_ROOK]; ++c)
            if (pos->lists[B_ROOK][c] == SQ_H8)
              pos->lists[B_ROOK][c] = SQ_F8;
        if (t == KING && from == SQ_E8 && to == SQ_C8 && type == CASTLING)
          for (int c = 0; c < pos->count[W_ROOK]; ++c)
            if (pos->lists[B_ROOK][c] == SQ_A8)
              pos->lists[B_ROOK][c] = SQ_D8;
        if (t == KING) {
          pos->castling &= pos->side == WHITE ? BLACK_OO | BLACK_OOO : WHITE_OO | WHITE_OOO;
        }
        if (t == ROOK && from == SQ_A1) {
          pos->castling &= pos->side == WHITE ? ~WHITE_OOO : ~BLACK_OOO;
        }
        if (t == ROOK && from == SQ_H1) {
          pos->castling &= pos->side == WHITE ? ~WHITE_OO : ~BLACK_OO;
        }
        pos->side = pos->side == WHITE ? BLACK : WHITE;
        ++pos->ply;
        ++pos->rule;
        return;
      }
    }
  }
}

bool move_attacked(Position *pos, int from, int to, int color) {
  for (int p = PAWN; p <= KING; ++p)
    for (int c = 0; c < pos->count[make_piece(color, p)]; ++c)
      if (attacks_bb(make_piece(color, p), pos->lists[make_piece(color, p)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK] | SquareBB[to]) & ~SquareBB[from])) & SquareBB[pos->lists[make_piece(pos->side, KING)][0]])
        return true;
  return false;
}

bool sq_attacked(Position *pos, int sq, int color) {
  for (int p = PAWN; p <= KING; ++p)
    for (int c = 0; c < pos->count[make_piece(color, p)]; ++c)
      if (attacks_bb(make_piece(color, p), pos->lists[make_piece(color, p)][c], (pos->occupied[WHITE] | pos->occupied[BLACK])) & SquareBB[sq])
        return true;
  return false;
}

int move_pinned(Position *pos, int from, int to, int color) {
  if (!pos->count[W_KING] || !pos->count[B_KING])
    return SQ_NONE;
  for (int c = 0; c < pos->count[make_piece(~color & 1, BISHOP)]; ++c)
    if ((attacks_bb(make_piece(~color & 1, BISHOP), pos->lists[make_piece(~color & 1, BISHOP)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK]) & ~SquareBB[from]))) & SquareBB[pos->lists[make_piece(color, KING)][0]])
      if (!((attacks_bb(make_piece(~color & 1, BISHOP), pos->lists[make_piece(~color & 1, BISHOP)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK] | SquareBB[to]) & ~SquareBB[from]))) & SquareBB[pos->lists[make_piece(color, KING)][0]]))
        return pos->lists[make_piece(~color & 1, BISHOP)][c];
  for (int c = 0; c < pos->count[make_piece(~color & 1, ROOK)]; ++c)
    if ((attacks_bb(make_piece(~color & 1, ROOK), pos->lists[make_piece(~color & 1, ROOK)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK]) & ~SquareBB[from]))) & SquareBB[pos->lists[make_piece(color, KING)][0]])
      if (!((attacks_bb(make_piece(~color & 1, ROOK), pos->lists[make_piece(~color & 1, ROOK)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK] | SquareBB[to]) & ~SquareBB[from]))) & SquareBB[pos->lists[make_piece(color, KING)][0]]))
        return pos->lists[make_piece(~color & 1, ROOK)][c];
  for (int c = 0; c < pos->count[make_piece(~color & 1, QUEEN)]; ++c)
    if ((attacks_bb(make_piece(~color & 1, QUEEN), pos->lists[make_piece(~color & 1, QUEEN)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK]) & ~SquareBB[from]))) & SquareBB[pos->lists[make_piece(color, KING)][0]])
      if (!((attacks_bb(make_piece(~color & 1, QUEEN), pos->lists[make_piece(~color & 1, QUEEN)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK] | SquareBB[to]) & ~SquareBB[from]))) & SquareBB[pos->lists[make_piece(color, KING)][0]]))
        return pos->lists[make_piece(~color & 1, QUEEN)][c];
  return SQ_NONE;
}

int sq_pinned(Position *pos, int sq, int color) {
  if (!pos->count[W_KING] || !pos->count[B_KING])
    return SQ_NONE;
  for (int c = 0; c < pos->count[make_piece(~color & 1, BISHOP)]; ++c)
    if ((attacks_bb(make_piece(~color & 1, BISHOP), pos->lists[make_piece(~color & 1, BISHOP)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK]) & ~SquareBB[sq]))) & SquareBB[pos->lists[make_piece(color, KING)][0]])
      return pos->lists[make_piece(~color & 1, BISHOP)][c];
  for (int c = 0; c < pos->count[make_piece(~color & 1, ROOK)]; ++c)
    if ((attacks_bb(make_piece(~color & 1, ROOK), pos->lists[make_piece(~color & 1, ROOK)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK]) & ~SquareBB[sq]))) & SquareBB[pos->lists[make_piece(color, KING)][0]])
      return pos->lists[make_piece(~color & 1, ROOK)][c];
  for (int c = 0; c < pos->count[make_piece(~color & 1, QUEEN)]; ++c)
    if ((attacks_bb(make_piece(~color & 1, QUEEN), pos->lists[make_piece(~color & 1, QUEEN)][c], ((pos->occupied[WHITE] | pos->occupied[BLACK]) & ~SquareBB[sq]))) & SquareBB[pos->lists[make_piece(color, KING)][0]])
      return pos->lists[make_piece(~color & 1, QUEEN)][c];
  return SQ_NONE;
}