#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#ifndef NDEBUG
#include <assert.h>
#endif
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define INLINE static inline __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))

// Declaring pure functions as pure seems not to help. (Investigate later.)
//#define PURE __attribute__((pure))
#define PURE

#if defined __has_attribute
#if __has_attribute(minsize)
#define SMALL __attribute__((minsize))
#elif __has_attribute(optimize)
#define SMALL __attribute__((optimize("Os")))
#endif
#endif

#ifndef SMALL
#define SMALL
#endif

// Predefined macros hell:
//
// __GNUC__           Compiler is gcc, Clang or Intel on Linux
// __INTEL_COMPILER   Compiler is Intel
// _MSC_VER           Compiler is MSVC or Intel on Windows
// _WIN32             Building on Windows (any)
// _WIN64             Building on Windows 64 bit

#if defined(_WIN64) && defined(_MSC_VER) // No Makefile used
#  include <intrin.h> // Microsoft header for _BitScanForward64()
#  define IS_64BIT
#endif

#if defined(USE_POPCNT) && (defined(__INTEL_COMPILER) || defined(_MSC_VER))
#  include <nmmintrin.h> // Intel and Microsoft header for _mm_popcnt_u64()
#endif

#if !defined(NO_PREFETCH) && (defined(__INTEL_COMPILER) || defined(_MSC_VER))
#  include <xmmintrin.h> // Intel and Microsoft header for _mm_prefetch()
#endif

#if defined(USE_PEXT)
#  include <immintrin.h> // Header for _pext_u64() intrinsic
#  define pext(b, m) _pext_u64(b, m)
#else
#  define pext(b, m) (0)
#endif

#ifdef USE_POPCNT
#define HasPopCnt 1
#else
#define HasPopCnt 0
#endif

#ifdef USE_PEXT
#define HasPext 1
#else
#define HasPext 0
#endif

#ifdef IS_64BIT
#define Is64Bit 1
#else
#define Is64Bit 0
#endif

#ifdef NUMA
#define HasNuma 1
#else
#define HasNuma 0
#endif

typedef uint64_t Key;
typedef uint64_t Bitboard;

enum { MAX_MOVES = 256 };

// A move needs 16 bits to be stored
//
// bit  0- 5: destination square (from 0 to 63)
// bit  6-11: origin square (from 0 to 63)
// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
// NOTE: EN-PASSANT bit is set only when a pawn can be captured
//
// Null move (MOVE_NULL) is encoded as a2a2.

enum { MOVE_NONE = 0, MOVE_NULL = 65 };

enum { NORMAL, PROMOTION, ENPASSANT, CASTLING };

enum { WHITE, BLACK };

enum { KING_SIDE, QUEEN_SIDE };

enum {
  NO_CASTLING = 0, WHITE_OO = 1, WHITE_OOO = 2,
  BLACK_OO = 4, BLACK_OOO = 8, ANY_CASTLING = 15
};

INLINE int make_castling_right(int c, int s)
{
  return c == WHITE ? s == QUEEN_SIDE ? WHITE_OOO : WHITE_OO : s == QUEEN_SIDE ? BLACK_OOO : BLACK_OO;
}

enum { PHASE_ENDGAME = 0, PHASE_MIDGAME = 128 };
enum { MG, EG };

enum {
  SCALE_FACTOR_DRAW = 0, SCALE_FACTOR_NORMAL = 64,
  SCALE_FACTOR_MAX = 128, SCALE_FACTOR_NONE = 255
};

enum { BOUND_NONE, BOUND_UPPER, BOUND_LOWER, BOUND_EXACT };

enum {
  VALUE_ZERO = 0, VALUE_DRAW = 0,
  VALUE_KNOWN_WIN = 10000, VALUE_MATE = 32000,
  VALUE_INFINITE = 32001, VALUE_NONE = 32002
};
enum { PAWN = 1, KNIGHT, BISHOP, ROOK, QUEEN, KING };

