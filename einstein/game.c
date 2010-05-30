/*
 *
 */
#include "einstein.h"
#include <stdio.h>
#include <malloc.h>
#include <assert.h>

/* return random innteger in [0,max) interval */
unsigned int
irand( unsigned int max ) /* {{{ */
{
	unsigned int r, ret;

	assert( max < RAND_MAX );

	/* make sure distribution is even */
	do {
		r = (unsigned int) rand();
		ret = r % max;
	} while ( r - ret + max >= RAND_MAX );

	return ret;
} /* }}} */

static void
puzzle_shuffle( puzzle_t *p )
{
	size_t col, row, a, b, i;
	cell_t tmp;
	for ( row = 0; row < p->rows; row++ ) {
		for ( col = 0; col < p->cols; col++ )
			CELL( p, row, col ) = (cell_t)col;

		for ( i = 0; i < 5 * p->cols; i++ ) {
			a = irand( p->cols );
			b = irand( p->cols );
			tmp = CELL( p, row, a );
			CELL( p, row, a ) = CELL( p, row, b );
			CELL( p, row, b ) = tmp;
		}
	}
}

static puzzle_t *
puzzle_new( size_t cols, size_t rows )
{
	puzzle_t *p;
	assert( cols >= 3 );
	assert( rows >= 2 );
	assert( cols < sizeof( try_cell_t ) * 8 );
	p = malloc( sizeof( puzzle_t ) + sizeof( cell_t ) * cols * rows );
	assert( p != NULL );
	p->rows = rows;
	p->cols = cols;

	return p;
}

bool
puzzle_try_is_valid( const puzzle_t *p, const try_t *t )
{
	size_t i, max = p->cols * p->rows;
	assert( p->cols == t->cols );
	assert( p->rows == t->rows );

	for ( i = 0; i < max; i++ )
		if ( ! ( t->board[ i ] & TRY_MASK( p->board[ i ] ) ) )
			return false;
	return true;
}

static bool
puzzle_rule_can_solve( const puzzle_t *p, const rule_t *rule_first )
{
	try_t *t;
	bool changed, ret;
	const rule_t *rule;
	t = try_new( p );

	do {
		rule = rule_first;
		changed = false;
		do {
			if ( ! rule->apply )
				continue;

			if ( rule->apply( rule, t ) ) {
				changed = true;
				assert( puzzle_try_is_valid( p, t ) );
			}
		} while ( ( rule = rule->next ) != NULL );
	} while( changed );

	ret = try_is_solved( t );
	free( t );
	return ret;
}

static void
puzzle_rule_gen( const puzzle_t *p, rule_t *rule_first )
{
	int rules_done = 0;
	rule_t * rule;
	rule_t * rule_last = rule_first;

	do {
		rule = rule_new( p );
		rule_last->next = rule;
		rule_last = rule;
		rules_done = puzzle_rule_can_solve( p, rule_first );
	} while ( !rules_done );
}

static void
puzzle_rule_remove( const puzzle_t *p, rule_t *rule_first )
{
	rule_t * rule_before, * rule;
	rule_before = rule_first;

	do {
		rule = rule_before->next;
		rule_before->next = rule->next;
		if ( puzzle_rule_can_solve( p, rule_first ) ) {
			free( rule );
		} else {
			rule_before->next = rule;
			rule_before = rule;
		}
	} while ( rule_before->next );
}

game_t *
game_gen( size_t cols, size_t rows )
{
	game_t *g;

	puzzle_t *p;
	rule_t *r;
	try_t *t;

	p = puzzle_new( cols, rows );
	puzzle_shuffle( p );

	r = malloc( sizeof( rule_t ) );
	r->apply = NULL;
	r->get = NULL;
	r->next = NULL;

	puzzle_rule_gen( p, r );
	puzzle_rule_remove( p, r );

	t = try_new( p );
	try_rule_init( t, r );

	g = malloc( sizeof( game_t ) );
	g->p = p;
	g->r = r;
	g->t = t;

	return g;
}

static void
puzzle_print( const puzzle_t *p )
{
	size_t row, col;
	assert( p != NULL );

	printf( "puzzle board:\n" );
	for ( row = 0; row < p->rows; row++ ) {
		printf( "%c:", 'A' + (int)row );
		for ( col = 0; col < p->cols; col++ ) {
			printf( " %2d", (int)CELL( p, row, col ) );
		}
		putchar( '\n' );
	}
}

void
game_print( const game_t *g )
{
	assert( g != NULL );
	puzzle_print( g->p );
	putchar( '\n' );
	rule_print( g->r );
	putchar( '\n' );
	try_print( g->t );
}

void
game_free( game_t *g )
{
	assert( g != NULL );
	rule_t *r, *rn;
   	r = g->r;

	free( g->p );
	free( g->t );
	free( g );

	do {
		rn = r->next;
		free( r );
	} while ( ( r = rn ) != NULL );
}

#ifdef TEST
int
main( int argc, char *argv[] )
{
	size_t col = 0, row = 0;

	game_t *g;
	if ( argc >= 3 ) {
		col = atoi( argv[1] );
		row = atoi( argv[2] );
	}
	if ( argc >= 4 ) {
		srand( atoi( argv[3] ) );
	}
	if ( col < 3 )
		col = 3;
	if ( row < 2 )
		row = 2;
	g = game_gen( col, row );
	game_print( g );

	game_free( g );

	return 0;
}
#endif

/* vim: ts=4:sw=4:fdm=marker
 */
