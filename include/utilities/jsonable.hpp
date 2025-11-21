#ifndef INCLUDE_UTILITIES_JSONABLE_HPP_
#define INCLUDE_UTILITIES_JSONABLE_HPP_

#include "nlohmann/json.hpp"

using json = nlohmann::json;

class jsonable {
    /*
    Class to inherit from to make sure that objects can be serialized to json format.
    */
    json to_json();

};

#endif // INCLUDE_UTILITIES_JSONABLE_HPP_