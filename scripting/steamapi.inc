
#if defined __steamapi_included
	#endinput
#endif

#define __steamapi_included


enum HTTPMethod
{
	HTTPMethod_Invalid = 0,
	HTTPMethod_GET,
	HTTPMethod_HEAD,
	HTTPMethod_POST,
};

enum HTTPStatusCode
{
	HTTPStatusCode_Invalid =						0,

	// Informational codes
	HTTPStatusCode_Continue =						100,
	HTTPStatusCode_SwitchingProtocols =				101,

	// Success codes
	HTTPStatusCode_OK =								200,
	HTTPStatusCode_Created =						201,
	HTTPStatusCode_Accepted =						202,
	HTTPStatusCode_NonAuthoritative =				203,
	HTTPStatusCode_NoContent =						204,
	HTTPStatusCode_ResetContent =					205,
	HTTPStatusCode_PartialContent =					206,

	// Redirection codes
	HTTPStatusCode_MultipleChoices =				300,
	HTTPStatusCode_MovedPermanently =				301,
	HTTPStatusCode_Found =							302,
	HTTPStatusCode_SeeOther =						303,
	HTTPStatusCode_NotModified =					304,
	HTTPStatusCode_UseProxy =						305,
	HTTPStatusCode_TemporaryRedirect =				307,

	// Error codes
	HTTPStatusCode_BadRequest =						400,
	HTTPStatusCode_Unauthorized =					401,
	HTTPStatusCode_PaymentRequired =				402,
	HTTPStatusCode_Forbidden =						403,
	HTTPStatusCode_NotFound =						404,
	HTTPStatusCode_MethodNotAllowed =				405,
	HTTPStatusCode_NotAcceptable =					406,
	HTTPStatusCode_ProxyAuthRequired =				407,
	HTTPStatusCode_RequestTimeout =					408,
	HTTPStatusCode_Conflict =						409,
	HTTPStatusCode_Gone =							410,
	HTTPStatusCode_LengthRequired =					411,
	HTTPStatusCode_PreconditionFailed =				412,
	HTTPStatusCode_RequestEntityTooLarge =			413,
	HTTPStatusCode_RequestURITooLong =				414,
	HTTPStatusCode_UnsupportedMediaType =			415,
	HTTPStatusCode_RequestedRangeNotSatisfiable =	416,
	HTTPStatusCode_ExpectationFailed =				417,
	
	// Server error codes
	HTTPStatusCode_InternalServerError =			500,
	HTTPStatusCode_NotImplemented =					501,
	HTTPStatusCode_BadGateway =						502,
	HTTPStatusCode_ServiceUnavailable =				503,
	HTTPStatusCode_GatewayTimeout =					504,
	HTTPStatusCode_HTTPVersionNotSupported =		505,
};


native SteamAPI_SetGameDescription( const String:gameDesc[] );


funcenum HttpReqCompletedCallback
{
	public( Handle:httpRequest, bool:bRequestSuccessful, HTTPStatusCode:statusCode ),
	public( Handle:httpRequest, bool:bRequestSuccessful, HTTPStatusCode:statusCode, any:context ),
};

native Handle:SteamAPI_CreateHttpRequest( const String:url[], HTTPMethod:method = HTTPMethod_GET, any:context = 0 );
native bool:SteamAPI_SendHttpRequest( Handle:httpRequest, HttpReqCompletedCallback:callback );

native SteamAPI_GetHttpResponseBodySize( Handle:httpRequest );
native SteamAPI_GetHttpResponseBody( Handle:httpRequest, String:buffer[], maxBuffer );

forward SteamAPI_RestartRequested();


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
