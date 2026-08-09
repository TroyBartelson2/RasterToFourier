// FourierDrawTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>

using namespace cv;
using namespace std;

Mat src_gray; // creates empty mat
int thresh = 100; //create theshhold
RNG rng(12345); //rng creates random colors for contours

void thresh_callback(int, void*); //prototypes the threshold function

int main(int argc, char** argv)
{
    CommandLineParser parser(argc, argv, "{@input | input.jpg | input image}"); //aligns parser to look at input command line argument, defualts at input.jpg
    Mat src = imread(parser.get<String>("@input")); //gets value of @input and reads it into source mat
    if (src.empty()) //input validation
    {
        cout << "Could not open or find the image!\n" << endl;
        cout << "Usage: " << argv[0] << " <Input image>" << endl; //tells the format i.e. FourierDrawTest.exe input.jpg
        return -1;
    }

    cvtColor(src, src_gray, COLOR_BGR2GRAY); //src_gray's matrix now points to src, while the header flags as a greyscale version of source
    blur(src_gray, src_gray, Size(3, 3)); //blurs src_gray in a 3x3 convolution

    const char* source_window = "Source";
    namedWindow(source_window); //creates a window (empty)
    imshow(source_window, src); //places color source in the newly made window

    const int max_thresh = 255; // Creates a maximum threshold
    createTrackbar("Canny thresh:", source_window, &thresh, max_thresh, thresh_callback); //creates a slider with createTrackbar entries as follows (Name of slider, which window to put it in i.e. window name, initial value, maximum value, and callback when slider changes)
    thresh_callback(0, 0); // runs thresh_callback at least once

    waitKey(); //waits for input so program doesnt exit immediately
    return 0;
}

void thresh_callback(int, void*)
{
    Mat canny_output; // creates edge detected mat
    Canny(src_gray, canny_output, thresh, thresh * 2); //(input, output, thresh1, thresh2)

    vector<vector<Point> > contours; //an set of contours containing sets of tuples i.e. (x,y)
    vector<Vec4i> hierarchy; //the hierarchy uses a 4-tuple (next, previous, first child, parent), this uses the indicies of the contours (-1 is none)
    findContours(canny_output, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE); //(input, output, hierarchy, retrieval mode, approximation method)

    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3); //creates black canvas with size of canny_output and in 8bit unsigned 3 channel colorspace
    for (size_t i = 0; i < contours.size(); i++) // loops over all contours (size_t is unsigned integer)
    {
        Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256)); // a scalar in cv is a tuple, representing color (B,G,R)
        drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0); //(destination, input contours, casts size_t to int and shows current index, color of indexed contour, line thickness, 8-connected line drawing, hierarchy data, maximum hierarchy level to draw)
    }

    imshow("Contours", drawing); //creates window and displays contours
}
