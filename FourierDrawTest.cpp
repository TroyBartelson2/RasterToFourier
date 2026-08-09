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


Mat src_gray; 
int thresh = 100; 
int thresh2 = 100; 
RNG rng(12345); 
vector<Point2f> untransformed_points; 
vector<vector<double>> transformed_Xpoints, transformed_Ypoints;
vector<vector<double>> wave;

//Callback function for contour sliders
void thresh_callback(int, void*); 
//flattens nested vectors into simple (x,y) tuples
vector<Point2f> flatten(const vector<vector<Point> >& contours); 
//overall function to draw transformed points onscreen
void Fdraw(vector<vector<double>> fourierX, vector<vector<double>> fourierY); 
//draws connected circles that represent frequencies
vector<double> epicycles(sf::RenderWindow& Fwindow, const vector<vector<double>>& transformed_points, double x, double y, double rotation, long double& time);

char multicontour;
int reduce_to = 1;

//Takes in an input image named "input.jpg"
//Runs the "canny" image process over it which does edge detection and finds contours
//The thresh_callback function reruns this detection with different thresholds from the user
//Transforms points from a flattened set of contours through the Fourier Transform into the frequency domain
//displays the progression of the set of points through rotating circles of different frequencies
int main(int argc, char** argv)
{
    //IMAGE PROCESSING
    CommandLineParser parser(argc, argv, "{@input | input.jpg | input image}");
    Mat src = imread(parser.get<String>("@input")); 
    if (src.empty())
    {
        cout << "Could not open or find the image!\n" << endl; 
        cout << "Usage: " << argv[0] << " <Input image>" << endl; 
        return -1;
    }
    
    cvtColor(src, src_gray, COLOR_BGR2GRAY); 
    blur(src_gray, src_gray, Size(3, 3)); 

    string source_window = "Source";
    namedWindow(source_window); 
    imshow(source_window, src); 

    const int max_thresh = 255; 
    createTrackbar("Canny thresh1:", source_window, nullptr, max_thresh, thresh_callback);
    createTrackbar("Canny thresh2:", source_window, nullptr, max_thresh * 2, thresh_callback);
   
    cout << "Enter positive integer for downsampling rate, three is typical: (Higher is faster)";
    cin >> reduce_to;
    cout << endl;
    cout << "Do you want more than one contour (Y/N)? (if not, largest contour will chosen)";
    cin >> multicontour;
    cout << endl;

    thresh_callback(0, 0);
    
    cout << "Control contours until they are minimized into as few as possible, then hit the Y key with window selected to start drawing";

    while (true)
    {
        int key = cv::waitKey(30);

        if (key == 'Y' || key == 'y')
            break;
    }
    //transform
    transformed_Xpoints = fourier(untransformed_points, 'x'); 
    transformed_Ypoints = fourier(untransformed_points, 'y'); 
   
    waitKey();
    //display
   Fdraw(transformed_Xpoints, transformed_Ypoints);

    return 0;
}


//Creates sliders that the user moves
//Takes grayscale version of input image and does edge detection through "canny" algorithm
//findcontours uses edge detected mat and places sets of points into groups (contours)
//If multicontour it flattens sets of tuples into a single large set of tuples of (x,y)
//Downsample by throwing out every element except every nth,Scale size of points to fit onto canvas
//and gives each contour a random color then displays all of them
void thresh_callback(int, void*)
{
    thresh = getTrackbarPos("Canny thresh1:", "Source");
    thresh2 = getTrackbarPos("Canny thresh2:", "Source");
    Mat canny_output;
    Canny(src_gray, canny_output, thresh, thresh2);

    vector<vector<Point> > contours; 
    vector<Vec4i> hierarchy; //hierarchy uses a 4-tuple (next, previous, first child, parent), this uses the indices of the contours (-1 is none)
    findContours(canny_output, contours, hierarchy, RETR_LIST, CHAIN_APPROX_SIMPLE); //(input, output, hierarchy, retrieval mode, approximation method)

    //multi-contour or single
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

    //downsample to 1/nth
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

    float desiredSize = 400.0f; // change to half your window
    float scale = desiredSize / maxAbs;

    for (auto& p : untransformed_points) {
        p.x *= scale;
        p.y *= scale;
    }

    Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3); //8bit unsigned 3 channel colorspace
    //colors contours
    for (size_t i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256)); // a scalar in cv is a tuple, representing color (B,G,R)
        drawContours(drawing, contours, (int)i, color, 2, LINE_8, hierarchy, 0);
    }

    imshow("Contours", drawing);
    
    
}


//Takes a set of sets of (x,y) points and places them all into one large set
vector<Point2f> flatten(const vector<vector<Point> >& contours) {
    vector<Point2f> flattened;

    size_t total_size = 0;
    for (const vector<Point>& vec : contours) {
        total_size += vec.size();
    }
    flattened.reserve(total_size);

    for (const vector<Point>& vec : contours) {
        flattened.insert(flattened.end(), vec.begin(), vec.end());
    }
    return flattened;
}

// Draws a chain of epicycles (circles rotating on top of circles) from a set of Fourier terms
// Each entry in transformed_points is one frequency component of the Discrete Fourier Transform, in the form:[ (unused), (unused), frequency, amplitude/radius, phase ]
// The loop walks through the terms from lowest to highest frequency, chaining them tip-to-center: each circle's center
// sits at the tip of the previous one, and its own tip is offset by an overall "rotation"
// Summed together, these rotating vectors reconstruct one coordinate (x or y) of the original traced shape at the given point in time
// This is the geometric interpretation of a Fourier series as a sum of rotating vectors representing frequencies.
// Returns the final tip position (x, y), which is the reconstructed point for this instant.
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
        circle.setOutlineColor(sf::Color::White);
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

//Takes Fourier Transformed points and splits them into an X and Y epicycle
//Each reconstructs their coordinate respectively
//The tip of the X and Y chain of attached circles is combined into a final point v
//v is the true placement of the recontructed original shape
//The current v is appended to "wave" as the history of points drawn so a full image is constructed
//"time" is a cyclic timer running from zero to 2 pi, incrementing by a tiny amount every frame.
//Animation restarts at 2 pi
void Fdraw(vector<vector<double>> fourierX, vector<vector<double>> fourierY) {
    string fourier_drawing = "Fourier Drawing";
    sf::RenderWindow Fwindow(sf::VideoMode(1200, 1200), fourier_drawing);
    Fwindow.setFramerateLimit(120);
    sf::View view;
    view.setCenter(-350.f, 50.f); //places drawing in the center of window (can change)
    view.setSize(1200.f, 1200.f); //Window size (can change to fit users window)
    Fwindow.setView(view);
    long double time = 0.0;

    while (Fwindow.isOpen())
    {
        sf::Event event;
        while (Fwindow.pollEvent(event))
        {
            // "close requested" event by user closes window
            if (event.type == sf::Event::Closed)
                Fwindow.close();
        }
        Fwindow.clear(sf::Color::Black);

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

        //point limit
        if (wave.size() > 10000) {
            wave.pop_back();
        }

        long double dt = (2 * M_PI) / fourierY.size();
        time += dt;

        if (time > 2 * M_PI) {
            time = 0;
        }
        Fwindow.display();
    }
}
