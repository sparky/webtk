#!/usr/bin/perl
#
use warnings;
use strict;
use WebTK;

sub cell_click
{
	print "@_\n";
}

sub cell
{
	my $td = shift;
	my $col = shift;
	my $row = shift;

	$td->addChild( $td->ownerDocument->createTextNode( "$col, $row" ) );
	$td->button( \&cell_click, $col, $row );
}

my $webtk = WebTK::init;
$webtk->autotable( 6, 6, \&cell );

print $webtk->ownerDocument->serialize( 1 );
