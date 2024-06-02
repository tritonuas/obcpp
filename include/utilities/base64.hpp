#ifndef INCLUDE_UTILITIES_BASE64_HPP_
#define INCLUDE_UTILITIES_BASE64_HPP_

#include <string>

std::string base64_encode(unsigned char const* , unsigned int len);
std::string base64_decode(std::string const& s);

#endif  // INCLUDE_UTILITIES_BASE64_HPP_
