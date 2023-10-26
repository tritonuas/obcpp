#include <iostream>

#include "core/states.hpp"

#include <opencv2/opencv.hpp>
#include <torch/torch.h>
#include <iostream>

int main() {
    std::cout << "hellasdfasdfTarget torch_cpu not found.o" << std::endl;

    PreparationState state;

    state.tick();

  torch::Tensor tensor = torch::rand({2, 3});
  std::cout << tensor << std::endl;

      //create a gui window:
    namedWindow("Output",1);
    
    //initialize a 120X350 matrix of black pixels:
    Mat output = Mat::zeros( 120, 350, CV_8UC3 );
    
    //write text on the matrix:
    putText(output,
            "Hello World :)",
            cvPoint(15,70),
            FONT_HERSHEY_PLAIN,
            3,
            cvScalar(0,255,0),
            4);
    
    //display the image:
    imshow("Output", output);
    
    //wait for the user to press any key:
    waitKey(0);
    
    return 0;
}