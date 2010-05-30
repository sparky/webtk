
use Test::More tests => 4;
use Game::Einstein;

ok( my $g = new Game::Einstein 6, 5, 1 );
is( $g->columns, 6 );
is( $g->rows, 5 );
$r = $g->rules;
is( scalar @$r, 18 );

$" = "\n";
diag( "@$r\n" );
