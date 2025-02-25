#include <iostream>

struct ImageData_t
{
    int width;
    int height;
    size_t imageSizeBytes;
    std::vector<std::uint8_t> data;
};

// What does OBC needs from camera
enum class RequestType_t
{
    SENDIMAGE
};

// What is the status of the request (debugging + error handling)
enum class ResponseType_t
{
    SUCC,
    ERROR
};

// OBC requesting camera for something 
struct CameraRequest_t
{
    // device id that requests (for debugging)
    int pid;

    // type of request, we should figure out edge cases
    RequestType_t requestType;
};

// Response back from camera to OBC
struct CameraResponse_t
{
    int pid; 
    ResponseType_t responseType;
    ImageData_t data;

};

