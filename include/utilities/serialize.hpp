#ifndef INCLUDE_UTILITIES_SERIALIZE_HPP_
#define INCLUDE_UTILITIES_SERIALIZE_HPP_

#include <nlohmann/json.hpp>
#include <google/protobuf/util/json_util.h>

#include "protos/obc.pb.h"

/*
 * Annoyingly you cannot use google::protobuf::util::MessageToJson on 
 * a RepeatedPtrField of messages, so we have this helper function
 * instead which manually serializes all of the items one by one
 * 
 * Note: template functions generally should be implemented in header
 * files, so that is why there is no separate source file.
 */
template<typename Iterator>
std::string messagesToJson(Iterator begin, Iterator end) {
    Iterator it = begin;

    std::string json = "[";
    while (it != end) {
        google::protobuf::Message& message = *it;
        std::string message_json;

        google::protobuf::util::MessageToJsonString(message, &message_json);
        json += message_json + ',';
        ++it;
    }

    json.pop_back(); // remove final comma
    json += ']';

    return json;
}

#endif  // INCLUDE_UTILITIES_SERIALIZE_HPP_