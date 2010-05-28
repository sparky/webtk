package Game::Einstein;
use strict;
use warnings;

require DynaLoader;

our $VERSION = '0.00_00';
our @ISA = qw(DynaLoader);

bootstrap Game::Einstein $VERSION;

1;
