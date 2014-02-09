
#include "extension.h"

#include "steam/steam_gameserver.h"


class CHttpRequest
{

public:
	CHttpRequest( const char *szUrl, EHTTPMethod eMethod, IPluginContext *pContext );
	~CHttpRequest();

public:
	bool Send( IPluginFunction *pCallbackFunc );
	bool IsValid();

	Handle_t GetPluginHandle() { return m_PluginHandle; }


private:
	void OnHttpRequestCompleted( HTTPRequestCompleted_t *pResult, bool bError );

private:
	Handle_t m_PluginHandle;
	HTTPRequestHandle m_ReqHandle;

	CCallResult<CHttpRequest, HTTPRequestCompleted_t> m_CallResult;

	IPluginFunction *m_pPluginCallback;
};


CHttpRequest::CHttpRequest( const char *szUrl, EHTTPMethod eMethod, IPluginContext *pContext )
{
	m_CallResult.SetGameserverFlag();

	m_ReqHandle = g_APIContext.SteamHTTP()->CreateHTTPRequest( eMethod, szUrl );

	if ( m_ReqHandle == INVALID_HTTPREQUEST_HANDLE )
	{
		pContext->ThrowNativeError( "Unable to create HTTP request" );
		return;
	}

	m_PluginHandle = handlesys->CreateHandle( g_SteamAPI.GetHttpHandleType(), this, pContext->GetIdentity(), myself->GetIdentity(), NULL );

	if ( m_PluginHandle == BAD_HANDLE )
	{
		g_APIContext.SteamHTTP()->ReleaseHTTPRequest( m_ReqHandle );
		m_ReqHandle = INVALID_HTTPREQUEST_HANDLE;

		pContext->ThrowNativeError( "Unable to create HTTP request handle" );
		return;
	}
}

CHttpRequest::~CHttpRequest()
{
	if ( m_ReqHandle != INVALID_HTTPREQUEST_HANDLE )
	{
		g_APIContext.SteamHTTP()->ReleaseHTTPRequest( m_ReqHandle );
	}
}

bool CHttpRequest::Send( IPluginFunction *pCallbackFunc )
{
	Assert( IsValid() );

	m_pPluginCallback = pCallbackFunc;

	SteamAPICall_t apiCall;
	bool bRet = g_APIContext.SteamHTTP()->SendHTTPRequest( m_ReqHandle, &apiCall );

	if ( bRet )
	{
		m_CallResult.Set( apiCall, this, &CHttpRequest::OnHttpRequestCompleted );
	}

	return bRet;
}


bool CHttpRequest::IsValid()
{
	if ( m_ReqHandle == INVALID_HTTPREQUEST_HANDLE )
		return false;

	if ( m_PluginHandle == BAD_HANDLE )
		return false;

	return true;
}

void CHttpRequest::OnHttpRequestCompleted( HTTPRequestCompleted_t *pResult, bool bError )
{
	if ( bError )
	{
		// a fairly exceptional error occurred, so there isn't much we can do
		// ie: steam process went away, ipc pipe died, etc
		return;
	}

	m_pPluginCallback->PushCell( m_PluginHandle );
	m_pPluginCallback->PushCell( pResult->m_bRequestSuccessful );
	m_pPluginCallback->PushCell( pResult->m_eStatusCode );

	m_pPluginCallback->Execute( NULL );
}



bool CSteamAPI::InitHttp()
{
	m_RequestHandleType = handlesys->CreateType( "HttpRequest", this, 0, NULL, NULL, myself->GetIdentity(), NULL );

	if ( !m_RequestHandleType )
	{
		g_pSM->LogError( myself, "Unable to create HttpRequest handle type" );
		return false;
	}

	extern sp_nativeinfo_t g_HttpNatives[];
	sharesys->AddNatives( myself, g_HttpNatives );

	return true;
}

void CSteamAPI::ShutdownHttp()
{
	if ( m_RequestHandleType )
	{
		handlesys->RemoveType( m_RequestHandleType, myself->GetIdentity() );
	}
}

void CSteamAPI::OnHandleDestroy( HandleType_t type, void *object )
{
	if ( type != m_RequestHandleType )
		return;

	CHttpRequest *pReq = reinterpret_cast<CHttpRequest *>( object );
	delete pReq;
}

static cell_t Native_CreateHttpRequest( IPluginContext *pContext, const cell_t *params )
{
	ISteamHTTP *pHttp = g_APIContext.SteamHTTP();

	if ( !pHttp )
		return pContext->ThrowNativeError( "SteamAPI not initialized!" );

	char *szUrl;
	pContext->LocalToString( params[ 1 ], &szUrl );

	CHttpRequest *pReq = new CHttpRequest( szUrl, static_cast<EHTTPMethod>( params[ 2 ] ), pContext );

	if ( !pReq->IsValid() )
	{
		delete pReq;
		return BAD_HANDLE;
	}

	return pReq->GetPluginHandle();
}

static cell_t Native_SendHttpRequest( IPluginContext *pContext, const cell_t *params )
{
	ISteamHTTP *pHttp = g_APIContext.SteamHTTP();

	if ( !pHttp )
		return pContext->ThrowNativeError( "SteamAPI not initialized!" );

	HandleSecurity handleSec( pContext->GetIdentity(), myself->GetIdentity() );

	CHttpRequest *pReq = NULL;
	HandleError error = handlesys->ReadHandle( params[ 1 ], g_SteamAPI.GetHttpHandleType(), &handleSec, (void **)&pReq );

	if ( error != HandleError_None )
	{
		return pContext->ThrowNativeError( "Invalid HTTP request handle %x (error %d)", params[ 1 ], error );
	}

	IPluginFunction *pCallbackFunc = pContext->GetFunctionById( params[ 2 ] );

	return pReq->Send( pCallbackFunc );
}


sp_nativeinfo_t g_HttpNatives[] =
{
	{ "SteamAPI_CreateHttpRequest", Native_CreateHttpRequest },
	{ "SteamAPI_SendHttpRequest", Native_SendHttpRequest },
	{ NULL, NULL },
};