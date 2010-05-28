
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

		if ( cols < 4 || cols > 15 )
			croak( "number of columns must be between 4 and 15" );
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
rules( self )
	Game::Einstein	self
	INIT:
		AV *	rule_list;
		rule_t *r;
		char * rule_text;
		STRLEN rule_size;
	CODE:
	{
		r = self->r->next;
		rule_list = (AV *)sv_2mortal( (SV *) newAV() );
		do {
			if ( ! r->get )
				continue;
			rule_text = r->get( r );
			rule_size = strlen( rule_text );
			av_push( rule_list, newSVpv( rule_text, rule_size ) );
			free( rule_text );
		} while ( ( r = r->next ) != NULL );

		RETVAL = newRV( (SV *) rule_list );
	}
	OUTPUT:
		RETVAL


void
DESTROY(self)
	Game::Einstein self
	CODE:
		game_free( self );
