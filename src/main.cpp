#include <iostream>

#include "Eigen"
#include "core/states.hpp"
#include <torch/torch.h>

#include <iostream>
#include <httplib.h>

<<<<<<< HEAD
=======
#include "core/states.hpp"
#include "pathing/plotting.hpp"
>>>>>>> main

int main()
{
    httplib::Server svr;
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello World!", "text/plain");
    });
    svr.listen("0.0.0.0", 8080);
  
  return 0;
}