

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

static cell_t Native_GetClientSteamID( IPluginContext *pContext, const cell_t *params )
{
	const CSteamID *pSteamID = engine->GetClientSteamIDByPlayerIndex( params[ 1 ] );

	if ( !pSteamID )
	{
		return pContext->ThrowNativeError( "Invalid or unconnected player!" );
	}

	char *steamIdBuffer;
	pContext->LocalToString( params[ 2 ], &steamIdBuffer );

	int numBytes = g_pSM->Format( steamIdBuffer, params[ 3 ], "%llu", pSteamID->ConvertToUint64() );
	numBytes++; // account for null terminator
	
	return numBytes;
}


sp_nativeinfo_t g_Natives[] =
{
	{ "SteamAPI_SetGameDescription", Native_SetGameDescription },
	{ "SteamAPI_GetClientSteamID", Native_GetClientSteamID },
	{ NULL, NULL }
};
