
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

void
DESTROY(self)
	Game::Einstein self
	CODE:
		game_free( self );
