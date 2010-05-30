/*
 *
 */
#ifndef _EINSTEIN_H
#define _EINSTEIN_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* return random innteger in [0,max) interval */
unsigned int
irand( unsigned int max );

/* holds values of solved puzzles */
typedef uint_fast8_t cell_t;

/* holds bitmasks of possible values */
typedef uint_fast32_t try_cell_t;

/* puzzle board */
typedef struct puzzle_s {
	size_t rows;
	size_t cols;
	cell_t board[ 0 ];
} puzzle_t;

/* try board */
typedef struct try_s {
	size_t rows;
	size_t cols;
	try_cell_t board[ 0 ];
} try_t;

/* select one board cell */
/* use macro because I want to be able to assign to it */
#define CELL( s, r, c ) ((s)->board[ r * (s)->cols + c ])

#define TRY_MASK( el ) ( ((try_cell_t)1UL) << (el) )

/* can an element be here */
static inline bool
try_is_possible( const try_t *t, size_t col, size_t row, cell_t el )
{
	return CELL( t, row, col ) & TRY_MASK( el );
}

/* return true if only one element in cell is set */
static inline bool
try_is_defined( const try_t *t, size_t col, size_t row )
{
	try_cell_t c = CELL( t, row, col );

	/* must not be 0 */
	assert( c );

	/* true if only one bit is set */
	return ( c & ( c - 1 ) ) == 0;
}

/* exclude some element from cell */
bool
try_exclude( try_t *t, size_t col, size_t row, cell_t el );

/* set some element in cell */
bool
try_set( try_t *t, size_t col, size_t row, cell_t el );

/* alloc and prepare try board */
try_t *
try_new( const puzzle_t *p );

/* are all elements unique ? */
bool
try_is_solved( const try_t *t );

int
try_find( const try_t *t, size_t row, cell_t el );

void
try_print( const try_t *t );


/* base rule definition */
typedef struct rule_s rule_t;

#define RULE_STRUCT_BASE \
	bool (*apply)( const rule_t *r, try_t *t ); \
	int (*get)( const rule_t *r, char *buf ); \
	bool (*check)( const rule_t *r, const try_t *t ); \
	rule_t *next;

struct rule_s
{
	RULE_STRUCT_BASE
	size_t data[0];
};

/* create new random rule */
rule_t *
rule_new( const puzzle_t *p );

void
rule_print( const rule_t *r );

/* init try board from open rules */
void
try_rule_init( try_t *t, const rule_t *r );

typedef struct game_s {
	puzzle_t *p;
	rule_t *r;
	try_t *t;
} game_t;

game_t *
game_gen( size_t cols, size_t rows );

void
game_print( const game_t *g );

void
game_free( game_t *g );

bool
puzzle_try_is_valid( const puzzle_t *p, const try_t *t );

#endif

/* vim: ts=4:sw=4:fdm=marker
 */
