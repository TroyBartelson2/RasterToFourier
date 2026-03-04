#ifndef DFT_H
#define DFT_H

#include <vector>
#include <opencv2/core/types.hpp>
using namespace cv;
using namespace std;


// Function declaration (prototype)
vector<vector<double>> fourier(vector<Point2f>& x, const char coord);

#endif