package WebTK;

use warnings;
use strict;
use XML::LibXML ();
use List::Util;

my %by_id;

sub init
{

	my $doc = XML::LibXML::Document->new( '1.0', 'utf-8' );
	$doc->createInternalSubset(
		'html',
		'-//W3C//DTD XHTML 1.1//EN',
		'http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd'
	);

	my $node = XML::LibXML::Comment->new( " WebTK 0.1 " );
	$doc->addChild( $node );

	my $root = $doc->createElement( "html" );
	$doc->setDocumentElement( $root );

	$root->setAttribute( "xmlns" => "http://www.w3.org/1999/xhtml" );
	$root->setAttribute( "xml:lang" => "en" );

	my $head = $doc->createElement( "head" );
	$root->addChild( $head );

	{
		my $title = $doc->createElement( "title" );
		$title->addChild( $doc->createTextNode( "WebTK" ) );
		$head->addChild( $title );
	}

	{
		my $link = $doc->createElement( "link" );
		$link->setAttribute( rel => "stylesheet" );
		$link->setAttribute( type => "text/css" );
		$link->setAttribute( href => "/_webtk/screen.css" );
		$link->setAttribute( media => "screen" );
		$head->addChild( $link );
	}

	{
		my $link = $doc->createElement( "link" );
		$link->setAttribute( rel => "shortcut icon" );
		$link->setAttribute( href => "/_webtk/favicon.png" );
		$head->addChild( $link );
	}

	{
		my $script = $doc->createElement( "script" );
		$script->setAttribute( type => "text/javascript" );
		$script->setAttribute( src => "/_webtk/interface.js" );
		$head->addChild( $script );
	}


	my $body = $doc->createElement( "body" );
	$root->addChild( $body );

	return $body;
}


sub autotable
{
	my $doc = shift;
	my $cols = shift;
	my $rows = shift;
	my $callback = shift;

	my $table = $doc->createElement( "table" );
	my $tbody = $doc->createElement( "tbody" );
	$table->addChild( $tbody );

	for ( my $row = 0; $row < $rows; $row++ ) {
		my $tr = $doc->createElement( "tr" );
		$tbody->addChild( $tr );
		for ( my $col = 0; $col < $cols; $col++ ) {
			my $td = $doc->createElement( "td" );
			$tr->addChild( $td );
			&$callback( $td, $col, $row, @_ );
		}
	}
	
	return $table;
}


sub _addClass
{
	my $el = shift;
	my $tkclass = shift;

	my $class = $el->getAttribute( "class" );
	unless ( $class ) {
		$el->setAttribute( "class", $tkclass );
		return;
	}
	my @class = split /\s+/, $class;

	return
		if @class and first { $_ eq $tkclass } @class;

	push @class, $tkclass;

	$el->setAttribute( "class", join " ", @class );
}

sub _id
{
	my $el = shift;
	my $id = $el->getAttribute( "id" );
	if ( $id and $id =~ /^tk_[0-9a-f]{8}$/ ) {
		return $id;
	}
	do {
		$id = sprintf "tk_%04x%04x", rand 1 << 16, rand 1 << 16;
	} while ( exists $by_id{ $id } );
	$el->setAttribute( "id", $id );
	return $id;
}

sub button
{
	my $tkclass = shift;
	my $el = shift;

	_addClass( $el, $tkclass );
	my $id = _id( $el );

	# TODO
}

package XML::LibXML::Element;
sub button
{
	unshift @_, "tk_click";
	goto &WebTK::button;
}

sub rbutton
{
	unshift @_, "tk_rclick";
	goto &WebTK::button;
}

sub autotable
{
	my $self = $_[0];
	$_[0] = $self->ownerDocument;
	my $table = &WebTK::autotable;
	$self->addChild( $table );
}

1;

#print "validate: \n" . $doc->validate();
