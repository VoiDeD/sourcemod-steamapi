

#include "extension.h"

#include "steam/steam_gameserver.h"


static cell_t Native_SetGameDescription( IPluginContext *pContext, const cell_t *params )
{
	ISteamGameServer *pGameServer = g_APIContext.SteamGameServer();

	if ( !pGameServer )
	{
		return pContext->ThrowNativeError( "SteamAPI not initialized!" );
	}

	char *szGameDesc;
	pContext->LocalToString( params[ 1 ], &szGameDesc );

	pGameServer->SetGameDescription( szGameDesc );

	return 0;
}


sp_nativeinfo_t g_Natives[] =
{
	{ "SteamAPI_SetGameDescription", Native_SetGameDescription },
	{ NULL, NULL }
};
