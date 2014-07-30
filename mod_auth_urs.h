/*
 * mod_auth_urs.h: URS OAuth2 Module
 *
 * Header declarations file for mod_auth_urs.
 *
 * Author: Peter Smith
 */

#include    "httpd.h"
#include    "http_config.h"
#include    "apr_tables.h"


extern module AP_MODULE_DECLARE_DATA auth_urs_module;



/****************************************
 * Server level declarations
 ***************************************/

/**
 * URS oauth2 module server level configuration structure.
 *
 */
typedef struct auth_urs_svr_config_t
{
    /**
     * The location of the directory in which to store session
     * data. All session data is stored here.
     */
    char*       session_store_path;

    /**
     * The address of the URS authentication server
     */
    apr_uri_t   urs_auth_server;

    /**
     * The path of the URS authentication request endpoint
     */
    char*       urs_auth_path;

    /**
     * The path of the URS token exchange endpoint
     */
    char*       urs_token_path;

    /**
     * A table of redirection URIs.
     *
     */
    apr_table_t* redirection_map;

} auth_urs_svr_config;




/**
 * URS oauth2 module directory level configuration structure.
 *
 */
typedef struct auth_urs_dir_config_t
{

    /**
     * Used as the name of the session cookie. This is based upon
     * the per-directory 'UrsAuthGroup' configuration, and permits
     * authentication of groups of applications on a single
     * server.
     */
    char*       authorization_group;

    /**
     * The client ID assigned when the application was registered
     * for this particular location.
     */
    char*       client_id;

    /**
     * The authorization code to be passed to the server. This
     * code embeds the password, so whatever file it resides in
     * should be restricted.
     */
    char*       authorization_code;

    /**
     * The name to user for anonymous access. If this is set,
     * anonymous access is enabled.
     */
    char*       anonymous_user;

    /**
     * The application redirection URL
     */
    apr_uri_t   redirect_url;

    /**
     * The idle timeout on a session. If a session has not
     * been used for this amount of time, it will be destroyed,
     * (forcing re-authentication). Set to 0 to disable.
     */
    long        idle_timeout;

    /**
     * The timeout on an active session. Set to 0 to
     * disable. This destroys a session after the given
     * time (in seconds), regardless of whether the session
     * is in use. Generally, this should be set to something
     * like 12 hours (43200) or 24 hours (86400).
     */
    long        active_timeout;

    /**
     * The number of parts of the IP4 address octets to check
     * as part of session verification. 0 disables.
     */
    int         check_ip_octets;

    /**
     *  Disables the URS Oauth2 splash screen
     */
    int         splash_disable;

    /**
     * A table of user profile parameters to save in the
     * sub-process environment.
     *
     */
    apr_table_t* user_profile_env;

    /**
     * The access error redirection URL
     */
    char*       access_error_url;


} auth_urs_dir_config;




/**
 * Early request processing hook designed to capture the redirection
 * that comes back from the authentication server. It checks to
 * see if the request is for a configured redirection URL
 * (UrsRedirectUrl directive in the directory level configuration).
 * If so, it extracts the URS authentication code and the state
 * query parameters, and redirects the user back to the original
 * page the requested when authentication was invoked. The URL
 * of the orignal request is encoded using the state query parameter.
 *
 * @param r a pointer to the request_rec structure
 * @return DECLINED or HTTP status code
 */
int auth_urs_post_read_request_redirect(request_rec* r);



/**
 * Early request processing hook designed to provide a logout
 * capability. This is intended to be transparent to the
 * request processing, so this method always returns the
 * DECLINE status.
 *
 * @param r a pointer to the request_rec structure
 * @return DECLINED
 */
int auth_urs_post_read_request_logout(request_rec* r);



/**
 * Checks to see whether URS OAuth2 type authentication should
 * be performed on the request. This is a hook callback method
 * invoked by apache as part of the request processing, and
 * performs the intial redirection as well as token exchange.
 *
 * @param r a pointer to the request structure for the
 *          currently active request.
 * @return DECLINED or HTTP status
 */
int auth_urs_check_user_id(request_rec* r);



/****************************************
 * JSON declarations
 ***************************************/


/**
 * JSON reference pointer used for handling json objects.
 */
typedef struct json json;


/**
 * JSON member type enumeration
 */
typedef enum
{
    json_string,
    json_number,
    json_object,
    json_array,
    json_boolean,
    json_null

} json_type;


/**
 * Parse a text string into a json object.
 * @param pool the pool to use for memory allocation
 * @param json_text the text to parse
 * @return a pointer to a json object, or NULL if the text
 *         could not be parsed.
 */
json* json_parse( apr_pool_t* pool, const char* json_text );


/**
 * Return whether or not the named json member exists.
 * @param json the json object to search
 * @param name the name of the member to test
 * @return true if the named member exists, false otherwise
 */
int json_has_member(json* json, const char* name );


/**
 * Return a named json member object.
 * @param json the json object to search
 * @param name the name of the member to be returned
 * @return a pointer to the json object, or NULL it the named
 *         member is not a json object.
 */
json* json_get_member_object(json* json, const char* name );


/**
 * Return the value of a named json member.
 * @param json the json object to search
 * @param name the name of the member whose value is to be returned
 * @return a pointer to the json member value, or NULL it the named
 *         member does not exist or is not a suitable type (e.g array)
 */
const char* json_get_member_string(json* json, const char* name );


