#ifndef INCLUDE_NETWORK_CAMERA_DATA_HPP_
#define INCLUDE_NETWORK_CAMERA_DATA_HPP_

#include <iostream>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

struct ImageData_t
{
    int width;
    int height;
    size_t imageSizeBytes;
    std::vector<std::uint8_t> imgBuffer;

    template<class Archive>
	void serialize(Archive & ar, const unsigned int version){
		ar & width;
		ar & height;
		ar & imageSizeBytes;
		ar & imgBuffer;
	}
};

// What does OBC needs from camera
enum class RequestType_t
{
    SENDIMAGE
};

// TODO: unsure if these need to be in a namespace or not
template<class Archive>
void serialize(Archive & ar, RequestType_t & request, const unsigned int version) {
	ar & static_cast<int>(request); // have to type cast enum class
}

// What is the status of the request (debugging + error handling)
enum class ResponseType_t
{
    SUCC,
    ERROR
};

// TODO: unsure if these need to be in a namespace or not
template<class Archive>
void serialize(Archive & ar, ResponseType_t & response, const unsigned int version) {
	ar & static_cast<int>(response); // have to type cast enum class
}

// OBC requesting camera for something 
struct CameraRequest_t
{
    // device id that requests (for debugging)
    int pid;

    // type of request, we should figure out edge cases
    RequestType_t requestType;

    template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & pid;
		ar & requestType;
	}
};

// Response back from camera to OBC
struct CameraResponse_t
{
    int pid; 
    ResponseType_t responseType;
    ImageData_t imageData;

    template<class Archive>
	void serialize(Archive & ar, const unsigned int version) {
		ar & pid;
		ar & responseType;
		ar & imageData;
	}
};

#endif
