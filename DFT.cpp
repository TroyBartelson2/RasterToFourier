#define _USE_MATH_DEFINES

#include <cmath>
#include <vector>
#include <math.h>
#include "DFT.h"
#include <opencv2/core/types.hpp>
#include <iostream>

using namespace cv;
using namespace std;

//Takes in a set of points and returns the scale of each frequency that occurred in the data set, in a set of scalars
//i.e. first element in return represents frequency of 0hz, second, frequency of 1 every 2 pi cycle
vector<vector<double>> fourier(vector<Point2f>& x, const char coord) {
	const int N = x.size();

	vector<vector<double>> X(N, vector<double>(5));
	if (coord == 'x') {
		for (int k = 0; k < N; k++) {
			double Im = 0.0;
			double Re = 0.0;

			for (int n = 0; n < N; n++) {
				const long double frequency = 2 * M_PI * k * n / N;
				Re += x[n].x * cos(frequency);
				Im += -x[n].x * sin(frequency);
			}
			Re = Re / N;
			Im = Im / N;
			double freq = k;
			double amp = sqrt(Re * Re + Im * Im);
			double phase = atan2(Im, Re);
			X[k][0] = Re;
			X[k][1] = Im;
			X[k][2] = freq;
			X[k][3] = amp;
			X[k][4] = phase;
		}
	}
	else if (coord == 'y') {
		for (int k = 0; k < N; k++) {
			double Im = 0.0;
			double Re = 0.0;

			for (int n = 0; n < N; n++) {
				const long double frequency = 2 * M_PI * k * n / N;
				Re += x[n].y * cos(frequency);
				Im += -x[n].y * sin(frequency);
			}
			Re = Re / N;
			Im = Im / N;

			double freq = k;
			double amp = sqrt(Re * Re + Im * Im);
			double phase = atan2(Im, Re);
			X[k][0] = Re;
			X[k][1] = Im;
			X[k][2] = freq;
			X[k][3] = amp;
			X[k][4] = phase;
		}
	}
	else {
		cout << "Invalid coordinate	chosen" << endl;
	}
	//sorting algorithm for circle size, largest to smallest
	sort(X.begin(), X.end(), [](const vector<double>& a, const vector<double>& b) 
		{	return a[3] > b[3];		});
	return X;
}