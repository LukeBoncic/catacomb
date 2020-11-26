#ifndef BITBOARD_H_INCLUDED
#define BITBOARD_H_INCLUDED

#include <assert.h>

#include "types.h"

void bitboards_init();
void bitboards_pretty(Bitboard b);

#define AllSquares   0xFFFFFFFFFFFFFFFFULL
#define DarkSquares  0xAA55AA55AA55AA55ULL
#define LightSquares 0x55AA55AA55AA55AAULL

#define FileABB 0x0101010101010101ULL
#define FileBBB (FileABB << 1)
#define FileCBB (FileABB << 2)
#define FileDBB (FileABB << 3)
#define FileEBB (FileABB << 4)
#define FileFBB (FileABB << 5)
#define FileGBB (FileABB << 6)
#define FileHBB (FileABB << 7)

#define Rank1BB 0xFFULL
#define Rank2BB (Rank1BB << (8 * 1))
#define Rank3BB (Rank1BB << (8 * 2))
#define Rank4BB (Rank1BB << (8 * 3))
#define Rank5BB (Rank1BB << (8 * 4))
#define Rank6BB (Rank1BB << (8 * 5))
#define Rank7BB (Rank1BB << (8 * 6))
#define Rank8BB (Rank1BB << (8 * 7))

#define QueenSide   (FileABB | FileBBB | FileCBB | FileDBB)
#define CenterFiles (FileCBB | FileDBB | FileEBB | FileFBB)
#define KingSide    (FileEBB | FileFBB | FileGBB | FileHBB)
#define Center      ((FileDBB | FileEBB) & (Rank4BB | Rank5BB))

extern uint8_t SquareDistance[64][64];

extern Bitboard SquareBB[64];
extern Bitboard FileBB[8];
extern Bitboard RankBB[8];
extern Bitboard ForwardRanksBB[2][8];
extern Bitboard BetweenBB[64][64];
extern Bitboard LineBB[64][64];
extern Bitboard DistanceRingBB[64][8];
extern Bitboard ForwardFileBB[2][64];
extern Bitboard PassedPawnSpan[2][64];
extern Bitboard PawnAttackSpan[2][64];
extern Bitboard PseudoAttacks[8][64];
extern Bitboard PawnAttacks[2][64];

extern Bitboard RookMasks[64];
extern Bitboard RookMagics[64];
extern uint8_t  RookShifts[64];
extern Bitboard BishopMasks[64];
extern Bitboard BishopMagics[64];
extern uint8_t  BishopShifts[64];
extern Bitboard *RookAttacks[64];
extern Bitboard *BishopAttacks[64];

// attacks_bb() returns a bitboard representing all the squares attacked
// by a // piece of type Pt (bishop or rook) placed on 's'. The helper magic_index() looks up the index using the 'magic bitboards' approach.

INLINE unsigned magic_index_bishop(Square s, Bitboard occupied)
{
  if (Is64Bit)
      return (unsigned)(((occupied & BishopMasks[s]) * BishopMagics[s])
                           >> BishopShifts[s]);

  unsigned lo = (unsigned)(occupied) & (unsigned)(BishopMasks[s]);
  unsigned hi = (unsigned)(occupied >> 32) & (unsigned)(BishopMasks[s] >> 32);
  return (lo * (unsigned)(BishopMagics[s]) ^ hi * (unsigned)(BishopMagics[s] >> 32)) >> BishopShifts[s];
}

INLINE unsigned magic_index_rook(Square s, Bitboard occupied)
{
  if (Is64Bit)
      return (unsigned)(((occupied & RookMasks[s]) * RookMagics[s])
                           >> RookShifts[s]);

  unsigned lo = (unsigned)(occupied) & (unsigned)(RookMasks[s]);
  unsigned hi = (unsigned)(occupied >> 32) & (unsigned)(RookMasks[s] >> 32);
  return (lo * (unsigned)(RookMagics[s]) ^ hi * (unsigned)(RookMagics[s] >> 32)) >> RookShifts[s];
}

INLINE Bitboard attacks_bb_bishop(int s, Bitboard occupied)
{
  return BishopAttacks[s][magic_index_bishop(s, occupied)];
}

INLINE Bitboard attacks_bb_rook(int s, Bitboard occupied)
{
  return RookAttacks[s][magic_index_rook(s, occupied)];
}

