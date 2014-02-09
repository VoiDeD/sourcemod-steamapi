
#if defined __steamapi_included
	#endinput
#endif

#define __steamapi_included


// This enum is used in client API methods, do not re-number existing values.
enum EHTTPMethod
{
	k_EHTTPMethodInvalid = 0,
	k_EHTTPMethodGET,
	k_EHTTPMethodHEAD,
	k_EHTTPMethodPOST,
	k_EHTTPMethodPUT,
	k_EHTTPMethodDELETE,
	k_EHTTPMethodOPTIONS,

	// The remaining HTTP methods are not yet supported, per rfc2616 section 5.1.1 only GET and HEAD are required for 
	// a compliant general purpose server.  We'll likely add more as we find uses for them.

	// k_EHTTPMethodTRACE,
	// k_EHTTPMethodCONNECT
};

// HTTP Status codes that the server can send in response to a request, see rfc2616 section 10.3 for descriptions
// of each of these.
enum EHTTPStatusCode
{
	// Invalid status code (this isn't defined in HTTP, used to indicate unset in our code)
	k_EHTTPStatusCodeInvalid =					0,

	// Informational codes
	k_EHTTPStatusCode100Continue =				100,
	k_EHTTPStatusCode101SwitchingProtocols =	101,

	// Success codes
	k_EHTTPStatusCode200OK =					200,
	k_EHTTPStatusCode201Created =				201,
	k_EHTTPStatusCode202Accepted =				202,
	k_EHTTPStatusCode203NonAuthoritative =		203,
	k_EHTTPStatusCode204NoContent =				204,
	k_EHTTPStatusCode205ResetContent =			205,
	k_EHTTPStatusCode206PartialContent =		206,

	// Redirection codes
	k_EHTTPStatusCode300MultipleChoices =		300,
	k_EHTTPStatusCode301MovedPermanently =		301,
	k_EHTTPStatusCode302Found =					302,
	k_EHTTPStatusCode303SeeOther =				303,
	k_EHTTPStatusCode304NotModified =			304,
	k_EHTTPStatusCode305UseProxy =				305,
	//k_EHTTPStatusCode306Unused =				306, (used in old HTTP spec, now unused in 1.1)
	k_EHTTPStatusCode307TemporaryRedirect =		307,

	// Error codes
	k_EHTTPStatusCode400BadRequest =			400,
	k_EHTTPStatusCode401Unauthorized =			401,
	k_EHTTPStatusCode402PaymentRequired =		402, // This is reserved for future HTTP specs, not really supported by clients
	k_EHTTPStatusCode403Forbidden =				403,
	k_EHTTPStatusCode404NotFound =				404,
	k_EHTTPStatusCode405MethodNotAllowed =		405,
	k_EHTTPStatusCode406NotAcceptable =			406,
	k_EHTTPStatusCode407ProxyAuthRequired =		407,
	k_EHTTPStatusCode408RequestTimeout =		408,
	k_EHTTPStatusCode409Conflict =				409,
	k_EHTTPStatusCode410Gone =					410,
	k_EHTTPStatusCode411LengthRequired =		411,
	k_EHTTPStatusCode412PreconditionFailed =	412,
	k_EHTTPStatusCode413RequestEntityTooLarge =	413,
	k_EHTTPStatusCode414RequestURITooLong =		414,
	k_EHTTPStatusCode415UnsupportedMediaType =	415,
	k_EHTTPStatusCode416RequestedRangeNotSatisfiable = 416,
	k_EHTTPStatusCode417ExpectationFailed =		417,

	// Server error codes
	k_EHTTPStatusCode500InternalServerError =	500,
	k_EHTTPStatusCode501NotImplemented =		501,
	k_EHTTPStatusCode502BadGateway =			502,
	k_EHTTPStatusCode503ServiceUnavailable =	503,
	k_EHTTPStatusCode504GatewayTimeout =		504,
	k_EHTTPStatusCode505HTTPVersionNotSupported = 505,
};

// results from UserHasLicenseForApp
enum EUserHasLicenseForAppResult
{
	k_EUserHasLicenseResultHasLicense = 0,					// User has a license for specified app
	k_EUserHasLicenseResultDoesNotHaveLicense = 1,			// User does not have a license for the specified app
	k_EUserHasLicenseResultNoAuth = 2,						// User has not been authenticated
};


native SteamAPI_SetGameDescription( const String:gameDesc[] );

native SteamAPI_GetClientSteamID( client, String:buffer[], maxBuffer );

native EUserHasLicenseForAppResult:SteamAPI_UserHasLicenseForApp( client, nAppId );


funcenum HttpReqCompletedCallback
{
	public( Handle:httpRequest, bool:bRequestSuccessful, HTTPStatusCode:statusCode ),
	public( Handle:httpRequest, bool:bRequestSuccessful, HTTPStatusCode:statusCode, any:context ),
};

native Handle:SteamAPI_CreateHttpRequest( const String:url[], HTTPMethod:method = k_EHTTPMethodGET, any:context = 0 );
native bool:SteamAPI_SendHttpRequest( Handle:httpRequest, HttpReqCompletedCallback:callback );

native SteamAPI_GetHttpResponseBodySize( Handle:httpRequest );
native SteamAPI_GetHttpResponseBody( Handle:httpRequest, String:buffer[], maxBuffer );

forward SteamAPI_RestartRequested();

forward SteamAPI_OnClientValidated( accountId, ownerId );


stock SteamAPI_GetClientByAccountID( accountId, bool:validate = true )
{
	for ( new i = 1 ; i <= MaxClients ; ++i )
	{
		if ( !IsClientConnected( i ) )
			continue;

		new clientAccId = GetSteamAccountID( i, validate );

		if ( accountId == clientAccId )
			return i;
	}

	return -1;
}


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
	MarkNativeAsOptional( "SteamAPI_GetClientSteamID" );
	MarkNativeAsOptional( "SteamAPI_UserHasLicenseForApp" );

	MarkNativeAsOptional( "SteamAPI_CreateHttpRequest" );
	MarkNativeAsOptional( "SteamAPI_SendHttpRequest" );

	MarkNativeAsOptional( "SteamAPI_GetHttpResponseBodySize" );
	MarkNativeAsOptional( "SteamAPI_GetHttpResponseBody" );
}
