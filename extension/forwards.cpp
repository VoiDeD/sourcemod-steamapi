
#include "extension.h"

#include "sourcehook.h"

#include "steam/steam_gameserver.h"


SH_DECL_HOOK0( ISteamGameServer, WasRestartRequested, SH_NOATTRIB, 0, bool );


IForward *g_pRestartReqFwd = NULL;
IForward *g_pValidatedFwd = NULL;


void CSteamAPI::InitForwards()
{
	g_pRestartReqFwd = forwards->CreateForward( "SteamAPI_RestartRequested", ET_Ignore, 0, NULL );
	g_pValidatedFwd = forwards->CreateForward( "SteamAPI_OnClientValidated", ET_Ignore, 2, NULL, Param_Cell, Param_Cell );

	SH_ADD_HOOK( ISteamGameServer, WasRestartRequested, g_APIContext.SteamGameServer(), SH_MEMBER( this, &CSteamAPI::WasRestartRequested ), false );
}

void CSteamAPI::ShutdownForwards()
{
	SH_REMOVE_HOOK( ISteamGameServer, WasRestartRequested, g_APIContext.SteamGameServer(), SH_MEMBER( this, &CSteamAPI::WasRestartRequested ), false );

	forwards->ReleaseForward( g_pValidatedFwd );
	forwards->ReleaseForward( g_pRestartReqFwd );
}

void CSteamAPI::OnClientValidated( ValidateAuthTicketResponse_t *pCallback )
{
	if ( pCallback->m_eAuthSessionResponse != k_EAuthSessionResponseOK )
		return; // not a callback we're interested in

	g_pValidatedFwd->PushCell( pCallback->m_SteamID.GetAccountID() );
	g_pValidatedFwd->PushCell( pCallback->m_OwnerSteamID.GetAccountID() );

	g_pValidatedFwd->Execute( NULL );
}

bool CSteamAPI::WasRestartRequested()
{
	g_pRestartReqFwd->Execute( NULL );

	RETURN_META_VALUE( MRES_IGNORED, false );
}
