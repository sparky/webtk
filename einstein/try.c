
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "einstein.h"

void
try_reset( try_t *t )
{
	int i;
	int max = t->cols * t->rows;
	/* mark with bit 1 all possible numbers */
	try_cell_t mask = ( 1 << t->cols ) - 1;

	for ( i = 0; i < max; i++ )
		t->board[ i ] = mask;

	return;
}

void
try_check_singles( try_t *t, cell_t row )
{
	int changed = 0;
	int * cells_count, * els_count, * els, * el_cells;
	size_t size = sizeof( int ) * t->cols * 4;
	cell_t col, el;

	/* alloc all at once for speed */
	cells_count = malloc( size );
	assert( cells_count != NULL );
	memset( cells_count, 0, size );

	cells_count	= &cells_count[ t->cols * 0 ];
	els_count	= &cells_count[ t->cols * 1 ];
	els			= &cells_count[ t->cols * 2 ];
	el_cells	= &cells_count[ t->cols * 3 ];

	/* foreach cell */
	for ( col = 0; col < t->cols; col++ ) {
		try_cell_t elements = CELL( t, row, col );
		cell_t el;
		for ( el = 0; el < t->cols; el++ )
			if ( elements & ( 1 << el ) ) {
				/* count elements of this value */
				els_count[ el ]++;
				/* in what cell this element can be found */
				el_cells[ el ] = col;
				/* how many elements does this cell have */
				cells_count[ col ]++;
				/* value of last element found */
				els[ col ] = el;
			}
	}

	/* foreach cell */
	for ( col = 0; col < t->cols; col++ )
		/* if there is only one element in this cell */
		/* and the element can be found somewhere else */
		if ( (cells_count[ col ] == 1) && ( els_count[ els[ col ] ] != 1 ) ) {
			int i;
			try_cell_t mask = 1 << els[ col ];
			/* remove this element from all cells */
			for ( i = 0; i < t->cols; i++ )
				CELL( t, row, i ) &= ~mask;

			CELL( t, row, col ) = mask;

			changed = 1;
		}

	/* foreach element */
	for ( el = 0; el < t->cols; el++ )
		/* if some element appears only once */
		/* and there are other elements in that cell */
		if ( (els_count[ el ] == 1) && ( cells_count[ el_cells[ el ] ] != 1 ) ) {
			try_cell_t mask = 1 << el;
			cell_t col = el_cells[ el ];
			CELL( t, row, col ) = mask;

			changed = 1;
		}

	free( cells_count );

	if ( changed )
		try_check_singles( t, row );
}

/* find element */
int
try_find( try_t *t, cell_t row, cell_t el )
{
	cell_t col;
	try_cell_t mask = 1 << el;
	for ( col = 0; col < t->cols; col++ )
		if ( CELL( t, row, col ) == mask )
			return col;
	return -1;
}

int
try_exclude( try_t *t, cell_t col, cell_t row, cell_t el )
{
	try_cell_t mask = 1 << el;
	if ( ! (CELL( t, row, col ) & mask ) )
		return 0;

	CELL( t, row, col ) &= ~mask;

	try_check_singles( t, row );

	return 1;
}

int
try_set( try_t *t, cell_t col, cell_t row, cell_t el )
{
	int i;
	try_cell_t mask = 1 << el;

	if ( ! (CELL( t, row, col ) & mask ) )
		return 0;

	for ( i = 0; i < t->cols; i++ )
		CELL( t, row, i ) &= ~mask;

	CELL( t, row, col ) = mask;

	try_check_singles( t, row );

	return 1;
}

try_t *
try_new( puzzle_t *p )
{
	try_t *t;
	assert( p != NULL );
	t = malloc( sizeof( try_t ) + sizeof( try_cell_t ) * p->cols * p->rows );
	assert( t != NULL );
	t->rows = p->rows;
	t->cols = p->cols;
	try_reset( t );

	return t;
}

void
try_rule_init( try_t *t, rule_t *r )
{
	do {
		r = r->next;
		if ( r->get == NULL && r->apply != NULL )
			r->apply( r, t );
	} while ( r->next );
}

int
try_is_solved( try_t *t )
{
	int col, row;
	for ( row = 0; row < t->rows; row++ )
		for ( col = 0; col < t->cols; col++ )
			if ( ! try_is_defined( t, col, row ) )
				return 0;
	return 1;
}


void
try_print( try_t *t )
{
	cell_t row, col, i;
	assert( t != NULL );

	try_cell_t c;
	for ( row = 0; row < t->rows; row++ ) {
		printf( "%c:", 'A' + row );
		for ( col = 0; col < t->cols; col++ ) {
			putchar( ' ' );
			c = CELL( t, row, col );
			for ( i = 0; i < t->cols; i++ ) {
				putchar( (c & (1 << i)) ? '0' + ( i & 7 ) : '_' );
			}
		}
		putchar( '\n' );
	}

}

/* vim: ts=4:sw=4:fdm=marker
 */
