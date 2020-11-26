#include <stdio.h>

#include "bitboard.h"

uint8_t SquareDistance[64][64];
Bitboard SquareBB[64];
Bitboard FileBB[8];
Bitboard RankBB[8];
Bitboard ForwardRanksBB[2][8];
Bitboard BetweenBB[64][64];
Bitboard LineBB[64][64];
Bitboard DistanceRingBB[64][8];
Bitboard ForwardFileBB[2][64];
Bitboard PassedPawnSpan[2][64];
Bitboard PawnAttackSpan[2][64];
Bitboard PseudoAttacks[8][64];
Bitboard PawnAttacks[2][64];

Bitboard  RookMasks  [64];
Bitboard  RookMagics [64];
Bitboard *RookAttacks[64];
uint8_t   RookShifts [64];

Bitboard  BishopMasks  [64];
Bitboard  BishopMagics [64];
Bitboard *BishopAttacks[64];
uint8_t   BishopShifts [64];

static Bitboard RookTable[0x19000];
static Bitboard BishopTable[0x1480];

static int RookDirs[] = { NORTH, EAST, SOUTH, WEST };
static int BishopDirs[] = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

static Bitboard sliding_attack(int dirs[], Square sq, Bitboard occupied)
{
  Bitboard attack = 0;

  for (int i = 0; i < 4; i++)
    for (Square s = sq + dirs[i];
         square_is_ok(s) && distance(s, s - dirs[i]) == 1; s += dirs[i])
    {
      attack |= sq_bb(s);
      if (occupied & sq_bb(s))
        break;
    }

  return attack;
}

void prng_init(Bitboard *rng, uint64_t seed)
{
  *rng = seed;
}

Bitboard prng_rand(Bitboard *rng)
{
  Bitboard s = *rng;

  s ^= s >> 12;
  s ^= s << 25;
  s ^= s >> 27;
  *rng = s;

  return s * 2685821657736338717LL;
}

Bitboard prng_sparse_rand(Bitboard *rng)
{
  Bitboard r1 = prng_rand(rng);
  Bitboard r2 = prng_rand(rng);
  Bitboard r3 = prng_rand(rng);
  return r1 & r2 & r3;
}

typedef unsigned (Fn)(Square, Bitboard);

static void init_magics(Bitboard table[], Bitboard *attacks[], Bitboard magics[], Bitboard masks[], uint8_t shifts[], int deltas[], Fn index) {
  int seeds[][8] = { { 8977, 44560, 54343, 38998,  5731, 95205, 104912, 17020 }, {  728, 10316, 55013, 32803, 12281, 15100,  16645,   255 } };
  Bitboard occupancy[4096], reference[4096], edges, b;
  int age[4096] = {0}, current = 0, i, size;
  attacks[0] = table;
  for (Square s = 0; s < 64; s++) {
    edges = ((Rank1BB | Rank8BB) & ~rank_bb_s(s)) | ((FileABB | FileHBB) & ~file_bb_s(s));
    masks[s]  = sliding_attack(deltas, s, 0) & ~edges;
    shifts[s] = (Is64Bit ? 64 : 32) - popcount(masks[s]);
    b = size = 0;
    do {
      occupancy[size] = b;
      reference[size] = sliding_attack(deltas, s, b);
      if (HasPext)
        attacks[s][pext(b, masks[s])] = reference[size];
      size++;
      b = (b - masks[s]) & masks[s];
    } while (b);
    if (s < 63)
      attacks[s + 1] = attacks[s] + size;
    if (HasPext)
      continue;
    Bitboard rng;
    prng_init(&rng, seeds[Is64Bit][rank_of(s)]);
    do {
      do
        magics[s] = prng_sparse_rand(&rng);
      while (popcount((magics[s] * masks[s]) >> 56) < 6);
      for (current++, i = 0; i < size; i++) {
        unsigned idx = index(s, occupancy[i]);
        if (age[idx] < current) {
          age[idx] = current;
          attacks[s][idx] = reference[i];
        }
        else if (attacks[s][idx] != reference[i])
          break;
      }
    } while (i < size);
  }
}