enum {
  W_PAWN = 1, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
  B_PAWN = 9, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING
};

enum {
  DEPTH_QS_CHECKS     =  0,
  DEPTH_QS_NO_CHECKS  = -1,
  DEPTH_QS_RECAPTURES = -5,
  DEPTH_NONE = -6,
  DEPTH_OFFSET = -7
};

enum {
  SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
  SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
  SQ_NONE
};

enum {
  NORTH = 8, EAST = 1, SOUTH = -8, WEST = -1,
  NORTH_EAST = NORTH + EAST, SOUTH_EAST = SOUTH + EAST,
  NORTH_WEST = NORTH + WEST, SOUTH_WEST = SOUTH + WEST,
};

enum { FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H };

enum { RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8 };

typedef uint32_t Move;
typedef int32_t Phase;
typedef int32_t Value;
typedef bool Color;
typedef uint32_t Piece;
typedef uint32_t PieceType;
typedef int32_t Depth;
typedef uint32_t Square;
typedef uint32_t File;
typedef uint32_t Rank;
typedef uint32_t Square;

// Score type stores a middlegame and an endgame value in a single integer.
// The endgame value goes in the upper 16 bits, the middlegame value in
// the lower 16 bits.

typedef uint32_t Score;

enum { SCORE_ZERO };

#define make_score(mg,eg) ((((unsigned)(eg))<<16) + (mg))

// Casting an out-of-range value to int16_t is implementation-defined, but
// we assume the implementation does the right thing.
INLINE Value eg_value(Score s)
{
  return (int16_t)((s + 0x8000) >> 16);
}

INLINE Value mg_value(Score s)
{
  return (int16_t)s;
}

/// Division of a Score must be handled separately for each term
INLINE int score_divide(Score s, int i)
{
  return make_score(mg_value(s) / i, eg_value(s) / i);
}

#define square_flip(s) (sq ^ 0x38)
#define mate_in(ply) ((VALUE_MATE - (ply)))
#define mated_in(ply) ((-VALUE_MATE + (ply)))
#define make_square(f,r) ((((r) << 3) + (f)))
#define make_piece(c,pt) ((((c) << 3) + (pt)))
#define type_of_p(p) ((p) & 7)
#define color_of(p) ((p) >> 3)
#define square_is_ok(s) (s <= SQ_H8)
#define file_of(s) ((s) & 7)
#define rank_of(s) ((s) >> 3)
#define relative_square(c,s) (((s) ^ ((c) * 56)))
#define relative_rank(c,r) ((r) ^ ((c) * 7))
#define relative_rank_s(c,s) relative_rank(c,rank_of(s))
#define pawn_push(c) ((c) == WHITE ? 8 : -8)
#define from_sq(m) (((m)>>6) & 0x3f)
#define to_sq(m) (((m) & 0x3f))
#define from_to(m) ((m) & 0xfff)
#define type_of_m(m) ((m) >> 14)
#define promotion_type(m) ((((m)>>12) & 3) + KNIGHT)
#define make_move(from,to) (((to) | ((from) << 6)))
#define reverse_move(m) (make_move(to_sq(m), from_sq(m)))
#define make_promotion(from,to,pt) (((to) | ((from)<<6) | (PROMOTION<<14) | (((pt)-KNIGHT)<<12)))
#define make_enpassant(from,to) (((to) | ((from)<<6) | (ENPASSANT<<14)))
#define make_castling(from,to) (((to) | ((from)<<6) | (CASTLING<<14)))
#define move_is_ok(m) (from_sq(m) != to_sq(m))

INLINE bool opposite_colors(Square s1, Square s2)
{
  int s = s1 ^ s2;
  return ((s >> 3) ^ s) & 1;
}

#undef max
#undef min

#define max(a,b) a > b ? a : b
#define min(a,b) a < b ? a : b

#endif