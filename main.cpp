#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/video/background_segm.hpp>
#include <vector>
#include <alure.h>
#include <al.h>
#include <alc.h>
#include <string.h>

#define countof(x) (sizeof x / sizeof *x)

ALuint buffers[7] = { 0 };
bool global_keystate[countof(buffers)] = { false };

cv::Mat get_piano(const cv::Mat& input)
{
    auto piano = input.clone();
    cv::inRange(piano, cv::Vec3i { 0, 0, 0 }, cv::Vec3i { 10, 255, 33 }, piano);
    cv::dilate(piano, piano, cv::getStructuringElement(cv::MORPH_ELLIPSE, {10, 10}));
    return piano;
}


cv::Mat get_hand(const cv::Mat& input) 
{
    //auto element = 
    auto hand = input.clone();
    cv::cvtColor(hand, hand, cv::COLOR_BGR2HSV);
    //cv::inRange(hand, cv::Vec3i {1, 104, 140}, cv::Vec3i { 12, 210, 250 }, hand);
    cv::inRange(hand, cv::Vec3i {1, 100, 140}, cv::Vec3i { 50, 210, 250 }, hand);
    cv::erode(hand, hand, cv::getStructuringElement(cv::MORPH_ELLIPSE, {3, 3}));
    cv::dilate(hand, hand, cv::getStructuringElement(cv::MORPH_ELLIPSE, {10, 10}));
    return hand;
}

cv::Mat get_shadow(const cv::Mat& input)
{
    auto shadow = input.clone();
    auto element = cv::getStructuringElement(cv::MORPH_ELLIPSE, {5, 5});
    cv::cvtColor(shadow, shadow, cv::COLOR_BGR2HSV);
    cv::inRange(shadow, cv::Vec3i {1, 0, 0}, cv::Vec3i { 255, 255, 100 }, shadow);
    cv::erode(shadow, shadow, cv::getStructuringElement(cv::MORPH_ELLIPSE, {3, 3}));
    cv::dilate(shadow, shadow, cv::getStructuringElement(cv::MORPH_ELLIPSE, {3, 3}));
    return shadow;
}


void foreground_mask(const cv::Mat& in, cv::Mat& out, const cv::Mat& background)
{
    auto old_in = in.clone();
    cv::absdiff(old_in, background, out);
    cv::threshold(out, out, 20, 255, CV_THRESH_BINARY);
    cv::erode(out, out, cv::getStructuringElement(cv::MORPH_ELLIPSE, {10, 10}));
    cv::dilate(out, out, cv::getStructuringElement(cv::MORPH_ELLIPSE, {20, 20}));
    cv::erode(out, out, cv::getStructuringElement(cv::MORPH_ELLIPSE, {5, 5}));
}


void equalize_histogram(const cv::Mat& in, cv::Mat& out)
{
    auto channels = std::vector<cv::Mat>{};
    cv::split(in, channels);
    for (auto& channel : channels) cv::equalizeHist(channel, channel);
    cv::merge(channels, out);
}


int count_edges(const cv::Mat& piano, int row)
{
    auto num_edges = 0;
    for (auto i = 1; i < piano.cols; ++i) {
        auto pixel = static_cast<int>(piano.at<uchar>(row, i));
        auto prev_pixel = static_cast<int>(piano.at<uchar>(row, i - 1));

        if (prev_pixel != pixel && prev_pixel * pixel == 0) {
            num_edges++;
        }
    }

    return num_edges;
}

void find_top_and_bottom_row(const cv::Mat& piano, int& top_row, int& bottom_row) 
{
    for (auto i = piano.rows - 6; i >= 0; i -= 5) {
        auto num_edges = count_edges(piano, i);
        auto prev_num_edges = count_edges(piano, i + 5);
        std::cout << prev_num_edges << " ";

        if (num_edges == 16 && prev_num_edges < 16) {
            bottom_row = i;
        }

        if (num_edges < 16 && prev_num_edges == 16) {
            top_row = i;
        }
    }
}

void make_sound(int i) 
{
    ALuint source = 0;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffers[i]);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 0.4f);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alurePlaySource(source, [](void *, ALuint source) -> void {
        alDeleteSources(1, &source);
    }, nullptr);
}

