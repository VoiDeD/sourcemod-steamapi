
#include "extension.h"

#include "steam/steam_gameserver.h"


class CHttpRequest
{

public:
	CHttpRequest( const char *szUrl, EHTTPMethod eMethod, IPluginContext *pContext );
	~CHttpRequest();

	static CHttpRequest *GetRequestFromHandle( IPluginContext *pContext, cell_t handle );

public:
	bool IsValid();

	bool SetContext( uint32 uiContextVal );
	bool Send( IPluginFunction *pCallbackFunc );

	bool GetResponseBodySize( uint32 *bodySize );
	bool GetResponseBody( uint8 *pBuffer, uint32 maxBuffer );


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
	: m_pPluginCallback( NULL )
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

CHttpRequest *CHttpRequest::GetRequestFromHandle( IPluginContext *pContext, cell_t handle )
{
	HandleSecurity handleSec( pContext->GetIdentity(), myself->GetIdentity() );

	CHttpRequest *pReq = NULL;
	HandleError error = handlesys->ReadHandle( handle, g_SteamAPI.GetHttpHandleType(), &handleSec, (void **)&pReq );

	if ( error != HandleError_None )
	{
		pContext->ThrowNativeError( "Invalid HTTP request handle %x (error %d)", handle, error );
		return NULL;
	}

	return pReq;
}


bool CHttpRequest::IsValid()
{
	if ( m_ReqHandle == INVALID_HTTPREQUEST_HANDLE )
		return false;

	if ( m_PluginHandle == BAD_HANDLE )
		return false;

	return true;
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

bool CHttpRequest::SetContext( uint32 contextVal )
{
	Assert( IsValid() );

	return g_APIContext.SteamHTTP()->SetHTTPRequestContextValue( m_ReqHandle, contextVal );
}

bool CHttpRequest::GetResponseBodySize( uint32 *bodySize )
{
	Assert( IsValid() );

	return g_APIContext.SteamHTTP()->GetHTTPResponseBodySize( m_ReqHandle, bodySize );
}

bool CHttpRequest::GetResponseBody( uint8 *buffer, uint32 maxBuffer )
{
	Assert( IsValid() );

	return g_APIContext.SteamHTTP()->GetHTTPResponseBodyData( m_ReqHandle, buffer, maxBuffer );
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
	m_pPluginCallback->PushCell( pResult->m_ulContextValue );

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

	pReq->SetContext( params[ 3 ] );

	return pReq->GetPluginHandle();
}

static cell_t Native_SendHttpRequest( IPluginContext *pContext, const cell_t *params )
{
	ISteamHTTP *pHttp = g_APIContext.SteamHTTP();

	if ( !pHttp )
		return pContext->ThrowNativeError( "SteamAPI not initialized!" );

	CHttpRequest *pReq = CHttpRequest::GetRequestFromHandle( pContext, params[ 1 ] );

	if ( pReq == NULL )
		return 0;

	IPluginFunction *pCallbackFunc = pContext->GetFunctionById( params[ 2 ] );

	return pReq->Send( pCallbackFunc );
}

static cell_t Native_GetHttpResponseBodySize( IPluginContext *pContext, const cell_t *params )
{
	ISteamHTTP *pHttp = g_APIContext.SteamHTTP();

	if ( !pHttp )
		return pContext->ThrowNativeError( "SteamAPI not initialized!" );

	CHttpRequest *pReq = CHttpRequest::GetRequestFromHandle( pContext, params[ 1 ] );

	if ( pReq == NULL )
		return 0;

	uint32 bodySize;

	if ( !pReq->GetResponseBodySize( &bodySize ) )
		return pContext->ThrowNativeError( "Unable to get HTTP response body size" );

	return bodySize;
}

static cell_t Native_GetHttpResponseBody( IPluginContext *pContext, const cell_t *params )
{
	ISteamHTTP *pHttp = g_APIContext.SteamHTTP();

	if ( !pHttp )
		return pContext->ThrowNativeError( "SteamAPI not initialized!" );

	CHttpRequest *pReq = CHttpRequest::GetRequestFromHandle( pContext, params[ 1 ] );

	if ( pReq == NULL )
		return 0;

	char *buffer;
	pContext->LocalToString( params[ 2 ], &buffer );

	if ( !pReq->GetResponseBody( (uint8 *)buffer, params[ 3 ] ) )
		return pContext->ThrowNativeError( "Unable to get HTTP response body" );

	return 0;
}


sp_nativeinfo_t g_HttpNatives[] =
{
	{ "SteamAPI_CreateHttpRequest", Native_CreateHttpRequest },
	{ "SteamAPI_SendHttpRequest", Native_SendHttpRequest },

	{ "SteamAPI_GetHttpResponseBodySize", Native_GetHttpResponseBodySize },
	{ "SteamAPI_GetHttpResponseBody", Native_GetHttpResponseBody },

	{ NULL, NULL },
};
