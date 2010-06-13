# This file is part of WebTK.
#
# 2010 (c) Przemysław Iskra <sparky@pld-linux.org>
#	This program is free software,
# you may distribute it under GPL v2 or newer.

package WebTK;

use warnings;
use strict;
use XML::LibXML ();
use Scalar::Util qw(weaken);
use List::Util qw(first);

my %session;

# current session id
my $sid;
# ids in current session
my $ids;


sub process
{
	my $post = shift;

	my @post = split /&/, $post;
	die "WebTK::process: No post" unless @post;
	my $_sid = shift @post;
	$_sid =~ s/^session=// or die "WebTK::process: No session";

	die "WebTK::process: Session expired" unless $session{ $_sid };

	$sid = $_sid;
	my $session = $session{ $sid };
	$ids = $session->{ids};

	while ( $_ = shift @post ) {
		s/^(tk_.+?)=//;
		my $id = $1;
		my $el = $ids->{ $id };
		unless ( $el ) {
			warn "Element with id '$id' cannot be found\n";
			next;
		}
		my $ev = $el->{ $_ };
		unless ( $ev ) {
			warn "Element '$id' has no event $_\n";
			next;
		}

		local $_ = $el->{node};
		@_ = @$ev;
		my $func = shift @_;
		die "Callback is not code\n"
			unless ref $func eq "CODE";
		&$func;
	}

	return $session->{doc}->serialize( 0 );
}

sub new
{
	my $init = shift;

	my $session = WebTK::_init();
	$sid = $session->{sid};
	$ids = $session->{ids};

	&$init( $session->{body} );

	return $session->{doc}->serialize( 0 );
}

my $docCache;
sub _doc
{
	if ( $docCache ) {
		return $docCache->cloneNode( 1 );
	}

	my $doc = XML::LibXML::Document->new( '1.0', 'utf-8' );
	$doc->createInternalSubset(
		'html',
		'-//W3C//DTD XHTML 1.1//EN',
		'http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd'
	);

	$doc->addChild( XML::LibXML::Comment->new( " WebTK 0.1 " ) );

	my $root = $doc->createElementNS( "http://www.w3.org/1999/xhtml", "html" );
	$doc->setDocumentElement( $root );

	$root->setAttributeNS( "http://www.w3.org/XML/1998/namespace", "lang" => "en" );

	my $head = $root->autoChild( 'head' );
	$head->autoChild( 'title' )->text( 'WebTK' );
	$head->autoChild( 'link',
		rel => "stylesheet",
		type => "text/css",
		href => "_webtk/screen.css",
		media => "screen" );
	$head->autoChild( 'link',
		rel => "shortcut icon",
		href => "_webtk/favicon.png" );
	$head->autoChild( 'script',
		type => "text/javascript",
		src => "_webtk/interface.js" );

	$docCache = $doc;
	return $docCache->cloneNode( 1 );
}

sub _init
{
	do {
		$sid = sprintf "tk_%04x%04x", rand 1 << 16, rand 1 << 16;
	} while ( exists $session{ $sid } );

	$ids = {};

	my $doc = _doc();
	my $root = $doc->lastChild;

	# body id is the session identified, each user has different body id
	my $body = $root->autoChild( 'body', id => $sid );
	my $obj = $ids->{ $sid } = {};
	weaken( $obj->{node} = $body );

	return $session{ $sid } = {
		ids => $ids,
		sid => $sid,
		doc => $doc,
		start => time,
		body => $body,
	};
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
	} while ( exists $ids->{ $id } );
	$el->setAttribute( "id", $id );
	return $id;
}

sub button
{
	my $tkclass = shift;
	my $el = shift;

	my $id = _id( $el );
	_addClass( $el, $tkclass );

	my $obj = $ids->{ $id } ||= {};
	weaken( $obj->{node} = $el );

	die "First button argument must be code reference (callback)\n"
		unless ref $_[0] and ref $_[0] eq "CODE";

	$obj->{$tkclass} = \@_;
}

sub dynamic
{
	my $el = shift;

	my $id = _id( $el );
	#_addClass( $el, "tk_dynamic" );

	my $obj = $ids->{ $id } ||= {};
	weaken( $obj->{node} = $el );
}

sub _remove
{
	my $node = shift;
	my $id = $node->getAttribute( "id" );

	$node->parentNode->removeChild( $node );
	delete $ids->{ $id };
}

sub _fixStyle
{
	my $hash = shift;
	return join " ", map { $_ . ":" . $hash->{$_} .
		($hash->{$_} =~ m/^-?\d+$/ and $hash->{$_} != 0 ? "px" : "") . ";" } keys %$hash;
}

sub _autoChild
{
	my $node = shift;
	my $tag = shift;
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

sub id
{
	return (shift)->getAttribute( "id" );
}

*addClass = \&WebTK::_addClass;


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

*autoChild = \&WebTK::_autoChild;

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
	eval "sub $tag { splice \@_, 1, 0, '$tag'; goto &WebTK::_autoChild; }";
}

sub text
{
	my $node = shift;
	my $text = $node->ownerDocument->createTextNode( shift );
	$node->addChild( $text );
	return $text;
}

*remove = \&WebTK::_remove;

1;

# vim: ts=4:sw=4:fdm=marker
