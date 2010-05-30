
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "einstein.h"

typedef game_t *Game__Einstein;

MODULE = Game::Einstein		PACKAGE = Game::Einstein
PROTOTYPES: ENABLE


Game::Einstein
new(class, cols=6, rows=6, seed=0)
	SV *	class
	int	cols
	int	rows
	unsigned int	seed
	CODE:
	{
		unsigned int reseed = 0;

		if ( cols < 4 || cols > 31 )
			croak( "number of columns must be between 4 and 31" );
		if ( rows < 2 )
			croak( "number of rows must be at least 2" );

		if ( seed ) {
			reseed = (unsigned int) rand();
			srand( seed );
		}
		RETVAL = game_gen( cols, rows );
		if ( seed )
			srand( reseed );
	}
	OUTPUT:
		RETVAL


void
print(self)
	Game::Einstein	self
	CODE:
		game_print( self );

int
cols( self )
	Game::Einstein	self
	ALIAS:
		columns = 1
	CODE:
		RETVAL = self->p->cols;
	OUTPUT:
		RETVAL

int
rows( self )
	Game::Einstein	self
	CODE:
		RETVAL = self->p->rows;
	OUTPUT:
		RETVAL

void
size( self )
	Game::Einstein	self
	INIT:
		puzzle_t *p;
	PPCODE:
		p = self->p;
		PUSHs( sv_2mortal( newSVuv( p->cols ) ) );
		PUSHs( sv_2mortal( newSVuv( p->rows ) ) );

SV *
puzzle( self )
	Game::Einstein	self
	INIT:
		AV *	rows;
		int row, col;
		puzzle_t *p;
	CODE:
	{
		p = self->p;
		rows = (AV *)sv_2mortal( (SV *) newAV() );
		for ( row = 0; row < p->rows; row++ ) {
			AV *rowh = (AV *)sv_2mortal( (SV *) newAV() );
			for ( col = 0; col < p->cols; col++ ) {
				cell_t cell = CELL( p, row, col );
				av_push( rowh, newSVuv( cell ) );
			}
			av_push( rows, newRV( (SV *) rowh ) );
		}
		RETVAL = newRV( (SV *) rows );
	}
	OUTPUT:
		RETVAL


SV *
try( self )
	Game::Einstein	self
	INIT:
		AV *	rows;
		int row, col;
		try_t *t;
	CODE:
	{
		t = self->t;

		rows = (AV *)sv_2mortal( (SV *) newAV() );
		for ( row = 0; row < t->rows; row++ ) {
			AV *rowh = (AV *)sv_2mortal( (SV *) newAV() );
			for ( col = 0; col < t->cols; col++ ) {
				try_cell_t cell = CELL( t, row, col );
				av_push( rowh, newSVuv( cell ) );
			}
			av_push( rows, newRV( (SV *) rowh ) );
		}
		RETVAL = newRV( (SV *) rows );
	}
	OUTPUT:
		RETVAL

SV *
try_get_row( self, row )
	Game::Einstein	self
	int row
	INIT:
		AV *	rowh;
		int col;
		try_t *t;
	CODE:
	{
		t = self->t;

		if ( row < 0 || row >= t->rows )
			croak( "row number must be between 0 and number of rows" );

		rowh = (AV *)sv_2mortal( (SV *) newAV() );
		for ( col = 0; col < t->cols; col++ ) {
			try_cell_t cell = CELL( t, row, col );
			av_push( rowh, newSVuv( cell ) );
		}
		RETVAL = newRV( (SV *) rowh );
	}
	OUTPUT:
		RETVAL

SV *
rules( self )
	Game::Einstein	self
	INIT:
		AV *	rule_list;
		rule_t *r;
		char rule_text[16];
		STRLEN rule_size;
	CODE:
	{
		r = self->r->next;
		rule_list = (AV *)sv_2mortal( (SV *) newAV() );
		do {
			if ( ! r->get )
				continue;
			rule_size = r->get( r, rule_text );
			av_push( rule_list, newSVpv( rule_text, rule_size ) );
		} while ( ( r = r->next ) != NULL );

		RETVAL = newRV( (SV *) rule_list );
	}
	OUTPUT:
		RETVAL

int
set( self, col, row, el )
	Game::Einstein	self
	int	col
	int	row
	int	el
	INIT:
		try_t *t;
	CODE:
	{
		t = self->t;
		if ( col < 0 || col >= t->cols )
			croak( "col number must be between 0 and number of columns" );
		if ( row < 0 || row >= t->rows )
			croak( "row number must be between 0 and number of rows" );
		if ( el < 0 || el >= t->cols )
			croak( "el number must be between 0 and number of columns" );
		RETVAL = try_set( t, col, row, el );
	}
	OUTPUT:
		RETVAL


int
exclude( self, col, row, el )
	Game::Einstein	self
	int	col
	int	row
	int	el
	INIT:
		try_t *t;
	CODE:
	{
		t = self->t;
		if ( col < 0 || col >= t->cols )
			croak( "col number must be between 0 and number of columns" );
		if ( row < 0 || row >= t->rows )
			croak( "row number must be between 0 and number of rows" );
		if ( el < 0 || el >= t->cols )
			croak( "el number must be between 0 and number of columns" );
		RETVAL = try_exclude( t, col, row, el );
	}
	OUTPUT:
		RETVAL


int
is_solved( self )
	Game::Einstein	self
	CODE:
		RETVAL = try_is_solved( self->t );
	OUTPUT:
		RETVAL

int
is_correct( self )
	Game::Einstein	self
	CODE:
		RETVAL = puzzle_try_is_valid( self->p, self->t );
	OUTPUT:
		RETVAL

void
invalid_rules( self )
	Game::Einstein	self
	INIT:
		try_t *t;
		rule_t *r;
		char rule_text[16];
		STRLEN rule_size;
	PPCODE:
		t = self->t;
		r = self->r->next;
		if ( ! try_is_solved( t ) )
			croak( "puzzle must be solved to get invalid_rules" );

		do {
			if ( ! r->check )
				continue;
			if ( ! r->get )
				continue;
			if ( ! r->check( r, t ) ) {
				rule_size = r->get( r, rule_text );
				PUSHs( sv_2mortal( newSVpv( rule_text, rule_size ) ) );
			}
		} while( ( r = r->next ) != NULL );

int
is_defined( cell )
	UV	cell
	CODE:
	{
		if ( cell == 0 )
			croak( "cell is empty" );
		RETVAL = ( cell & ( cell - 1 ) ) == 0;
	}
	OUTPUT:
		RETVAL

void
DESTROY(self)
	Game::Einstein self
	CODE:
		game_free( self );

# vim: ts=4:sw=4
