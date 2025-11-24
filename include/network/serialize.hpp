#ifndef INCLUDE_NETWORK_SERIALIZE_HPP_
#define INCLUDE_NETWORK_SERIALIZE_HPP_

#include <boost/asio.hpp>

/*
    Includes some reference on how to serialize and deserialize structs
*/

namespace serialh {
    // TODO: make this so that it returns any struct
    template <typename T>
    void serialize(T* response, boost::asio::streambuf* buf) {
        std::ostream os(buf);
        boost::archive::binary_oarchive oa(os);
        oa << *response;
    }

    // TODO: make this so that it returns any struct
    template <typename T>
    void deserialize(T* response, boost::asio::streambuf* buf) {
        std::istream is(buf);
        boost::archive::binary_iarchive ia(is);
        ia >> *response;
    }
}

#endif
