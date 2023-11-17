#include <iostream>
#include "core/states.hpp"
#include "cv/hdr.hpp"
#include "Eigen"
#include <torch/torch.h>

#include <iostream>
#include <httplib.h>

#include "core/states.hpp"
#include "pathing/plotting.hpp"

int main()
{
    httplib::Server svr;
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });
    svr.listen("0.0.0.0", 8080);
}
