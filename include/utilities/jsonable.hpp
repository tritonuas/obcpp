#ifndef INCLUDE_UTILITIES_JSONABLE_HPP_
#define INCLUDE_UTILITIES_JSONABLE_HPP_

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class jsonable {

    json to_json();

};

#endif