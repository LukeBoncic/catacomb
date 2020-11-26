#include <stdio.h>

#include "movegen.h"
#include "position.h"

void add_move(Position *pos, Movelist *list, bool check, int move) {
  if (sq_pinned(pos, from_sq(move), pos->side) == SQ_NONE) {
    if (check) {
      if (!move_attacked(pos, from_sq(move), to_sq(move), ~pos->side & 1)) {
        list->moves[list->count] = move;
        list->scores[list->count] = 0;
        ++list->count;
      }
    }
    else {
      list->moves[list->count] = move;
      list->scores[list->count] = 0;
      ++list->count;
    }
  }
  else if (sq_pinned(pos, from_sq(move), pos->side) == move_pinned(pos, from_sq(move), to_sq(move), pos->side)) {
    if (check) {
      if (!move_attacked(pos, from_sq(move), to_sq(move), ~pos->side & 1)) {
        list->moves[list->count] = move;
        list->scores[list->count] = 0;
        ++list->count;
      }
    }
    else {
      list->moves[list->count] = move;
      list->scores[list->count] = 0;
      ++list->count;
    }
  }
  else if (sq_pinned(pos, from_sq(move), pos->side) == to_sq(move)) {
    if (check) {
      if (!move_attacked(pos, from_sq(move), to_sq(move), ~pos->side & 1)) {
        list->moves[list->count] = move;
        list->scores[list->count] = 0;
        ++list->count;
      }
    }
    else {
      list->moves[list->count] = move;
      list->scores[list->count] = 0;
      ++list->count;
    }
  }
}

void add_castling(Position *pos, Movelist *list, bool check, int move) {
  if (!check)
    list->moves[list->count] = move;
    list->scores[list->count] = 0;
    ++list->count;
}

void add_pawn_move(Position *pos, Movelist *list, bool check, int from, int to) {
  if (pos->side == WHITE && rank_of(from) == RANK_7) {
    add_move(pos, list, check, make_promotion(from, to, KNIGHT));
    add_move(pos, list, check, make_promotion(from, to, BISHOP));
    add_move(pos, list, check, make_promotion(from, to, ROOK));
    add_move(pos, list, check, make_promotion(from, to, QUEEN));
  }
  else if (pos->side == BLACK && rank_of(from) == RANK_2) {
    add_move(pos, list, check, make_promotion(from, to, KNIGHT));
    add_move(pos, list, check, make_promotion(from, to, BISHOP));
    add_move(pos, list, check, make_promotion(from, to, ROOK));
    add_move(pos, list, check, make_promotion(from, to, QUEEN));
  }
  else
    add_move(pos, list, check, make_move(from, to));
}

void generate_all_moves(Position *pos, Movelist *list) {
  int from;
  int to;
  int pin;
  int enemy = ~pos->side & 1;
  int castled = pos->side * RANK_8;
  bool check = sq_attacked(pos, pos->lists[make_piece(pos->side, KING)][0], enemy);
  Bitboard moves;
  Bitboard occupied;
  list->count = 0;
  for (int pt = PAWN; pt <= KING; ++pt)
    for (int c = 0; c < pos->count[make_piece(pos->side, pt)]; ++c) {
      from = pos->lists[make_piece(pos->side, pt)][c];
      occupied = pos->occupied[WHITE] | pos->occupied[BLACK];
      if (pt == PAWN)
        occupied = pos->occupied[enemy];
      moves = attacks_bb(make_piece(pos->side, pt), from, occupied);
      while (moves) {
        to = pop_lsb(&moves);
        if (pt == PAWN) {
          if (SquareBB[to] & occupied)
            add_pawn_move(pos, list, check, from, to);
          else if (to == pos->passant)
            add_move(pos, list, check, make_enpassant(from, to));
        }
        else if (pt == KING) {
          if (!sq_attacked(pos, to, enemy) && SquareBB[to] & ~pos->occupied[pos->side])
            add_move(pos, list, check, make_move(from, to));
        }
        else if (SquareBB[to] & ~pos->occupied[pos->side])
          add_move(pos, list, check, make_move(from, to));
      }
      if (pt == PAWN) {
        to = from + (pos->side == WHITE ? 8 : -8);
        occupied = pos->occupied[WHITE] | pos->occupied[BLACK];
        if (SquareBB[to] & ~occupied) {
          add_pawn_move(pos, list, check, from, to);
          to += (pos->side == WHITE ? 8 : -8);
          if ((SquareBB[to] & ~occupied) && pos->side == WHITE && (rank_of(from) == RANK_2) )
            add_pawn_move(pos, list, check, from, to);
          if ((SquareBB[to] & ~occupied) && pos->side == BLACK && (rank_of(from) == RANK_7))
            add_pawn_move(pos, list, check, from, to);
        }
      }
      if (pt == KING) {
        if (pos->castling & make_castling_right(pos->side, KING_SIDE))
          if (!sq_attacked(pos, make_square(FILE_F, castled), enemy))
            if (!sq_attacked(pos, make_square(FILE_G, castled), enemy))
              if (!(SquareBB[make_square(FILE_F, castled)] & (pos->occupied[WHITE] | pos->occupied[BLACK])))
                if (!(SquareBB[make_square(FILE_G, castled)] & (pos->occupied[WHITE] | pos->occupied[BLACK])))
                  add_castling(pos, list, check, make_castling(make_square(FILE_E, castled), make_square(FILE_G, castled)));
        if (pos->castling & make_castling_right(pos->side, QUEEN_SIDE))
          if (!sq_attacked(pos, make_square(FILE_D, castled), enemy))
            if (!sq_attacked(pos, make_square(FILE_C, castled), enemy))
              if (!(SquareBB[make_square(FILE_D, castled)] & (pos->occupied[WHITE] | pos->occupied[BLACK])))
                if (!(SquareBB[make_square(FILE_C, castled)] & (pos->occupied[WHITE] | pos->occupied[BLACK])))
                  if (!(SquareBB[make_square(FILE_B, castled)] & (pos->occupied[WHITE] | pos->occupied[BLACK])))
                    add_castling(pos, list, check, make_castling(make_square(FILE_E, castled), make_square(FILE_C, castled)));
      }
    }
}

void movelist_pretty(Movelist *list) {
  char charmove[5];
  for (int i = 0; i < list->count; ++i) {
    charmove[0] = 'a' + file_of(from_sq(list->moves[i]));
    charmove[1] = '1' + rank_of(from_sq(list->moves[i]));
    charmove[2] = 'a' + file_of(to_sq(list->moves[i]));
    charmove[3] = '1' + rank_of(to_sq(list->moves[i]));
    if (type_of_m(list->moves[i]) == PROMOTION) {
      if (promotion_type(list->moves[i]) == KNIGHT)
        printf("%sn\n",charmove);
      if (promotion_type(list->moves[i]) == BISHOP)
        printf("%sb\n",charmove);
      if (promotion_type(list->moves[i]) == ROOK)
        printf("%sr\n",charmove);
      if (promotion_type(list->moves[i]) == QUEEN)
        printf("%sq\n",charmove);
    }
    else
      printf("%s\n",charmove);
  }
}