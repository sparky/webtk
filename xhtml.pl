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

	$td->text( "$col, $row" );
	my $a = $td->a( href => "#", style => {
			left => 10,
			top => 0,
			width => 12,
			height => 12,
			border => 1,
		} );
	$a->text( "link" );
	$td->button( \&cell_click, $col, $row );
	$td->rbutton( \&cell_click, $col, $row );
}

sub test
{
	my $webtk = WebTK::init;
	$webtk->autotable( 6, 6, \&cell );

	print $webtk->ownerDocument->serialize( 1 );
}


test();
