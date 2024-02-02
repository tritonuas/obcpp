#ifndef INCLUDE_UTILITIES_HTTP_HPP_
#define INCLUDE_UTILITIES_HTTP_HPP_

#include <unordered_map>

enum HTTPStatus {
    OK = 200,

    BAD_REQUEST = 400,
    NOT_FOUND = 404,

    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
};

#define _SET_HTTP_MAPPING(msg) {HTTPStatus::msg, #msg}
const std::unordered_map<HTTPStatus, const char*> HTTP_STATUS_TO_STRING = {
    _SET_HTTP_MAPPING(OK),
    _SET_HTTP_MAPPING(BAD_REQUEST), _SET_HTTP_MAPPING(NOT_FOUND),
    _SET_HTTP_MAPPING(INTERNAL_SERVER_ERROR), _SET_HTTP_MAPPING(NOT_IMPLEMENTED)
};

namespace mime {
    const char json[] = "application/json";
    const char plaintext[] = "text/plain";
}

#endif  // INCLUDE_UTILITIES_HTTP_HPP_
