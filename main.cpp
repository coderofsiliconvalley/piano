#include <iostream>
#include <opencv2/opencv.hpp>


int main(int argc, char ** argv) 
{
    auto video_capture = cv::VideoCapture {"video.mp4"};

    if (!video_capture.isOpened()) {
        std::cout << "Couldn't open video.mp4" << std::endl;
        return 0;
    }

    cv::namedWindow("main", cv::WINDOW_AUTOSIZE | 1);

    while (cv::waitKey(30) < 0) {
        auto frame = cv::Mat {};


        if (!video_capture.read(frame)) {
            break;
        }

        cv::Canny(frame, frame, 50, 200, 3);

        imshow("main", frame); 
    }

    cv::waitKey(0);
    return 0;
}
