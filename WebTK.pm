package WebTK;

use warnings;
use strict;
use XML::LibXML ();
use Scalar::Util qw(weaken);
use List::Util qw(first);

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

	my $root = $doc->createElementNS( "http://www.w3.org/1999/xhtml", "html" );
	$doc->setDocumentElement( $root );

	$root->setAttributeNS( "http://www.w3.org/XML/1998/namespace", "lang" => "en" );

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
		$link->setAttribute( href => "_webtk/screen.css" );
		$link->setAttribute( media => "screen" );
		$head->addChild( $link );
	}

	{
		my $link = $doc->createElement( "link" );
		$link->setAttribute( rel => "shortcut icon" );
		$link->setAttribute( href => "_webtk/favicon.png" );
		$head->addChild( $link );
	}

	{
		my $script = $doc->createElement( "script" );
		$script->setAttribute( type => "text/javascript" );
		$script->setAttribute( src => "_webtk/interface.js" );
		$head->addChild( $script );
	}


	my $body = $doc->createElement( "body" );
	$root->addChild( $body );
	$body->dynamic();

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
		if first { $_ eq $tkclass } @class;

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

	my $id = _id( $el );
	_addClass( $el, $tkclass );

	my $obj = $by_id{ $id } ||= {};
	weaken( $obj->{node} = $el );
	$obj->{$tkclass} = \@_;
}

sub dynamic
{
	my $el = shift;

	my $id = _id( $el );
	#_addClass( $el, "tk_dynamic" );

	my $obj = $by_id{ $id } ||= {};
	weaken( $obj->{node} = $el );
}

sub _fixStyle
{
	my $hash = shift;
	return join " ", map { $_ . ":" . $hash->{$_} .
		($hash->{$_} =~ m/^-?\d+$/ and $hash->{$_} != 0 ? "px" : "") . ";" } keys %$hash;
}

sub _autoChild
{
	my $tag = shift;
	my $node = shift;
	my $doc = $node->ownerDocument;
	my $child = $doc->createElement( $tag );
	$node->addChild( $child );
	while ( my ( $key, $value ) = splice @_, 0, 2 ) {
		if ( $key eq "style" and ref $value eq "HASH" ) {
			$value = _fixStyle( $value );
		}
		$child->setAttribute( $key, $value );
	}
	return $child;
}

package XML::LibXML::Element;

# object is clickable
sub button
{
	unshift @_, "tk_click";
	goto &WebTK::button;
}

# right-click
sub rbutton
{
	unshift @_, "tk_rclick";
	goto &WebTK::button;
}

# can be replaced with some other object
sub dynamic
{
	goto &WebTK::dynamic
}

sub autotable
{
	my $self = $_[0];
	$_[0] = $self->ownerDocument;
	my $table = &WebTK::autotable;
	$self->addChild( $table );
}

sub id
{
	return (shift)->getAttribute( "id" );
}

*addClass = \&WebTK::_addClass;

foreach my $tag ( qw(
	a img
	table tbody thead tfoot tr td th caption colgroup col
	ul ol li dl dt dd
	div span p pre
	form fieldset legend input textarea
	h1 h2 h3 h4 h5 h6
	address strong em tt ins del sub sup hr small big
	) )
{
	eval "sub $tag { unshift \@_, '$tag'; goto &WebTK::_autoChild; }";
}

sub text
{
	my $node = shift;
	my $text = $node->ownerDocument->createTextNode( shift );
	$node->addChild( $text );
	return $text;
}

1;

# vim: ts=4:sw=4
