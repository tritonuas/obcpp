#ifndef INCLUDE_UTILITIES_MACROS_HPP_
#define INCLUDE_UTILITIES_MACROS_HPP_

#include <memory>

/*
   (  )   /\   _                 (     
    \ |  (  \ ( \.(               )                      _____
  \  \ \  `  `   ) \             (  ___                 / _   \
 (_`    \+   . x  ( .\            \/   \____-----------/ (o)   \_
- .-               \+  ;          (  O                           \____
                          )        \_____________  `              \  /
(__                +- .( -'.- <. - _  VVVVVVV VV V\                 \/
(_____            ._._: <_ - <- _  (--  _AAAAAAA__A_/                  |
  .    /./.+-  . .- /  +--  - .     \______________//_              \_______
  (__ ' /x  / x _/ (                                  \___'          \     /
 , x / ( '  . / .  /                                      |           \   /
    /  /  _/ /    +                                      /              \/
   '  (__/                                             /                  \
                        HERE BE DRAGONS

    Don't edit these unless you know what you are doing!

    Most likely you will want to adding or editing a GCS Handler function,
    which is made extremely easy by use of these macros. Just mimic the code
    structure already in gcs_routes.hpp
*/

// For use in overloading macros
#define GET_MACRO_5(_1, _2, _3, _4, _5, NAME, ...) NAME
#define GET_MACRO_4(_1, _2, _3, _4, NAME, ...) NAME

// For use in concatenating macro arguments
// https://stackoverflow.com/questions/74723697/how-to-concatenate-all-of-the-arguments-of-a-variadic-macro-into-a-quoted-string NOLINT
#define CAT_(a,b,c,d,e,f,g,i,j,k,l,m,n,o,p,...) a##b##c##d##e##f##g##i##j##k##l##m##n##o##p // NOLINT
#define CAT(...) CAT_(__VA_ARGS__,,,,,,,,,,,,,,,,,) // NOLINT
#define STR(...) #__VA_ARGS__
#define STRe(...) STR(__VA_ARGS__)

#define BIND_PARAMS(...) \
    std::bind_front(&GCS_HANDLE(__VA_ARGS__), this->state)
#define BIND_HANDLER_4(Method, uri1, uri2, uri3) \
    server.Method(STRe(/uri1/uri2/uri3), BIND_PARAMS(Method, uri1, uri2, uri3))
#define BIND_HANDLER_3(Method, uri1, uri2) \
    server.Method(STRe(/uri1/uri2), BIND_PARAMS(Method, uri1, uri2))
#define BIND_HANDLER_2(Method, uri1) \
    server.Method(STRe(/uri1), BIND_PARAMS(Method, uri1))

// Call with the same parameters as DEF_GCS_HANDLE to bind the function
// to the http server endpoint.
#define BIND_HANDLER(...) \
    GET_MACRO_4(__VA_ARGS__, BIND_HANDLER_4, BIND_HANDLER_3, BIND_HANDLER_2)(__VA_ARGS__)

#define GCS_HANDLER_PARAMS \
    std::shared_ptr<MissionState> state, \
    const httplib::Request& request, \
    httplib::Response& response

#define GCS_HANDLE_4(Method, uri1, uri2, uri3) \
    Method ## _ ## uri1 ## _ ## uri2 ## _ ## uri3

#define GCS_HANDLE_3(Method, uri1, uri2) \
    Method ## _ ## uri1 ## _ ## uri2

#define GCS_HANDLE_2(Method, uri1) \
    Method ## _ ## uri1

// Overload the different macros
#define DEF_GCS_HANDLE_4(Method, uri1, uri2, uri3) \
    void GCS_HANDLE_4(Method, uri1, uri2, uri3) (GCS_HANDLER_PARAMS)

#define DEF_GCS_HANDLE_3(Method, uri1, uri2) \
    void GCS_HANDLE_3(Method, uri1, uri2) (GCS_HANDLER_PARAMS)

#define DEF_GCS_HANDLE_2(Method, uri1) \
    void GCS_HANDLE_2(Method, uri1) (GCS_HANDLER_PARAMS)

/*
 * DEF_GCS_HANDLE(Method, uri1, uri2?, uri3?)
 *
 * Generate a function of the format
 * {Method}_{uri1}_{uri2}_{uri3}(GCS_HANDLER_PARAMS)
 * where uri2 and uri3 are optional.
 * 
 * Note: each argument should be capitalized how they are in the macro
 *       arguments, so the method should be like "Get", "Post", ... while
 *       the uris should be all lowercase
 */
#define DEF_GCS_HANDLE(...) \
    GET_MACRO_4(__VA_ARGS__, DEF_GCS_HANDLE_4, DEF_GCS_HANDLE_3, DEF_GCS_HANDLE_2) \
        (__VA_ARGS__)

/*
 * Get the function name for a GCS handler
 * Follows the same rules as DEF_GCS_HANDLE
 */
#define GCS_HANDLE(...) \
    GET_MACRO_4(__VA_ARGS__, GCS_HANDLE_4, GCS_HANDLE_3, GCS_HANDLE_2)(__VA_ARGS__)

// Should be called at the beginning of every handler function so we can
// log out all of the relevant information
//
// LOG_REQUEST(method, route)
//     where both arguments are c-strings
#define LOG_REQUEST(method, route) \
    LOG_SCOPE_F(INFO, "%s %s", method, route); \
    LOG_F(INFO, "User-Agent: %s", request.get_header_value("User-Agent").c_str())

// One of the LOG_RESPONSE logging functions should be used to both log and
// set the HTTP response
#define LOG_RESPONSE_5(LOG_LEVEL, msg, response_code, body, mime) \
    if (msg != body) LOG_F(LOG_LEVEL, "%s", msg); \
    LOG_F(LOG_LEVEL, "HTTP %d: %s", response_code, HTTP_STATUS_TO_STRING.at(response_code)); \
    LOG_F(LOG_LEVEL, "%s", body); \
    response.set_content(body, mime); \
    response.status = response_code

// Essentially a special case of the 5 param log function, where
// the message body is in plaintext and is also what you want to log
#define LOG_RESPONSE_3(LOG_LEVEL, msg, response_code) \
    LOG_RESPONSE_5(LOG_LEVEL, msg, response_code, msg, mime::plaintext)


// Logs important information about the HTTP request. There are two different
// ways to call: a 3 parameter version and a 5 parameter version.
//
// LOG_RESPONSE(LOG_LEVEL, msg, response_code)
//   This assumes that the mimetype is plaintext, and that the msg
//   you are logging out is what you also return back in the HTTP response
//
// LOG_RESPONSE(LOG_LEVEL, msg, response_code, body, mime)
//   This explicitly lets you send back a body of arbitrary mimetype.
#define LOG_RESPONSE(...) \
    GET_MACRO_5(__VA_ARGS__, LOG_RESPONSE_5, _4, LOG_RESPONSE_3)(__VA_ARGS__)

#endif  // INCLUDE_UTILITIES_MACROS_HPP_
