/*
 *
 */
#include "einstein.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>

static void
try_reset( try_t *t )
{
	size_t i;
	size_t max = t->cols * t->rows;

	assert( t->cols <= sizeof( try_cell_t ) * 8 );

	/* mark with bit 1 all possible numbers */
	try_cell_t mask = TRY_MASK( t->cols ) - 1;

	for ( i = 0; i < max; i++ )
		t->board[ i ] = mask;

	return;
}

static void
try_check_singles( try_t *t, size_t row )
{
	bool changed = false;
	long int * cells_count, * els_count, * els, * el_cells;
	size_t size = sizeof( long int ) * t->cols * 4;
	size_t col;
	cell_t el;

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
			if ( elements & TRY_MASK( el ) ) {
				/* count elements of this value */
				els_count[ el ]++;
				/* in what cell this element can be found */
				el_cells[ el ] = col;
				/* how many elements does this cell have */
				cells_count[ col ]++;
				/* value of last element found */
				els[ col ] = (long int)el;
			}
	}

	/* foreach cell */
	for ( col = 0; col < t->cols; col++ )
		/* if there is only one element in this cell */
		/* and the element can be found somewhere else */
		if ( (cells_count[ col ] == 1) && ( els_count[ els[ col ] ] != 1 ) ) {
			long int i;
			try_cell_t mask = TRY_MASK( els[ col ] );
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
			try_cell_t mask = TRY_MASK( el );
			size_t col = el_cells[ el ];
			CELL( t, row, col ) = mask;

			changed = 1;
		}

	free( cells_count );

	if ( changed )
		try_check_singles( t, row );
}

/* find element */
int
try_find( const try_t *t, size_t row, cell_t el )
{
	size_t col;
	try_cell_t mask = TRY_MASK( el );
	for ( col = 0; col < t->cols; col++ )
		if ( CELL( t, row, col ) == mask )
			return col;
	return -1;
}

bool
try_exclude( try_t *t, size_t col, size_t row, cell_t el )
{
	try_cell_t mask = TRY_MASK( el );
	if ( ! (CELL( t, row, col ) & mask ) )
		return false;

	CELL( t, row, col ) &= ~mask;

	try_check_singles( t, row );

	return true;
}

bool
try_set( try_t *t, size_t col, size_t row, cell_t el )
{
	size_t i;
	try_cell_t mask = TRY_MASK( el );

	if ( ! (CELL( t, row, col ) & mask ) )
		return false;

	for ( i = 0; i < t->cols; i++ )
		CELL( t, row, i ) &= ~mask;

	CELL( t, row, col ) = mask;

	try_check_singles( t, row );

	return true;
}

try_t *
try_new( const puzzle_t *p )
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
try_rule_init( try_t *t, const rule_t *r )
{
	do {
		r = r->next;
		if ( r->get == NULL && r->apply != NULL )
			r->apply( r, t );
	} while ( r->next );
}

bool
try_is_solved( const try_t *t )
{
	size_t col, row;
	for ( row = 0; row < t->rows; row++ )
		for ( col = 0; col < t->cols; col++ )
			if ( ! try_is_defined( t, col, row ) )
				return false;
	return true;
}


void
try_print( const try_t *t )
{
	size_t row, col, i;
	try_cell_t c;
	assert( t != NULL );

	printf( "try board:\n" );
	for ( row = 0; row < t->rows; row++ ) {
		printf( "%c:", 'A' + (int)row );
		for ( col = 0; col < t->cols; col++ ) {
			putchar( ' ' );
			c = CELL( t, row, col );
			for ( i = 0; i < t->cols; i++ ) {
				putchar( ( c & TRY_MASK( i ) ) ? '0' + ( i & 7 ) : '_' );
			}
		}
		putchar( '\n' );
	}

}

/* vim: ts=4:sw=4:fdm=marker
 */
