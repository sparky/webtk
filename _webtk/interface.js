/*
 * This file is part of WebTK.
 *
 * 2010 (c) Przemysław Iskra <sparky@pld-linux.org>
 *	This program is free software,
 * you may distribute it under GPL v2 or newer.
 */

/* missing extensions {{{ */
if (Array.prototype.indexOf)
{
	Array.prototype.exists = function( x )
	{
		if ( this.indexOf( x ) >= 0 )
			return true;
		return false;
	};
} else {
	Array.prototype.exists = function( x )
	{
		for ( var i = 0; i < this.length; i++ ) {
			if ( this[ i ] == x )
				return true;
		}
		return false;
	};

	Array.prototype.indexOf = function( el )
	{
		var len = this.length;

		var from = Number(arguments[1]) || 0;
		from = (from < 0)
			? Math.ceil(from)
			: Math.floor(from);
		if (from < 0)
			from += len;
		for (; from < len; from++) {
			if (from in this && this[from] === el)
				return from;
		}
		return -1;
	};
}
/* }}} */

/* own extensions {{{ */
HTMLElement.prototype.addClass = function ( acl )
{
	var cls = this.getAttribute( 'class' );
	if ( !cls )
		return this.setAttribute( 'class', acl );

	if ( cls.indexOf( acl ) >= 0 )
		return;

	return this.setAttribute( 'class', cls + " " + acl );
};
/* }}} */

var xmlns = "http://www.w3.org/1999/xhtml";
function error( msg )
{
	var board = document.getElementById( 'tk_errors' );
	if ( ! board ) {
		var body = document.getElementsByTagName( 'body' )[0];
		board = document.createElementNS( xmlns, 'div' );
		board.setAttribute( 'id', 'tk_errors' );
		body.appendChild( board );
	}

	var p = document.createElementNS( xmlns, 'p' );
	p.appendChild( document.createTextNode( msg ) );
	board.appendChild( p );
	window.setTimeout( _error_rm, 1000, p );
}
function _error_rm( node )
{
	var board = node.parentNode;
	board.removeChild( node );
	if ( ! board.firstChild )
		board.parentNode.removeChild( board );
}

/* tree: dom tree processing methods {{{ */
var tree = {
	_func: {},
	register: function register_tk_class( cl, func )
	{
		tree._func[ cl ] = func;
	},
	_check: function ( node )
	{
		var cl = node.getAttribute( "class" );
		if ( !cl )
			return;

		var cls = cl.split( /\s+/ );
		for ( var i = 0; i < cls.length; i++ ) {
			var cl = cls[ i ];
			if ( !cl.match( /^tk_/ ) )
				continue;

			var func = tree._func[ cl ];
			if ( !func ) {
				error( "No function for " + cl );
				continue;
			}
			func( node );
		}
	},
	_climb: function ( node )
	{
		if ( ! node )
			return;

		tree._check( node );

		for ( var i = 0; i < node.childNodes.length; i++ ) {
			var n = node.childNodes[ i ];
			if ( !n || !n.nodeName || n.nodeName == '#text' )
				continue;
			tree._climb( n );
		}
	},
	update: function ( node )
	{
		var id = node.getAttribute( 'id' );

		if ( !id ) {
			/* node has no id -> check all its children */
			var c;
			while ( c = node.firstChild ) {
				node.removeChild( c );
	
				tree.update( c );
			}
			return;
		}

		var old = document.getElementById( id );
		if ( !old ) {
			error( "Node with id '" + id + "' does not exist" );
			return;
		}

		/* replace original with new one */
		/* adapt node ? import node ? */
		old.parentNode.replaceChild( node, old );
		tree._climb( node );
		return;
	},
	init: function ()
	{
		tree._climb( document.getElementsByTagName("body")[0] );
	}
};
/* }}} */


/* ajax: data exchange {{{ */
var ajax = {
	_uri: "einstein", // XXX: temporary
	_post: null,
	_idle: true,
	_timeStart: 5,
	_req: new XMLHttpRequest(),
	_recv: function ()
	{
		var req = ajax._req;

		if ( req.readyState != 4 )
			return;

		var status;
		try {
			status = req.status;
		} catch (e) {
			status = -1;
		}

		if ( status != 200 )
			return;

		if ( ! req.responseXML ||
				! ( req.responseXML.lastChild instanceof HTMLHtmlElement ) )
			return;

		/* confirm watchdog */
		ajax._post = null;
		ajax._idle = true;

		try {
			ajax._complete( req );
		} catch ( e ) {
			error( "ajax.complete() error: " + e );
		}
	},
	_complete: function ( req )
	{
		var xml = req.responseXML;
		if ( !xml ) {
			var parser = new DOMParser();
			xml = parser.parseFromString( req.responseText, "text/xml" );
			if ( !xml )
				throw "Cannot parse server response";
		}
		tree.update( /* body */ xml.lastChild.lastChild );
	},
	_send: function()
	{
		var req = ajax._req;

		if ( req.readyState != 4 && req.readyState != 0 ) {
			error( "Ajax request already in progress" );
			return;
		}

		req.onreadystatechange = ajax._recv;
		req.open( "POST", ajax._uri, true );
		req.setRequestHeader( 'Content-Type', 'application/x-www-form-urlencoded' );
		req.setRequestHeader( 'Content-Length', ajax._post.length );
		req.setRequestHeader( 'Connection', 'close' );
		req.send( ajax._post );

		ajax._timeStart = 0;
		ajax._idle = false;
	},
	_watchdogRestart: function ()
	{
		var req = ajax._req;
		if ( req.readyState != 4 && req.readyState != 0 )
			req.abort();

		ajax._req = new XMLHttpRequest();

		if ( ajax._post )
			ajax._send( );
	},
	_tick: function ()
	{
		ajax._timeStart++;
		if ( ajax._timeStart > 20 ) {
			ajax._watchdogRestart();
		} else if ( ajax._idle && ajax._timeStart > 5 ) {
			ajax._post = ajax._compose();
			ajax._send( );
		}
	},

	_data: new Array(),
	push: function ( id, value )
	{
		ajax._data.push( id + '=' + value );
		if ( ajax._idle && ajax._timeStart < 4 )
			ajax._timeStart = 4;
	},
	_compose: function ()
	{
		var body = document.getElementsByTagName("body")[0];
		var sid = body.getAttribute( 'id' );

		ajax._data.unshift( 'session=' + sid );

		var str = ajax._data.join( "&" );
		ajax._data = new Array();

		return str;
	},
	init: function ()
	{
		window.setInterval( ajax._tick, 100 );
	}
};
/* }}} */

function init()
{
	tree.init();
	ajax.init();
}

window.addEventListener( 'load', init, false );


/* tk_click {{{ */
function callback_tk_click( e )
{
	e.preventDefault();
	e.stopPropagation();

	ajax.push( this.getAttribute( 'id' ), 'tk_click' );
	this.addClass( "tk_click_active" );
}

function register_tk_click( node )
{
	node.addEventListener( "click", callback_tk_click, false );
}
tree.register( "tk_click", register_tk_click );
/* }}} */


/* tl_rclick {{{ */
function callback_tk_rclick( e )
{
	e.preventDefault();
	e.stopPropagation();

	ajax.push( this.getAttribute( 'id' ), 'tk_rclick' );
	this.addClass( "tk_rclick_active" );
}

function register_tk_rclick( node )
{
	node.addEventListener( "contextmenu", callback_tk_rclick, false );
}
tree.register( "tk_rclick", register_tk_rclick );
/* }}} */

/* vim: ts=4:sw=4:fdm=marker
 */
