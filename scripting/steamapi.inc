
#if defined __steamapi_included
	#endinput
#endif

#define __steamapi_included


native SteamAPI_SetGameDescription( const String:gameDesc[] );


public Extension:__ext__steamapi =
{
	name = "SteamAPI",
	file = "steamapi.ext",

#if defined AUTOLOAD_EXTENSIONS
	autoload = 1,
#else
	autoload = 0,
#endif

#if defined REQUIRE_EXTENSIONS
	required = 1,
#else
	required = 0,
#endif

};


#if !defined REQUIRE_EXTENSIONS
public __ext_steamapi_SetNTVOptional()
{
	MarkNativeAsOptional( "SteamAPI_SetGameDescription" );
}