/**
 * Return the type of a named json member.
 * @param json the json object to search
 * @param name the name of the member whose type is to be returned
 * @return the type of the named member, or json_null if it does
 *         not exists. Note that json_null is also a valid type.
 */
json_type json_get_member_type(json* json, const char* name );




/****************************************
 * Session declarations
 ***************************************/


/**
 * Creates a unique cookie ID that can be used as a session
 * reference.
 *
 * @param r a pointer to the request structure for the
 *          currently active request.
 * @return a pointer to the name of a new, unique, session
 */
const char* create_urs_cookie_id(request_rec *r);



/**
 * Writes session data table to a session file.
 * @param r a pointer to the request structure for the
 *          currently active request.
 * @param auth_cookie the cookie value. This is used to identify
 *          the session file.
 * @param session_data the current session data that should be stored.
 * @return APR_SUCCESS on success.
 */
apr_status_t write_urs_session(
        request_rec *r,
        const char* auth_cookie,
        apr_table_t* session_data );



/**
 * Reads a session file into a session data table.
 * @param r a pointer to the request structure for the
 *          currently active request.
 * @param auth_cookie the cookie value. This is used to identify
 *          the session file.
 * @param session_data a table into which all the session data
 *          will be placed.
 * @return APR_SUCCESS on success.
 */
apr_status_t read_urs_session(
        request_rec *r,
        const char* auth_cookie,
        apr_table_t* session_data );


/**
 * Deletes a session file.
 * @param r a pointer to the request structure for the
 *          currently active request.
 * @param auth_cookie the cookie value. This is used to identify
 *          the session file.
 *
 * @return APR_SUCCESS.
 */
apr_status_t destroy_urs_session(request_rec *r, const char* auth_cookie);



/****************************************
 * HTTP declarations
 ***************************************/

/**
 * Extracts the value of a query parameter from the client request.
 *
 * @param r a pointer to the request structure for the
 *          currently active request.
 * @param parameter the name of the query parameter to extract.
 * @return a pointer to the query parameter value, or NULL
 *         if it did not exist or was empty.
 */
char* get_query_param(
        request_rec* r,
        const char* parameter );


/**
 * Extracts the value of a named cookie.
 *
 *
 * @param r a pointer to the request structure for the
 *          currently active request.
 * @param cookie_name the name of the cookie extract.
 * @return a pointer to the cookie value, or NULL
 *         if it did not exist or was empty.
 */
char* get_cookie(
        request_rec* r,
        const char* cookie_name );


/**
 * Encode a URL string.
 * This function maps reserved characters in a string to their % equivalent.
 *
 * @param r the client request
 * @param uri the URI to encode.
 * @return a pointer to the encoded string. This can be the same
 *         string if no encoding is necessary.
 */
const char* url_encode(
        request_rec *r,
        const char* uri );


/**
 * Decode a URL string.
 * This function maps % encoded characters back to their string equivalent
 *
 * @param r the client request
 * @param uri the URI to decode.
 * @return a pointer to the decoded string. This can be the same
 *         string if no decoding is necessary.
 */
const char* url_decode(
        request_rec *r,
        const char* uri );


/**
 * Performs an http post type request and reads the response.
 *
 * @param r the current request (used for configuration/memory pool allocations)
 * @param server URI containing the address of the server to send the request to
 * @param path the path to post
 * @param headers a table of headers to send. Also used to return response
 *        headers.
 * @param body the body of the request to send. Also used to return the
 *        response body.
 * @return the response status
 */
int http_post(
        request_rec *r,
        apr_uri_t* server,
        const char* path,
        apr_table_t* headers,
        char** body);


/**
 * Performs an http get type request and reads the response.
 *
 * @param r the current request (used for configuration/memory pool allocations)
 * @param server URI containing the address of the server to send the request to
 * @param path the path to post
 * @param headers a table of headers to send. Also used to return response
 *        headers.
 * @param body returns the response body
 * @return the response status
 */
int http_get(
        request_rec *r,
        apr_uri_t* server,
        const char* path,
        apr_table_t* headers,
        char** body);



/****************************************
 * SSL declarations
 ***************************************/

/**
 * External representation of a connection.
 */
typedef struct ssl_connection ssl_connection;


/**
 * Establishes an SSL connection to a remote server.
 * @param r a pointer to the apache request_rec structure.
 * @param host the name of the host to connect to
 * @param port the port number to connect to
 * return a pointer to an ssl_connection structure, or
 *        NULL on error
 */
ssl_connection *ssl_connect(request_rec* r, const char* host, int port );



/**
 * Close and tidy up an SSL connection.
 * @param r a pointer to the current request (not currently needed)
 * @param c a pointer to the ssl_connection structure to be cleaned
 */
void ssl_disconnect(request_rec* r, ssl_connection *c );


/**
 * Reads a chunk of data from the SSL connection.
 * @param r a pointer to the current request
 * @param c a pointer to the ssl_connection structure to be
 *          read from.
 * @param buffer the buffer into which the data is to be placed
 * @param bufsize the size of the buffer.
 * @return the number of bytes read, or negative error number
 */
int ssl_read(request_rec* r, ssl_connection *c, char* buffer, int bufsize);


/**
 * Writes a chunk of data to the SSL connection.
 * @param r a pointer to the current request
 * @param c a pointer to the ssl_connection structure to be
 *          read from.
 * @param buffer the data to be written
 * @param bufsize the size of the data in the buffer.
 * @return the number of bytes written, or negative error number
 */
int ssl_write(request_rec* r, ssl_connection *c, char* buffer, int bufsize );

