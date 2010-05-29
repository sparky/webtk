
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "einstein.h"

#define MAKE_RULE( r, rtype ) \
	rule_ ## rtype ## _t *r; \
	r = malloc( sizeof( rule_ ## rtype ## _t ) ); \
	assert( r != NULL ); \
	r->apply = rule_ ## rtype ## _apply; \
	r->get = rule_ ## rtype ## _get; \
	r->next = NULL;


/* rule open {{{ */
typedef struct
{
	RULE_STRUCT_BASE
	cell_t col, row, el;
} rule_open_t;

#define rule_open_get NULL

/*static*/ int
rule_open_apply( rule_t *r_, try_t *t ) /* {{{ */
{
	rule_open_t *r = (rule_open_t *)r_;
	if ( ! try_is_defined( t, r->col, r->row ) ) {
		try_set( t, r->col, r->row, r->el );
		return 1;
	}
	return 0;
} /* }}} */

static rule_t *
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

#define	DESC_UNDER "u %c%c %c%c"
static char *
rule_under_get( rule_t *r_ ) /* {{{ */
{
	rule_under_t *r = (rule_under_t *)r_;
	char * ret = malloc( strlen( DESC_UNDER ) );
	assert( ret != NULL );
	sprintf( ret, DESC_UNDER,
			'A' + r->row1,
			'0' + r->el1,
			'A' + r->row2,
			'0' + r->el2
	);
	return ret;
} /* }}} */

static int
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

static rule_t *
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

#define	DESC_BETWEEN "b %c%c %c%c %c%c"
static char *
rule_between_get( rule_t *r_ ) /* {{{ */
{
	rule_between_t *r = (rule_between_t *)r_;
	char * ret = malloc( strlen( DESC_BETWEEN ) );
	assert( ret != NULL );
	sprintf( ret, DESC_BETWEEN,
			'A' + r->row1,
			'0' + r->el1,
			'A' + r->rowC,
			'0' + r->elC,
			'A' + r->row2,
			'0' + r->el2
	);
	return ret;
} /* }}} */

static int
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

static rule_t *
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

/* rule near {{{ */
typedef struct
{
	RULE_STRUCT_BASE
	cell_t row1, el1;
	cell_t row2, el2;
} rule_near_t;

#define	DESC_NEAR "n %c%c %c%c"
static char *
rule_near_get( rule_t *r_ ) /* {{{ */
{
	rule_near_t *r = (rule_near_t *)r_;
	char * ret = malloc( strlen( DESC_NEAR ) );
	assert( ret != NULL );
	sprintf( ret, DESC_NEAR,
			'A' + r->row1,
			'0' + r->el1,
			'A' + r->row2,
			'0' + r->el2
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

static int
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

static rule_t *
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

#define	DESC_DIR "d %c%c %c%c"
static char *
rule_dir_get( rule_t *r_ ) /* {{{ */
{
	rule_dir_t *r = (rule_dir_t *)r_;
	char * ret = malloc( strlen( DESC_DIR ) );
	assert( ret != NULL );
	sprintf( ret, DESC_DIR,
			'A' + r->row1,
			'0' + r->el1,
			'A' + r->row2,
			'0' + r->el2
	);
	return ret;
} /* }}} */

static int
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

static rule_t *
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

rule_t *
rule_new( puzzle_t *p ) /* {{{ */
{
	int rnd = irand( 14 );

	switch ( rnd ) {
		case 0:
			return rule_open_new( p );
		case 1:
		case 2:
			return rule_under_new( p );
		case 3:
		case 4:
		case 5:
			return rule_between_new( p );
		case 6:
		case 7:
		case 8:
		case 9:
			return rule_near_new( p );
		case 10:
		case 11:
		case 12:
		case 13:
			return rule_dir_new( p );
		default:
			assert( 0 );
	}

	return NULL;
} /* }}} */

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

/* vim: ts=4:sw=4:fdm=marker
 */
