#ifndef INCLUDE_UTILITIES_HTTP_HPP_
#define INCLUDE_UTILITIES_HTTP_HPP_

#include <unordered_map>
#include <string>

enum HTTPStatus {
    OK = 200,

    BAD_REQUEST = 400,
    NOT_FOUND = 404,

    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
};

#define _SET_HTTP_MAPPING(msg) case HTTPStatus::msg: return #msg
constexpr const char* HTTP_STATUS_TO_STRING(HTTPStatus status) {
    switch (status) {
        // 2xx
        _SET_HTTP_MAPPING(OK);

        // 4xx
        _SET_HTTP_MAPPING(BAD_REQUEST);
        _SET_HTTP_MAPPING(NOT_FOUND);

        // 5xx
        _SET_HTTP_MAPPING(INTERNAL_SERVER_ERROR);
        _SET_HTTP_MAPPING(NOT_IMPLEMENTED);

        default: return std::to_string(status).c_str();  // just return the straight number code
    }
}

namespace mime {
    const char json[] = "application/json";
    const char plaintext[] = "text/plain";
}

#endif  // INCLUDE_UTILITIES_HTTP_HPP_
