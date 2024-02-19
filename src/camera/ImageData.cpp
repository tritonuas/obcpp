#include "camera/interface.hpp"
#include <opencv2/opencv.hpp>
#include <opencv2/core/mat.hpp>

ImageData::ImageData()
{
}

ImageData::ImageData(std::string NAME, std::string PATH, Mat DATA)
{
    this->NAME = NAME;
    this->PATH = PATH;
    this->DATA = DATA;
}

std::string ImageData::getName()
{
    return this->NAME;
}

std::string ImageData::getPath()
{
    return this->PATH;
}

cv::Mat ImageData::getData()
{
    return this->DATA;
}