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

/* */
function error( msg )
{
	window.alert( "Error: " + msg );
}


var tk_func = {};

function register_tk_class( cl, func )
{
	tk_func[ cl ] = func;
}

function check_node( node )
{
	var cl = node.getAttribute( "class" );
	if ( !cl )
		return;

	var cls = cl.split( /\s+/ );
	for ( var i = 0; i < cls.length; i++ ) {
		var cl = cls[ i ];
		if ( !cl.match( /^tk_/ ) )
			continue;

		var func = tk_func[ cl ];
		if ( !func ) {
			error( "No function for " + cl );
			continue;
		}
		func( node );
	}
}

function crawl_nodes( node )
{
	if ( ! node )
		return;

	check_node( node );

	for ( var i = 0; i < node.childNodes.length; i++ ) {
		var n = node.childNodes[ i ];
		if ( !n || !n.nodeName || n.nodeName == '#text' )
			continue;
		crawl_nodes( n );
	}
}

function start()
{
	crawl_nodes( document.getElementsByTagName("body")[0] );
}
window.addEventListener( 'load', start, false );

var ajax = {
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
			tree.update( req.responseXML );
		} catch ( e ) {
			error( "ajax.complete() error: " + e );
		}
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
	tick: function ()
	{
		ajax._timeStart++;
		if ( ajax._timeStart > 20 ) {
			ajax._watchdogRestart();
		} else if ( ajax._idle && ajax._timeStart > 3 ) {
			ajax.send( "", ajax._compose() );
		}
	},

	_init: null,
	_data: new Array(),
	push: function ( id, value )
	{
		ajax._data.push( id + '=' + value );
	},
	_compose: function ()
	{
		if ( ! ajax._init )
			ajax._init = init();

		ajax._data.unshift( ajax._init );

		var str = ajax._data.join( "&" );
		ajax._data = new Array();

		return str;
	},

};

window.setInterval( ajax.tick, 100 );


/* tk_click {{{ :*/
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
register_tk_class( "tk_click", register_tk_click );
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
register_tk_class( "tk_rclick", register_tk_rclick );
/* }}} */

/* vim: ts=4:sw=4:fdm=marker
 */
