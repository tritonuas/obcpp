#ifndef INCLUDE_UTILITIES_OBC_CONFIG_MACROS_HPP_
#define INCLUDE_UTILITIES_OBC_CONFIG_MACROS_HPP_

#include <string>

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

    You shouldn't ever need to look in here. Ever. It should just work. If it is not working
    then you are doing something wrong... probably.
*/

// dont worry about it...
// just use these macros to do the parsing for the config
// it saves like 5 characters, and forces you to use .at instead of [] to index into the json
// because .at gives more descriptive error messages
// ALSO: does some shenanegans with tracking the line number of the
// last call to PARSE and handling errors

// return statement in this macro in catch case is there to supress compiler warning b/c
// it doesn't know that FATAL log will kill it
#define HANDLE_JSON_EXCEPTION(expr) \
    try { \
        return expr; \
    } catch (nlohmann::json::exception ex) { \
        LOG_F(FATAL, "%s", ex.what()); \
        return nlohmann::json(); \
    }

// basically setting up an overloaded macro where you can pass in a variable number of args
// with the same macro name. Same setup as in the gcs macros
#define GET_MACRO_5(_1, _2, _3, _4, _5, NAME, ...) NAME
#define PARSE_CONFIG_2(p1, p2) configs.at(#p1).at(#p2)
#define PARSE_CONFIG_3(p1, p2, p3) configs.at(#p1).at(#p2).at(#p3)
#define PARSE_CONFIG_4(p1, p2, p3, p4) configs.at(#p1).at(#p2).at(#p3).at(#p4)
#define PARSE_CONFIG_5(p1, p2, p3, p4, p5) configs.at(#p1).at(#p2).at(#p3).at(#p4).at(#p5)

// wrapper around accessing the configs json and handling exceptions with useful error messages
// yes this is an inline lambda function that is instantly being called
// yes there is probably a cleaner way to do this
// and yes this works
#define PARSE(...) \
    [configs]() { \
        HANDLE_JSON_EXCEPTION( \
            GET_MACRO_5(__VA_ARGS__, PARSE_CONFIG_5, PARSE_CONFIG_4, \
                PARSE_CONFIG_3, PARSE_CONFIG_2) (__VA_ARGS__)) \
    }()

// look in the code below for an example on how to use this function
// basically handles parsing a string and converting it to a valid enum value
// panics if the string value is invalid
template <typename T>
constexpr T PARSE_VARIANT(CONFIG_VARIANT_MAPPING_T(T) mapping, std::string input) {
    for (const auto& [str_val, enum_val] : mapping) {
        if (input == str_val) {
            return enum_val;
        }
    }
    LOG_F(FATAL, "Unknown config option %s", input.c_str());
    std::exit(1);  // not needed except so the compiler knows this cannot reach end of function
}

// and then at the very end we prevent having to duplicate typing
// the struct config option and the json accessing syntax...

#define SET_CONFIG_OPT_5(p1, p2, p3, p4, p5) \
    this->p1.p2.p3.p4.p5 = PARSE(p1, p2, p3, p4, p5)
#define SET_CONFIG_OPT_4(p1, p2, p3, p4) \
    this->p1.p2.p3.p4 = PARSE(p1, p2, p3, p4)
#define SET_CONFIG_OPT_3(p1, p2, p3) \
    this->p1.p2.p3 = PARSE(p1, p2, p3)
#define SET_CONFIG_OPT_2(p1, p2) \
    this->p1.p2 = PARSE(p1, p2)
#define SET_CONFIG_OPT(...) \
    GET_MACRO_5(__VA_ARGS__, SET_CONFIG_OPT_5, SET_CONFIG_OPT_4, \
        SET_CONFIG_OPT_3, SET_CONFIG_OPT_2) (__VA_ARGS__)

// then we finally repeat everything for the variants...
#define SET_CONFIG_OPT_5_VARIANT(mapping, p1, p2, p3, p4, p5) \
    this->p1.p2.p3.p4.p5 = PARSE_VARIANT(mapping, PARSE(p1, p2, p3, p4, p5))
#define SET_CONFIG_OPT_4_VARIANT(mapping, p1, p2, p3, p4) \
    this->p1.p2.p3.p4 = PARSE_VARIANT(mapping, PARSE(p1, p2, p3, p4))
#define SET_CONFIG_OPT_3_VARIANT(mapping, p1, p2, p3) \
    this->p1.p2.p3 = PARSE_VARIANT(mapping, PARSE(p1, p2, p3))
#define SET_CONFIG_OPT_2_VARIANT(mapping, p1, p2) \
    this->p1.p2 = PARSE_VARIANT(mapping, PARSE(p1, p2))
#define SET_CONFIG_OPT_VARIANT(mapping, ...) \
    GET_MACRO_5(__VA_ARGS__, SET_CONFIG_OPT_5_VARIANT, SET_CONFIG_OPT_4_VARIANT, \
        SET_CONFIG_OPT_3_VARIANT, SET_CONFIG_OPT_2_VARIANT) (mapping::MAPPINGS, __VA_ARGS__)

#endif  // INCLUDE_UTILITIES_OBC_CONFIG_MACROS_HPP_
