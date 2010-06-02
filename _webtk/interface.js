
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
	var cl = node.getAttribure( "class" );
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




function callback_tk_click( e )
{
	e.preventDefault();
	e.stopPropagation();

	alert( "click" );
}



function register_tk_click( node )
{
	node.addEventListener( "click", callback_tk_click, false );
}
register_tk_class( "tk_click", register_tk_click );



function register_tk_rclick( node )
{
	node.addEventListener( "contextmenu", callback_tk_click, false );
}
register_tk_class( "tk_rclick", register_tk_rclick );
