
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "einstein.h"

typedef game_t *Game__Einstein;

MODULE = Game::Einstein		PACKAGE = Game::Einstein


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
		int row, col, el;
		try_t *t;
		char bits[17];
		STRLEN cols;
	CODE:
	{
		t = self->t;
		cols = t->cols;
		bits[ cols ] = '\0';

		rows = (AV *)sv_2mortal( (SV *) newAV() );
		for ( row = 0; row < t->rows; row++ ) {
			AV *rowh = (AV *)sv_2mortal( (SV *) newAV() );
			for ( col = 0; col < t->cols; col++ ) {
				register try_cell_t cell = CELL( t, row, col );
				register int el;
				for ( el = 0; el < t->cols; el++ )
					bits[ el ] = cell & (1 << el) ? '1' : '0';
				av_push( rowh, newSVpv( bits, cols ) );
			}
			av_push( rows, newRV( (SV *) rowh ) );
		}
		RETVAL = newRV( (SV *) rows );
	}
	OUTPUT:
		RETVAL


void
DESTROY(self)
	Game::Einstein self
	CODE:
		game_free( self );
