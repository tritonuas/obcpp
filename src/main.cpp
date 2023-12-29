#include <iostream>
#include "core/states.hpp"
#include "cv/hdr.hpp"
// #include <torch/torch.h>

#include <iostream>
#include <httplib.h>
#include "utilities/constants.hpp"

#include "core/states.hpp"
#include "pathing/plotting.hpp"

int main()
{
    std::cout << "Starting HTTP server at port " << SERVER_PORT << std::endl;

    httplib::Server svr;
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });
    svr.listen("0.0.0.0", SERVER_PORT);
}
