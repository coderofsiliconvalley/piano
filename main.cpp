#include <iostream>
#include <opencv2/opencv.hpp>


int main(int argc, char ** argv) 
{
    cv::namedWindow("Hello world", cv::WINDOW_AUTOSIZE);
    cv::waitKey(0);
    return 0;
}
