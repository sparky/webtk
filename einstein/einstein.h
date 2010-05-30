
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


/* return random innteger in [0,max) interval */
unsigned long int
irand( unsigned long int max );

/* cell_t holds small integer values */
typedef unsigned char cell_t;

/* puzzle board */
typedef struct puzzle_s {
	cell_t rows;
	cell_t cols;
	cell_t board[ 0 ];
} puzzle_t;

/* try_cell_t holds bitmasks of possible values */
typedef unsigned int try_cell_t;

/* try board */
typedef struct try_s {
	cell_t rows;
	cell_t cols;
	try_cell_t board[ 0 ];
} try_t;

/* select one board cell */
#define CELL( s, r, c ) ((s)->board[ r * (s)->cols + c ])

/* can an element be here */
static inline int
try_is_possible( try_t *t, cell_t col, cell_t row, cell_t el )
{
	return CELL( t, row, col ) & ( 1 << el );
}

/* return true if only one element in cell is set */
static inline int
try_is_defined( try_t *t, cell_t col, cell_t row )
{
	try_cell_t c = CELL( t, row, col );

	/* must not be 0 */
	assert( c );

	/* true if only one bit is set */
	return ( c & ( c - 1 ) ) == 0;
}

/* exclude some element from cell */
int
try_exclude( try_t *t, cell_t col, cell_t row, cell_t el );

/* set some element in cell */
int
try_set( try_t *t, cell_t col, cell_t row, cell_t el );

/* alloc and prepare try board */
try_t *
try_new( puzzle_t *p );

/* are all elements unique ? */
int
try_is_solved( try_t *t );

int
try_find( try_t *t, cell_t row, cell_t el );

void
try_print( try_t *t );


/* base rule definition */
typedef struct rule_s rule_t;

#define RULE_STRUCT_BASE \
	int (*apply)( rule_t *r, try_t *t ); \
	int (*get)( rule_t *r, char *buf ); \
	int (*check)( rule_t *r, try_t *t ); \
	rule_t *next;

struct rule_s
{
	RULE_STRUCT_BASE
	cell_t data[0];
};

/* create new random rule */
rule_t *
rule_new( puzzle_t *p );

void
rule_print( rule_t *r );

/* init try board from open rules */
void
try_rule_init( try_t *t, rule_t *r );

typedef struct game_s {
	puzzle_t *p;
	rule_t *r;
	try_t *t;
} game_t;

game_t *
game_gen( cell_t cols, cell_t rows );

void
game_print( game_t *g );

void
game_free( game_t *g );

int
puzzle_try_is_valid( puzzle_t *p, try_t *t );

/* vim: ts=4:sw=4:fdm=marker
 */
