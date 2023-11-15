#include <iostream>
#include "core/states.hpp"
#include "Eigen"
// #include <opencv2/opencv.hpp>
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
  
  return 0;
}