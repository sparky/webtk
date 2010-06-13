#!/usr/bin/perl
#
use warnings;
use strict;
use WebTK;

sub cell_click
{
	print "@_\n";
}

sub test
{
	my $webtk = shift;

	my $table = $webtk->table->tbody;
	foreach my $row ( 1..6 ) {
		my $tr = $table->tr;
		foreach my $col ( 1..6 ) {
			my $td = $tr->td;
			$td->text( "$col, $row" );
			my $a = $td->a( href => "#", style => {
					width => 12,
					height => 12,
					border => 1,
				} );
			$a->text( "link" );
			$td->button( \&cell_click, $col, $row );
			$td->rbutton( \&cell_click, $col, $row );
		}
	}

}


print WebTK::new( \&test );
