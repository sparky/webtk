
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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


/* cell_t holds small integer values */
typedef unsigned char cell_t;
typedef struct puzzle_s {
	cell_t rows;
	cell_t cols;
	cell_t board[ 0 ];
} puzzle_t;

/* try_cell_t holds bitmasks of possible values */
typedef unsigned int try_cell_t;
typedef struct try_s {
	cell_t rows;
	cell_t cols;
	try_cell_t board[ 0 ];
} try_t;

#define CELL( s, r, c ) ((s)->board[ r * (s)->cols + c ])

int
try_is_possible( try_t *t, cell_t col, cell_t row, cell_t el );

int
try_is_defined( try_t *t, cell_t col, cell_t row );

void
try_exclude( try_t *t, cell_t col, cell_t row, cell_t el );

void
try_set( try_t *t, cell_t col, cell_t row, cell_t el );


typedef struct rule_s rule_t;

#define RULE_STRUCT_BASE \
	int (*apply)( rule_t *r, try_t *t ); \
	char * (*get)( rule_t *r ); \
	rule_t *next;

struct rule_s
{
	RULE_STRUCT_BASE
	cell_t data[0];
};


#define MAKE_RULE( r, rtype ) \
	rule_ ## rtype ## _t *r; \
	r = malloc( sizeof( rule_ ## rtype ## _t ) ); \
	assert( r != NULL ); \
	r->apply = rule_ ## rtype ## _apply; \
	r->get = rule_ ## rtype ## _get; \
	r->next = NULL;
	

/* rule near {{{ */
typedef struct
{
	RULE_STRUCT_BASE
	cell_t row1, el1;
	cell_t row2, el2;
} rule_near_t;

#define	DESC_NEAR "near: %c%c %c%c"
char *
rule_near_get( rule_t *r_ ) /* {{{ */
{
	rule_near_t *r = (rule_near_t *)r_;
	char * ret = malloc( strlen( DESC_NEAR ) );
	assert( ret != NULL );
	sprintf( ret, DESC_NEAR,
			'A' + r->row1,
			'1' + r->el1,
			'A' + r->row2,
			'1' + r->el2
	);
	return ret;
} /* }}} */

static int
rule_near_apply_to_col( try_t *t, cell_t col,
		cell_t near_row, cell_t near_num,
		cell_t this_row, cell_t this_num ) /* {{{ */
{
	int has_left, has_right;

	if ( col == 0 )
		has_left = 0;
	else
		has_left = try_is_possible( t, col - 1, near_row, near_num );

	if ( col == t->cols - 1 )
		has_right = 0;
	else
		has_right = try_is_possible( t, col + 1, near_row, near_num );

	if ( (!has_right) && (!has_left) && try_is_possible( t, col, this_row, this_num ) ) {
		try_exclude( t, col, this_row, this_num );
		return 1;
	} else {
		return 0;
	}
} /* }}} */

int
rule_near_apply( rule_t *r_, try_t *t ) /* {{{ */
{
	rule_near_t *r = (rule_near_t *)r_;
	int i;
	int changed = 0;

	for ( i = 0; i < t->cols; i++ ) {
		if ( rule_near_apply_to_col( t, i,
					r->row1, r->el1,
					r->row2, r->el2 ) )
			changed = 1;
		if ( rule_near_apply_to_col( t, i,
					r->row2, r->el2,
					r->row1, r->el1 ) )
			changed = 1;
	}

	if ( changed )
		rule_near_apply( r_, t );

	return changed;
} /* }}} */

rule_t *
rule_near_new( puzzle_t *p ) /* {{{ */
{
	int col1, col2;
	MAKE_RULE( r, near );

restart:
	col1 = irand( p->cols );

	if ( irand( 2 ) ) {
		col2 = col1 + 1;
		if ( col2 >= p->cols )
			goto restart;
	} else {
		col2 = col1 - 1;
		if ( col2 < 0 )
			goto restart;
	}

	r->row1 = irand( p->rows );
	r->el1 = CELL( p, r->row1, col1 );

	r->row2 = irand( p->rows );
	r->el2 = CELL( p, r->row2, col2 );

	return (rule_t *)r;
} /* }}} */
/* }}} */

/* rule direction {{{ */
typedef struct
{
	RULE_STRUCT_BASE
	cell_t row1, el1;
	cell_t row2, el2;
} rule_dir_t;

#define	DESC_DIR "dir: %c%c %c%c"
char *
rule_dir_get( rule_t *r_ ) /* {{{ */
{
	rule_dir_t *r = (rule_dir_t *)r_;
	char * ret = malloc( strlen( DESC_DIR ) );
	assert( ret != NULL );
	sprintf( ret, DESC_DIR,
			'A' + r->row1,
			'1' + r->el1,
			'A' + r->row2,
			'1' + r->el2
	);
	return ret;
} /* }}} */

int
rule_dir_apply( rule_t *r_, try_t *t ) /* {{{ */
{
	rule_dir_t *r = (rule_dir_t *)r_;
	int i;
	int changed = 0;

	/* remove all right els that are not on the right of the leftmost left el */
	for ( i = 0; i < t->cols; i++ ) {
		if ( try_is_possible( t, i, r->row2, r->el2 ) ) {
			try_exclude( t, i, r->row2, r->el2 );
			changed = 1;
		}
		if ( try_is_possible( t, i, r->row1, r->el1 ) )
			break;
	}

	/* remove all left els that are not on the left of the rightmost right el */
	for ( i = t->cols - 1; i >= 0; i-- ) {
		if ( try_is_possible( t, i, r->row1, r->el1 ) ) {
			try_exclude( t, i, r->row1, r->el1 );
			changed = 1;
		}
		if ( try_is_possible( t, i, r->row2, r->el2 ) )
			break;
	}

	return changed;
} /* }}} */

rule_t *
rule_dir_new( puzzle_t *p ) /* {{{ */
{
	int col1, col2;
	MAKE_RULE( r, dir );

	r->row1 = irand( p->rows );
	r->row2 = irand( p->rows );
	col1 = irand( p->cols - 1 );
	col2 = irand( p->cols - col1 - 1 ) + col1 + 1;
	r->el1 = CELL( p, r->row1, col1 );
	r->el2 = CELL( p, r->row2, col2 );

	return (rule_t *)r;
} /* }}} */
/* }}} */

/* rule open {{{ */
typedef struct
{
	RULE_STRUCT_BASE
	cell_t col, row, el;
} rule_open_t;

#define	DESC_OPEN "open: %c %c%c"
char *
rule_open_get( rule_t *r_ ) /* {{{ */
{
	rule_open_t *r = (rule_open_t *)r_;
	char * ret = malloc( strlen( DESC_OPEN ) );
	assert( ret != NULL );
	sprintf( ret, DESC_OPEN,
			'1' + r->col,
			'A' + r->row,
			'1' + r->el
	);
	return ret;
} /* }}} */

int
rule_open_apply( rule_t *r_, try_t *t ) /* {{{ */
{
	rule_open_t *r = (rule_open_t *)r_;
	if ( ! try_is_defined( t, r->col, r->row ) ) {
		try_set( t, r->col, r->row, r->el );
		return 1;
	}
	return 0;
} /* }}} */

rule_t *
rule_open_new( puzzle_t *p ) /* {{{ */
{
	MAKE_RULE( r, open );

	r->col = irand( p->cols );
	r->row = irand( p->rows );
	r->el = CELL( p, r->row, r->col );

	return (rule_t *)r;
} /* }}} */
/* }}} */

/* rule under {{{ */
typedef struct
{
	RULE_STRUCT_BASE
	cell_t row1, el1;
	cell_t row2, el2;
} rule_under_t;

#define	DESC_UNDER "under: %c%c %c%c"
char *
rule_under_get( rule_t *r_ ) /* {{{ */
{
	rule_under_t *r = (rule_under_t *)r_;
	char * ret = malloc( strlen( DESC_UNDER ) );
	assert( ret != NULL );
	sprintf( ret, DESC_UNDER,
			'A' + r->row1,
			'1' + r->el1,
			'A' + r->row2,
			'1' + r->el2
	);
	return ret;
} /* }}} */

int
rule_under_apply( rule_t *r_, try_t *t ) /* {{{ */
{
	rule_under_t *r = (rule_under_t *)r_;
	int i;
	int changed = 0;

	for ( i = 0; i < t->cols; i++ ) {
		if ( (!try_is_possible( t, i, r->row1, r->el1 ) ) &&
				try_is_possible( t, i, r->row2, r->el2 ) )
		{
			try_exclude( t, i, r->row2, r->el2 );
			changed = 1;
		}
		if ( (!try_is_possible( t, i, r->row2, r->el2 ) ) &&
				try_is_possible( t, i, r->row1, r->el1 ) )
		{
			try_exclude( t, i, r->row1, r->el1 );
			changed = 1;
		}
	}

	return changed;
} /* }}} */

rule_t *
rule_under_new( puzzle_t *p ) /* {{{ */
{
	int col;
	MAKE_RULE( r, under );

	col = irand( p->cols );
	r->row1 = irand( p->rows );
	r->row2 = irand( p->rows - 1 );
	if ( r->row2 >= r->row1 )
		r->row2++;
	else {
		/* inverted order is very annoing */
		cell_t row = r->row2;
		r->row2 = r->row1;
		r->row1 = row;
	}
	r->el1 = CELL( p, r->row1, col );
	r->el2 = CELL( p, r->row2, col );

	return (rule_t *)r;
} /* }}} */
/* }}} */

/* rule between {{{ */
typedef struct
{
	RULE_STRUCT_BASE
	unsigned char row1, el1;
	unsigned char rowC, elC;
	unsigned char row2, el2;
} rule_between_t;

#define	DESC_BETWEEN "between: %c%c %c%c %c%c"
char *
rule_between_get( rule_t *r_ ) /* {{{ */
{
	rule_between_t *r = (rule_between_t *)r_;
	char * ret = malloc( strlen( DESC_BETWEEN ) );
	assert( ret != NULL );
	sprintf( ret, DESC_BETWEEN,
			'A' + r->row1,
			'1' + r->el1,
			'A' + r->rowC,
			'1' + r->elC,
			'A' + r->row2,
			'1' + r->el2
	);
	return ret;
} /* }}} */

int
rule_between_apply( rule_t *r_, try_t *t ) /* {{{ */
{
	rule_between_t *r = (rule_between_t *)r_;
	int good_loop = 0;
	int changed = 0;

	if ( try_is_possible( t, 0, r->rowC, r->elC ) ) {
		changed = 1;
		try_exclude( t, 0, r->rowC, r->elC );
	}

	if ( try_is_possible( t, t->cols - 1, r->rowC, r->elC ) ) {
		changed = 1;
		try_exclude( t, t->cols - 1, r->rowC, r->elC );
	}

	do {
		int i;
		good_loop = 0;

		for ( i = 1; i < t->cols - 1; i++ ) {
			if ( try_is_possible( t, i, r->rowC, r->elC ) ) {
				if ( ! ( (try_is_possible( t, i - 1, r->row1, r->el1 ) &&
								try_is_possible( t, i + 1, r->row2, r->el2) ) ||
							( try_is_possible( t, i - 1, r->row2, r->el2 ) &&
								try_is_possible( t, i + 1, r->row1, r->el1))
						)
					)
				{
					try_exclude( t, i, r->rowC, r->elC );
					good_loop = 1;
				}
			}
		}

		for ( i = 0; i < t->cols; i++ ) {
			int left_possible, right_possible;

			if ( try_is_possible( t, i, r->row2, r->el2 ) ) {
				if ( i < 2 )
					left_possible = 0;
				else
					left_possible = (
							try_is_possible( t, i - 1, r->rowC, r->elC )
								&&
							try_is_possible( t, i - 2, r->row1, r->el1 )
						);

				if ( i >= t->cols - 2 )
					right_possible = 0;
				else
					right_possible = (
							try_is_possible( t, i + 1, r->rowC, r->elC )
								&&
							try_is_possible( t, i + 2, r->row1, r->el1 )
						);

				if ( (!left_possible) && (!right_possible) ) {
					try_exclude( t, i, r->row2, r->el2 );
					good_loop = 1;
				}
			}

			if ( try_is_possible( t, i, r->row1, r->el1 ) ) {
				if ( i < 2 )
					left_possible = 0;
				else
					left_possible = (
							try_is_possible( t, i - 1, r->rowC, r->elC )
								&&
							try_is_possible( t, i - 2, r->row2, r->el2 )
						);

				if ( i >= t->cols - 2 )
					right_possible = 0;
				else
					right_possible = (
							try_is_possible( t, i + 1, r->rowC, r->elC )
								&&
							try_is_possible( t, i + 2, r->row2, r->el2 )
						);

				if ( (!left_possible) && (!right_possible) ) {
					try_exclude( t, i, r->row1, r->el1 );
					good_loop = 1;
				}
			}

		}
		
		if ( good_loop )
			changed = 1;

	} while ( good_loop );

	return changed;
} /* }}} */

rule_t *
rule_between_new( puzzle_t *p ) /* {{{ */
{
	int colC;
	MAKE_RULE( r, between );

	r->row1 = irand( p->rows );
	r->rowC = irand( p->rows );
	r->row2 = irand( p->rows );

	colC = irand( p->cols - 2 ) + 1;
	r->elC = CELL( p, r->rowC, colC );
	if ( irand( 2 ) ) {
		r->el1 = CELL( p, r->row1, colC - 1 );
		r->el2 = CELL( p, r->row2, colC + 1 );
	} else {
		r->el1 = CELL( p, r->row1, colC + 1 );
		r->el2 = CELL( p, r->row2, colC - 1 );
	}

	return (rule_t *)r;
} /* }}} */
/* }}} */

rule_t *
rule_new( puzzle_t *p ) /* {{{ */
{
	int rnd = irand( 14 );

	switch ( rnd ) {
		case 0:
		case 1:
		case 2:
		case 3:
			return rule_near_new( p );
		case 4:
			return rule_open_new( p );
		case 5:
		case 6:
			return rule_under_new( p );
		case 7:
		case 8:
		case 9:
		case 10:
			return rule_dir_new( p );
		case 11:
		case 12:
		case 13:
			return rule_between_new( p );
		default:
			assert( 0 );
	}

	return NULL;
} /* }}} */


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
/*
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
*/

int
try_is_possible( try_t *t, cell_t col, cell_t row, cell_t el )
{
	return CELL( t, row, col ) & ( 1 << el );
}

int
try_is_defined( try_t *t, cell_t col, cell_t row )
{
	int i;
	int cnt = 0;
	try_cell_t c = CELL( t, row, col );
	for ( i = 0; i < t->cols; i++ )
		if ( c & ( 1 << i ) )
			if ( ++cnt > 1 )
				return 0;

	return cnt == 1;
}

void
try_exclude( try_t *t, cell_t col, cell_t row, cell_t el )
{
	try_cell_t mask = 1 << el;
	if ( ! (CELL( t, row, col ) & mask ) )
		return;

	CELL( t, row, col ) &= ~mask;

	try_check_singles( t, row );
}

void
try_set( try_t *t, cell_t col, cell_t row, cell_t el )
{
	int i;
	try_cell_t mask = 1 << el;

	for ( i = 0; i < t->cols; i++ )
		CELL( t, row, i ) &= ~mask;

	CELL( t, row, col ) = mask;

	try_check_singles( t, row );
}

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
		if ( r->apply == rule_open_apply )
			rule_open_apply( r, t );
	} while ( r->next );
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
try_is_solved( try_t *t )
{
	int col, row;
	for ( row = 0; row < t->rows; row++ )
		for ( col = 0; col < t->cols; col++ )
			if ( ! try_is_defined( t, col, row ) )
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

typedef struct game_s {
	puzzle_t *p;
	rule_t *r;
	try_t *t;
} game_t;

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
				printf( " " );
			printf( "%c", '1' + CELL(p, row, col ) );
		}
		printf( "\n" );
	}
}

void
rule_print( rule_t *r )
{
	char *c;
	assert( r != NULL );

	printf( "rules:\n" );
	do {
		r = r->next;
		if ( ! r->get )
			continue;
		c = r->get( r );
		printf( "R: %s\n", c );
		free( c );
	} while ( r->next );

}

void
try_print( try_t *t )
{
	cell_t row, col, i;
	assert( t != NULL );

	try_cell_t c;
	for ( row = 0; row < t->rows; row++ ) {
		printf( "%c: ", 'A' + row );
		for ( col = 0; col < t->cols; col++ ) {
			if ( col )
				printf( " " );
			c = CELL( t, row, col );
			for ( i = 0; i < t->cols; i++ ) {
				printf( "%c", (c & (1 << i)) ? '1' + i : '_' );
			}
		}
		printf( "\n" );
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

/* vim: ts=4:sw=4:fdm=marker
 */
