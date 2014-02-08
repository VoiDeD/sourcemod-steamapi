
#include "extension.h"

#include "sourcehook.h"

#include "steam/steam_gameserver.h"


SH_DECL_HOOK0( ISteamGameServer, WasRestartRequested, SH_NOATTRIB, 0, bool );


IForward *g_pRestartReqFwd = NULL;


void CSteamAPI::InitForwards()
{
	g_pRestartReqFwd = forwards->CreateForward( "SteamAPI_RestartRequested", ET_Event, 0, NULL );

	SH_ADD_HOOK( ISteamGameServer, WasRestartRequested, g_APIContext.SteamGameServer(), SH_MEMBER( this, &CSteamAPI::WasRestartRequested ), false );
}

void CSteamAPI::ShutdownForwards()
{
	SH_REMOVE_HOOK( ISteamGameServer, WasRestartRequested, g_APIContext.SteamGameServer(), SH_MEMBER( this, &CSteamAPI::WasRestartRequested ), false );

	forwards->ReleaseForward( g_pRestartReqFwd );
}


bool CSteamAPI::WasRestartRequested()
{
	g_pRestartReqFwd->Execute( NULL );

	RETURN_META_VALUE( MRES_IGNORED, false );
}
