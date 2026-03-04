#define _USE_MATH_DEFINES
//OpenCV
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
//SFML
#include <SFML/Graphics.hpp>
//CPP
#include <iostream>
#include <vector>
#include <numeric>
#include <cmath>
#include <math.h>
//Personal
#include "DFT.h"


using namespace cv;
using namespace std;


Mat src_gray; // creates empty mat
int thresh = 100; //theshhold 1
int thresh2 = 100; //threshold 2
RNG rng(12345); //rng creates random colors for contours
vector<Point2f> untransformed_points; //untransformed points
vector<vector<double>> transformed_Xpoints, transformed_Ypoints;
vector<vector<double>> wave;

void thresh_callback(int, void*); //threshold function prototype
vector<Point2f> flatten(const vector<vector<Point> >& contours); //flatten function prototype
void Fdraw(vector<vector<double>> fourierX, vector<vector<double>> fourierY);
vector<double> epicycles(sf::RenderWindow& Fwindow, const vector<vector<double>>& transformed_points, double x, double y, double rotation, long double& time);
char multicontour;
int reduce_to = 1;
int main(int argc, char** argv)
{
    //IMAGE PROCESSING

    
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

    string source_window = "Source";
    namedWindow(source_window); //creates a window
    imshow(source_window, src); //places color source in the newly made window

    const int max_thresh = 255; //maximum threshold
    createTrackbar("Canny thresh1:", source_window, nullptr, max_thresh, thresh_callback);
    createTrackbar("Canny thresh2:", source_window, nullptr, max_thresh * 2, thresh_callback);
   
    cout << "Enter positive integer for downsampling rate (higher is faster, and around three is typical): ";
    cin >> reduce_to;
    cout << endl;
    cout << "Do you want more than one contour (Y/N)? (if no, largest contour will chosen) ";
    cin >> multicontour;
    cout << endl;

    thresh_callback(0, 0); // runs thresh_callback at least once
    
    cout << "Control contours until they are minimized into as few as possible then hit the Y key with window selected to start drawing";

    while (true)
    {
        int key = cv::waitKey(30);

        if (key == 'Y' || key == 'y')
            break;
    }
        //transform
    transformed_Xpoints = fourier(untransformed_points, 'x'); //transforms x coordinate of all  points
    transformed_Ypoints = fourier(untransformed_points, 'y'); //transforms y coordinate of all points
   
    waitKey(); //waits for input so program doesnt exit immediately
   Fdraw(transformed_Xpoints, transformed_Ypoints);

    return 0;
}



void thresh_callback(int, void*)
{
    thresh = getTrackbarPos("Canny thresh1:", "Source");
    thresh2 = getTrackbarPos("Canny thresh2:", "Source");
    Mat canny_output; // creates edge detected mat
    Canny(src_gray, canny_output, thresh, thresh2); //(input, output, thresh1, thresh2)

    vector<vector<Point> > contours; //an set of contours containing sets of tuples i.e. (x,y)
    vector<Vec4i> hierarchy; //the hierarchy uses a 4-tuple (next, previous, first child, parent), this uses the indicies of the contours (-1 is none)
    findContours(canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE); //(input, output, hierarchy, retrieval mode, approximation method)

    if (multicontour == 'Y') {
        untransformed_points = flatten(contours);
    }
    else {
    int largestindex = 0;
    double largestarc = 0;
    for (int i = 0; i < contours.size(); i++) {
        double arclength = arcLength(contours[i], 0);
        if (arclength > largestarc) {
            largestarc = arclength;
            largestindex = i;
        }
    }
    untransformed_points.clear();
    for (const Point& p : contours[largestindex]) {
        untransformed_points.emplace_back(p.x, p.y);
    }
}

    // downsample to 1/nth
    vector<Point2f> reduced; 
    reduced.reserve(untransformed_points.size() / reduce_to);

    for (size_t i = 0; i < untransformed_points.size(); i += reduce_to)
    {
        reduced.push_back(untransformed_points[i]);
    }
    untransformed_points = reduced;
   

    //auto scaling
    float maxAbs = 0.0f;
for (const auto& p : untransformed_points) {
        maxAbs = max(maxAbs, abs(p.x));
        maxAbs = max(maxAbs, abs(p.y));
    }

    if (maxAbs == 0)
        maxAbs = 1;

    float desiredSize = 400.0f; // half your window
    float scale = desiredSize / maxAbs;

    for (auto& p : untransformed_points) {
        p.x *= scale;
        p.y *= scale;
    }

    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3); //creates black canvas with size of canny_output and in 8bit unsigned 3 channel colorspace
    for (size_t i = 0; i < contours.size(); i++) // loops over all contours (size_t is unsigned integer)
    {
        Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256)); // a scalar in cv is a tuple, representing color (B,G,R)
        drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0); //(destination, input contours, casts size_t to int and shows current index, color of indexed contour, line thickness, 8-connected line drawing, hierarchy data, maximum hierarchy level to draw)
    }

    imshow("Contours", drawing); //creates window and displays contours
    
    
}


