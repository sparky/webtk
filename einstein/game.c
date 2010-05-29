
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "einstein.h"

/* return random innteger in [0,max) interval */
unsigned long int
irand( unsigned long int max ) /* {{{ */
{
	unsigned long int r, ret;

	assert( max < RAND_MAX );

	/* make sure distribution is even */
	do {
		r = (unsigned long int) rand();
		ret = r % max;
	} while ( r - ret + max >= RAND_MAX );

	return ret;
} /* }}} */

void
puzzle_shuffle( puzzle_t *p )
{
	cell_t col, row, a, b, tmp;
	int i;
	for ( row = 0; row < p->rows; row++ ) {
		for ( col = 0; col < p->cols; col++ )
			CELL( p, row, col ) = col;

		for ( i = 0; i < 5 * p->cols; i++ ) {
			a = irand( p->cols );
			b = irand( p->cols );
			tmp = CELL( p, row, a );
			CELL( p, row, a ) = CELL( p, row, b );
			CELL( p, row, b ) = tmp;
		}
	}
}

puzzle_t *
puzzle_new( cell_t cols, cell_t rows )
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

int
puzzle_try_is_valid( puzzle_t *p, try_t *t )
{
	int i, max = p->cols * p->rows;
	assert( p->cols == t->cols );
	assert( p->rows == t->rows );

	for ( i = 0; i < max; i++ )
		if ( ! ( t->board[ i ] & ( 1 << p->board[ i ] ) ) )
			return 0;
	return 1;
}

int
puzzle_rule_can_solve( puzzle_t *p, rule_t *rule_first )
{
	try_t *t;
	int changed, ret;
	rule_t *rule;
	t = try_new( p );

	do {
		rule = rule_first;
		changed = 0;
		do {
			if ( ! rule->apply )
				continue;

			if ( rule->apply( rule, t ) ) {
				changed = 1;
				assert( puzzle_try_is_valid( p, t ) );
			}
		} while ( ( rule = rule->next ) != NULL );
	} while( changed );

	ret = try_is_solved( t );
	free( t );
	return ret;
}

void
puzzle_rule_gen( puzzle_t *p, rule_t *rule_first )
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

void
puzzle_rule_remove( puzzle_t *p, rule_t *rule_first )
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
game_gen( cell_t cols, cell_t rows )
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

void
puzzle_print( puzzle_t *p )
{
	cell_t row, col;
	assert( p != NULL );
	for ( row = 0; row < p->rows; row++ ) {
		printf( "%c: ", 'A' + row );
		for ( col = 0; col < p->cols; col++ ) {
			if ( col )
				putchar( ' ' );
			printf( "%2d", CELL(p, row, col ) );
		}
		putchar( '\n' );
	}
}

void
game_print( game_t *g )
{
	assert( g != NULL );
	puzzle_print( g->p );
	rule_print( g->r );
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
	cell_t col = 0, row = 0;

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