INLINE __attribute__((pure)) Bitboard sq_bb(Square s)
{
  return SquareBB[s];
}

#if __x86_64__
INLINE Bitboard inv_sq(Bitboard b, Square s)
{
  __asm__("btcq %1, %0" : "+r" (b) : "r" ((uint64_t)s) : "cc");
  return b;
}
#else
INLINE Bitboard inv_sq(Bitboard b, Square s)
{
  return b ^ sq_bb(s);
}
#endif

INLINE bool more_than_one(Bitboard b)
{
  return b & (b - 1);
}


// rank_bb() and file_bb() return a bitboard representing all the squares on
// the given file or rank.

INLINE Bitboard rank_bb(Rank r)
{
  return RankBB[r];
}

INLINE Bitboard rank_bb_s(Square s)
{
  return RankBB[rank_of(s)];
}

INLINE Bitboard file_bb(File f)
{
  return FileBB[f];
}

INLINE Bitboard file_bb_s(Square s)
{
  return FileBB[file_of(s)];
}


// shift_bb() moves a bitboard one step along direction Direction.
INLINE Bitboard shift_bb(int Direction, Bitboard b)
{
  return  Direction == NORTH       ?  b << 8
        : Direction == SOUTH       ?  b >> 8
        : Direction == NORTH+NORTH ?  b << 16
        : Direction == SOUTH+SOUTH ?  b >> 16
        : Direction == EAST        ? (b & ~FileHBB) << 1
        : Direction == WEST        ? (b & ~FileABB) >> 1
        : Direction == NORTH_EAST  ? (b & ~FileHBB) << 9
        : Direction == SOUTH_EAST  ? (b & ~FileHBB) >> 7
        : Direction == NORTH_WEST  ? (b & ~FileABB) << 7
        : Direction == SOUTH_WEST  ? (b & ~FileABB) >> 9
        : 0;
}


// pawn_attacks_bb() returns the squares attacked by pawns of the given color
// from the squares in the given bitboard.

INLINE Bitboard pawn_attacks_bb(Bitboard b, const Color C)
{
  return C == WHITE ? shift_bb(NORTH_WEST, b) | shift_bb(NORTH_EAST, b)
                    : shift_bb(SOUTH_WEST, b) | shift_bb(SOUTH_EAST, b);
}


// pawn_double_attacks_bb() returns the pawn attacks for the given color
// from the squares in the given bitboard.

INLINE Bitboard pawn_double_attacks_bb(Bitboard b, const Color C)
{
  return C == WHITE ? shift_bb(NORTH_WEST, b) & shift_bb(NORTH_EAST, b)
                    : shift_bb(SOUTH_WEST, b) & shift_bb(SOUTH_EAST, b);
}


// adjacent_files_bb() returns a bitboard representing all the squares
// on the adjacent files of the given one.

INLINE Bitboard adjacent_files_bb(unsigned f)
{
  return shift_bb(EAST, FileBB[f]) | shift_bb(WEST, FileBB[f]);
}


// between_bb() returns a bitboard representing all the squares between
// the two given ones. For instance, between_bb(SQ_C4, SQ_F7) returns a
// bitboard with the bits for square d5 and e6 set. If s1 and s2 are not
// on the same rank, file or diagonal, 0 is returned.

INLINE Bitboard between_bb(Square s1, Square s2)
{
  return BetweenBB[s1][s2];
}


// forward_ranks_bb() returns a bitboard representing all the squares on
// all the ranks in front of the given one, from the point of view of the
// given color. For instance, forward_ranks_bb(BLACK, RANK_3) will return
// the squares on ranks 1 and 2.

INLINE Bitboard forward_ranks_bb(Color c, unsigned r)
{
  return ForwardRanksBB[c][r];
}


// forward_file_bb() returns a bitboard representing all the squares
// along the line in front of the given one, from the point of view of
// the given color:
//     ForwardFileBB[c][s] = forward_ranks_bb(c, rank_of(s)) & file_bb(s)

INLINE Bitboard forward_file_bb(Color c, Square s)
{
  return ForwardFileBB[c][s];
}


// pawn_attack_span() returns a bitboard representing all the squares
// that can be attacked by a pawn of the given color when it moves along
// its file, starting from the given square:
//     PawnAttackSpan[c][s] = forward_ranks_bb(c, rank_of(s)) & adjacent_files_bb(s);