//flattens contour vectors into one large vector of points
vector<Point2f> flatten(const vector<vector<Point> >& contours) {
    vector<Point2f> flattened;

    //sets vector size beforehand
    size_t total_size = 0;
    for (const vector<Point>& vec : contours) {
        total_size += vec.size();
    }
    flattened.reserve(total_size);

    //runs through every set of points and adds it to the end of the "flattened" vector
    for (const vector<Point>& vec : contours) {
        flattened.insert(flattened.end(), vec.begin(), vec.end());
    }
    return flattened;
}
vector<double> epicycles(sf::RenderWindow& Fwindow, const vector<vector<double>>& transformed_points, double x, double y, double rotation, long double &time) {
   
    for (int j = 0; j < transformed_points.size(); j++) {
        double prevx = x;
        double prevy = y;
        double freq = transformed_points[j][2];
        double radius = transformed_points[j][3];
        double phase = transformed_points[j][4];
        x += radius * cos(freq * time + phase+ rotation);
        y += radius * sin(freq * time + phase + rotation);

        //circles
        sf::CircleShape circle(radius);
        circle.setOrigin(radius, radius);
        circle.setFillColor(sf::Color::Transparent);
        circle.setOutlineColor(sf::Color::White); \
            circle.setPosition(prevx, prevy);
        circle.setOutlineThickness(1.f);
        Fwindow.draw(circle);

        //runner points
        sf::CircleShape runnerpoint(2.f);
        runnerpoint.setOrigin(2, 2);
        runnerpoint.setFillColor(sf::Color::White);
        runnerpoint.setPosition(x, y);
        Fwindow.draw(runnerpoint);

        //lines from center of circle to runner points
        sf::Vertex innerline[] =
        {
            sf::Vertex(sf::Vector2f(prevx, prevy)),
            sf::Vertex(sf::Vector2f(x, y))
        };
        Fwindow.draw(innerline, 2, sf::Lines);
    }
    vector<double> epicycleout = { x,y };
    return epicycleout;
}


void Fdraw(vector<vector<double>> fourierX, vector<vector<double>> fourierY) {
    string fourier_drawing = "Fourier Drawing";
    sf::RenderWindow Fwindow(sf::VideoMode(1200, 1200), fourier_drawing);
    Fwindow.setFramerateLimit(40);
    sf::View view;
    view.setCenter(-350.f, 50.f);
    view.setSize(1200.f, 1200.f);
    Fwindow.setView(view);
    long double time = 0.0;

    while (Fwindow.isOpen())
    {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (Fwindow.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                Fwindow.close();
        }
        // clear the window with black color
        Fwindow.clear(sf::Color::Black);

        // draw everything
        vector<double> vx = epicycles(Fwindow, transformed_Xpoints, -650, 350, 0, time);
        vector<double> vy = epicycles(Fwindow, transformed_Ypoints, -75, -250, M_PI/2, time);
        vector<double> v = { vx[0],vy[1] };

        wave.insert(wave.begin(), v);

        sf::Vertex xtopoint[] =
        {
            sf::Vertex(sf::Vector2f(vx[0], vx[1])),
            sf::Vertex(sf::Vector2f(v[0], v[1]))
        };
        Fwindow.draw(xtopoint, 2, sf::Lines);

        sf::Vertex ytopoint[] =
        {
            sf::Vertex(sf::Vector2f(vy[0], vy[1])),
            sf::Vertex(sf::Vector2f(v[0], v[1]))
        };
        Fwindow.draw(ytopoint, 2, sf::Lines);


        for (int i = 0; i < wave.size(); i++) {
            sf::CircleShape point(1);
            point.setFillColor(sf::Color::White);
            point.setPosition(wave[i][0], wave[i][1]);
            Fwindow.draw(point);
        }

        //from tip of final runner point to tip of wave


        if (wave.size() > 5000) {
            wave.pop_back();
        }

        long double dt = (2 * M_PI) / fourierY.size();
        time += dt;

        if (time > 2 * M_PI) {
            time = 0;
        }
        // end the current frame
        Fwindow.display();
    }
}
