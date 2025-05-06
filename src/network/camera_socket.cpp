#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>

// Very large just in case strerror(errno) actually ends up being incredibly large
#define AD_ERR_LEN 999

using namespace boost::asio;
using ip::tcp;

tcp::socket socket;

// CameraSocket::CameraSocket() {

// }
