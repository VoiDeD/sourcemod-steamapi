/**
 * vim: set ts=4 :
 * =============================================================================
 * SteamAPI Extension
 * Copyright (C) 2004-2008 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, AlliedModders LLC gives you permission to link the
 * code of this program (as well as its derivative works) to "Half-Life 2," the
 * "Source Engine," the "SourcePawn JIT," and any Game MODs that run on software
 * by the Valve Corporation.  You must obey the GNU General Public License in
 * all respects for all other code used.  Additionally, AlliedModders LLC grants
 * this exception to all derivative works.  AlliedModders LLC defines further
 * exceptions, found in LICENSE.txt (as of this writing, version JULY-31-2007),
 * or <http://www.sourcemod.net/license.php>.
 *
 * Version: $Id$
 */

#include "extension.h"

#include "steam/steam_gameserver.h"

#include "CDetour/detours.h"

#include "tier1/strtools.h"
#include "tier1/interface.h"


CSteamAPI g_SteamAPI;		/**< Global singleton for extension's main interface */
SMEXT_LINK( &g_SteamAPI );

CSteamGameServerAPIContext g_APIContext;


DETOUR_DECL_STATIC6( SteamGameServer_InitSafeDetour, bool, uint32, unIP, uint16, usSteamPort, uint16, usGamePort, uint16, usQueryPort, EServerMode, eServerMode, const char *, pchVersionString )
{
	g_SteamAPI.Init();

	return DETOUR_STATIC_CALL( SteamGameServer_InitSafeDetour )( unIP, usSteamPort, usGamePort, usQueryPort, eServerMode, pchVersionString );
}


bool CSteamAPI::SDK_OnLoad( char *error, size_t maxlength, bool late )
{
	extern sp_nativeinfo_t g_Natives[];
	sharesys->AddNatives( myself, g_Natives );

	CDetourManager::Init( g_pSM->GetScriptingEngine(), NULL );

	extern void *Sys_GetProcAddress( const char *pModuleName, const char *pName );
	void *pFuncInitSafe = Sys_GetProcAddress( "steam_api", "SteamGameServer_InitSafe" );

	if ( pFuncInitSafe == NULL )
	{
		// the world is probably ending
		V_snprintf( error, maxlength, "Unable to find SteamGameServer_InitSafe" );
		return false;
	}

	m_pInitDetour = DETOUR_CREATE_STATIC_FIXED( SteamGameServer_InitSafeDetour, pFuncInitSafe );
	m_pInitDetour->EnableDetour();

	return true;
}

void CSteamAPI::SDK_OnUnload()
{
	if ( m_pInitDetour != NULL )
	{
		m_pInitDetour->Destroy();
		m_pInitDetour = NULL;
	}
}

void CSteamAPI::Init()
{
	if ( !g_APIContext.Init() )
	{
		g_pSM->LogError( myself, "Unable to initialize SteamAPI context!" );
		return;
	}
}