void analyze(const cv::Mat& piano, const cv::Mat& hand, const cv::Mat& shadow, int top, int bottom)
{
    bool key_skipped[7] = { false };
    bool key_state[7] = { false };

    for (auto row = bottom - 5; row > top + 5; --row) {
        auto current_key = -1;
        for (auto col = 1; col < piano.cols; ++col) {
            auto piano_pixel = static_cast<int>(piano.at<uchar>(row, col));
            auto piano_prev_pixel = static_cast<int>(piano.at<uchar>(row, col - 1));
            auto hand_pixel = static_cast<int>(hand.at<uchar>(row, col));
            //auto hand_prev_pixel = static_cast<int>(hand.at<uchar>(row, col - 1));
            auto shadow_pixel = static_cast<int>(shadow.at<uchar>(row, col));
            //auto shadow_prev_pixel = static_cast<int>(shadow.at<uchar>(row, col - 1));

            if (!piano_pixel && piano_prev_pixel) {
                current_key++;
            }
            
            if (shadow_pixel) {
                if (0 <= current_key && current_key < 7) key_skipped[current_key] = true;
            }

            if (hand_pixel && !key_skipped[current_key]) {
                if (0 <= current_key && current_key < 7) key_state[current_key] = true;
            }

        }
    }

    for (auto key : key_state) {
        std::cout << (key ? 1 : 0) << " ";
    }

    std::cout << std::endl;

    for (auto i = 0u; i < countof(key_state); ++i) {
        if (!global_keystate[i] && key_state[i]) {
            make_sound(i);
        }
        global_keystate[i] = key_state[i];  
    }
}

int main(int argc, char ** argv) 
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <video_file>" << std::endl;
        return 1;
    }

    auto video_capture = cv::VideoCapture { argv[1] };

    if (!video_capture.isOpened()) {
        std::cerr << "Couldn't open video.mp4" << std::endl;
        return 1;
    }

    if (alureInitDevice(NULL, NULL) == AL_FALSE) {
        std::cerr << "Couldn't create AL device" << std::endl;
        return 1;
    }

    buffers[0] = alureCreateBufferFromFile("samples/a.wav");
    buffers[1] = alureCreateBufferFromFile("samples/b.wav");
    buffers[2] = alureCreateBufferFromFile("samples/c.wav");
    buffers[3] = alureCreateBufferFromFile("samples/d.wav");
    buffers[4] = alureCreateBufferFromFile("samples/e.wav");
    buffers[5] = alureCreateBufferFromFile("samples/f.wav");
    buffers[6] = alureCreateBufferFromFile("samples/g.wav");



    auto original_frame = cv::Mat {};
    auto first_frame = cv::Mat {};
    auto piano_template = cv::Mat {};

    cv::namedWindow("main", cv::WINDOW_AUTOSIZE);

    auto piano_base = cv::Mat {};
    if (video_capture.read(first_frame)) {
        piano_template = get_piano(first_frame);
        cv::cvtColor(first_frame, first_frame, CV_BGR2GRAY);
    }

    auto top_row = 0;
    auto bottom_row = 0;
    find_top_and_bottom_row(piano_template, top_row, bottom_row);
    std::cout << "top: "<< top_row << std::endl;
    std::cout << "bottom: "<< bottom_row << std::endl;

    while (cv::waitKey(28) < 0) {
        if (!video_capture.read(original_frame)) {
            break;
        }

        auto shadow = get_shadow(original_frame);
        auto hand = get_hand(original_frame);
        auto piano = piano_template.clone();

        cv::cvtColor(original_frame, original_frame, CV_BGR2GRAY);
        auto fg_mask = cv::Mat {};
        foreground_mask(original_frame, fg_mask, first_frame); 

        //cv::bitwise_not(piano, piano);
        //cv::bitwise_and(shadow, piano, shadow);
        //cv::bitwise_not(piano, piano);
        //
        cv::bitwise_and(shadow, fg_mask, shadow);
        cv::bitwise_and(hand, fg_mask, hand);

        cv::bitwise_not(hand, hand);
        cv::bitwise_and(shadow, hand, shadow);
        cv::bitwise_not(hand, hand);

        auto channels = std::vector<cv::Mat> { hand, piano, shadow };

        auto result = cv::Mat {};
        cv::merge(channels, result);

        analyze(piano, hand, shadow, top_row, bottom_row);

        imshow("main", original_frame);

        alureUpdate();
    }

    cv::waitKey(0);

    alDeleteBuffers(sizeof buffers / sizeof *buffers, buffers);

    alureShutdownDevice();
    return 0;
}
