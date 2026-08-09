# RasterToFourier
A program to draw raster images using a Fourier algorithm

TROY BARTELSON 2026

# Math
The program traces the contours of an input image and redraws them in a continuous way using a set of circles each rotating at a distinct frequency.
Image processed points are put through a Discrete Fourier Transform. With this transform any set of points can be broken down into its frequencies and amplitude of those frequencies.
I use this data of the points to create rotating circles, representing frequency (speed of rotation) and amplitude (size of circle) and attach them tip to tip.
The final tip of each is them used as the X and Y position of the drawhead.

# Instructions:
Place any JPEG into the folder and rename to "input.jpg"
Open FourierDrawTest.exe
Enter downsampling rate n. This is calculated as 1/n total points sampled.
If you want more than just the largest contour, enter "Y" (Must be capitalized) 
Move thresh sliders until number of colors(contours) are minimized to a reasonable amount.
Type y or close both windows when ready to view the drawing.

# Demo
[![Watch the video](https://img.youtube.com/vi/jwA9FcdP8u0/hqdefault.jpg)](https://www.youtube.com/embed/jwA9FcdP8u0)

The code is a combination of own work along with source code from OpenCV, SFML, and The Coding Train's Youtube video "Coding Challenge #130.1: Drawing with Fourier Transform and Epicycles"