INLINE Bitboard pawn_attack_span(Color c, Square s)
{
  return PawnAttackSpan[c][s];
}


// passed_pawn_span() returns a bitboard mask which can be used to test
// if a pawn of the given color and on the given square is a passed pawn:
//     PassedPawnSpan[c][s] = pawn_attack_span(c, s) | forward_bb(c, s)

INLINE Bitboard passed_pawn_span(Color c, Square s)
{
  return PassedPawnSpan[c][s];
}


// aligned() returns true if square s is on the line determined by move m.

INLINE uint64_t aligned(Move m, Square s)
{
  return ((Bitboard *)LineBB)[m & 4095] & sq_bb(s);
}


// distance() functions return the distance between x and y, defined as
// the number of steps for a king in x to reach y. Works with squares,
// ranks, files.

INLINE int distance(Square x, Square y)
{
  return SquareDistance[x][y];
}

INLINE unsigned distance_f(Square x, Square y)
{
  unsigned f1 = file_of(x), f2 = file_of(y);
  return f1 < f2 ? f2 - f1 : f1 - f2;
}

INLINE unsigned distance_r(Square x, Square y)
{
  unsigned r1 = rank_of(x), r2 = rank_of(y);
  return r1 < r2 ? r2 - r1 : r1 - r2;
}

#define attacks_bb_queen(s, occupied) (attacks_bb_bishop((s), (occupied)) | attacks_bb_rook((s), (occupied)))

INLINE Bitboard attacks_bb(int p, Square s, Bitboard occupied)
{
  switch (type_of_p(p)) {
  case BISHOP:
      return attacks_bb_bishop(s, occupied);
  case ROOK:
      return attacks_bb_rook(s, occupied);
  case QUEEN:
      return attacks_bb_queen(s, occupied);
  case PAWN:
      return PawnAttacks[color_of(p)][s];
  default:
      return PseudoAttacks[type_of_p(p)][s];
  }
}


// popcount() counts the number of non-zero bits in a bitboard.

INLINE int popcount(Bitboard b)
{
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)

  return (int)_mm_popcnt_u64(b);

#else // Assumed gcc or compatible compiler

  return __builtin_popcountll(b);

#endif
}


// lsb() and msb() return the least/most significant bit in a non-zero
// bitboard.

#if defined(__GNUC__)

INLINE int lsb(Bitboard b)
{
  assert(b);
  return __builtin_ctzll(b);
}

INLINE int msb(Bitboard b)
{
  assert(b);
  return 63 ^ __builtin_clzll(b);
}

#elif defined(_MSC_VER)

#if defined(_WIN64)

INLINE Square lsb(Bitboard b)
{
  assert(b);
  unsigned long idx;
  _BitScanForward64(&idx, b);
  return (Square) idx;
}

INLINE Square msb(Bitboard b)
{
  assert(b);
  unsigned long idx;
  _BitScanReverse64(&idx, b);
  return (Square) idx;
}

#else

INLINE Square lsb(Bitboard b)
{
  assert(b);
  unsigned long idx;
  if ((uint32_t)b) {
    _BitScanForward(&idx, (uint32_t)b);
    return idx;
  } else {
    _BitScanForward(&idx, (uint32_t)(b >> 32));
    return idx + 32;
  }
}

INLINE Square msb(Bitboard b)
{
  assert(b);
  unsigned long idx;
  if (b >> 32) {
    _BitScanReverse(&idx, (uint32_t)(b >> 32));
    return idx + 32;
  } else {
    _BitScanReverse(&idx, (uint32_t)b);
    return idx;
  }
}

#endif

#else

#error "Compiler not supported."

#endif


// pop_lsb() finds and clears the least significant bit in a non-zero
// bitboard.

INLINE Square pop_lsb(Bitboard* b)
{
  const Square s = lsb(*b);
  *b &= *b - 1;
  return s;
}


// frontmost_sq() and backmost_sq() return the square corresponding to the
// most/least advanced bit relative to the given color.

INLINE Square frontmost_sq(Color c, Bitboard b)
{
  return c == WHITE ? msb(b) : lsb(b);
}

INLINE Square  backmost_sq(Color c, Bitboard b)
{
  return c == WHITE ? lsb(b) : msb(b);
}

#endif