static void init_sliding_attacks(void)
{
  init_magics(RookTable, RookAttacks, RookMagics, RookMasks, RookShifts, RookDirs, magic_index_rook);
  init_magics(BishopTable, BishopAttacks, BishopMagics, BishopMasks, BishopShifts, BishopDirs, magic_index_bishop);
}

void bitboards_pretty(Bitboard b)
{
  printf("+---+---+---+---+---+---+---+---+\n");

  for (int r = 7; r >= 0; r--) {
    for (int f = 0; f <= 7; f++)
      printf((b & sq_bb(make_square(f, r))) ? "| X " : "|   ");

    printf("| %d\n+---+---+---+---+---+---+---+---+\n", 1 + r);
  }
  printf("  a   b   c   d   e   f   g   h\n");
}

// bitboards_init() initializes various bitboard tables. It is called at
// startup and relies on global objects to be already zero-initialized.

void bitboards_init(void) {

  for (Square s = 0; s < 64; s++)
    SquareBB[s] = 1ULL << s;

  for (int f = 0; f < 8; f++)
    FileBB[f] = f > FILE_A ? FileBB[f - 1] << 1 : FileABB;

  for (int r = 0; r < 8; r++)
    RankBB[r] = r > RANK_1 ? RankBB[r - 1] << 8 : Rank1BB;

  for (int r = 0; r < 7; r++)
    ForwardRanksBB[WHITE][r] = ~(ForwardRanksBB[BLACK][r + 1] = ForwardRanksBB[BLACK][r] | RankBB[r]);

  for (int c = 0; c < 2; c++)
    for (Square s = 0; s < 64; s++) {
      ForwardFileBB[c][s]  = ForwardRanksBB[c][rank_of(s)] & FileBB[file_of(s)];
      PawnAttackSpan[c][s] = ForwardRanksBB[c][rank_of(s)] & adjacent_files_bb(file_of(s));
      PassedPawnSpan[c][s] = ForwardFileBB[c][s] | PawnAttackSpan[c][s];
    }

  for (Square s1 = 0; s1 < 64; s1++)
    for (Square s2 = 0; s2 < 64; s2++)
      if (s1 != s2) {
        SquareDistance[s1][s2] = max(distance_f(s1, s2), distance_r(s1, s2));
        DistanceRingBB[s1][SquareDistance[s1][s2]] |= sq_bb(s2);
      }

  int steps[][5] = {
    {0}, { 7, 9 }, { 6, 10, 15, 17 }, {0}, {0}, {0}, { 1, 7, 8, 9 }
  };

  for (int c = 0; c < 2; c++)
    for (int pt = PAWN; pt <= KING; pt++)
      for (int s = 0; s < 64; s++)
        for (int i = 0; steps[pt][i]; i++) {
          Square to = s + (Square)(c == WHITE ? steps[pt][i] : -steps[pt][i]);

          if (square_is_ok(to) && distance(s, to) < 3) {
            if (pt == PAWN)
              PawnAttacks[c][s] |= sq_bb(to);
            else
              PseudoAttacks[pt][s] |= sq_bb(to);
          }
        }

  init_sliding_attacks();

  for (Square s1 = 0; s1 < 64; s1++) {
    PseudoAttacks[QUEEN][s1] = PseudoAttacks[BISHOP][s1] = attacks_bb_bishop(s1, 0);
    PseudoAttacks[QUEEN][s1] |= PseudoAttacks[ROOK][s1] = attacks_bb_rook(s1, 0);

    for (int pt = BISHOP; pt <= ROOK; pt++)
      for (Square s2 = 0; s2 < 64; s2++) {
        if (!(PseudoAttacks[pt][s1] & sq_bb(s2)))
          continue;

        LineBB[s1][s2] = (attacks_bb(pt, s1, 0) & attacks_bb(pt, s2, 0)) | sq_bb(s1) | sq_bb(s2);
        BetweenBB[s1][s2] = attacks_bb(pt, s1, SquareBB[s2]) & attacks_bb(pt, s2, SquareBB[s1]);
      }
  }
}