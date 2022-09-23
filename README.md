# CubeAR

## Project Description
This repository contains code of a marker-tracking based AR Cube visualizer. To achieve this openCV and openGL were used.


## How to play
* W: Cube neg. rotation around x-axis
* S: Cube pos. rotation around x-axis
* A: Cube neg. rotation around y-axis
* D: Cube pos. rotation around y-axis
* T: enable/disable Board Background

## Setup

Setup was tested on windows machine

1. Install OpenCV 
Link: https://opencv.org/releases/

2. Install Visual Studio 2019

3. Setup the following within your VS project

**VC++ Directories:**

Include Directories:
C:\opencv\build\include

Library Directories:
C:\opencv\build\x64\vc15\lib

**Linker:**

Input - Additional Dependencies

* glfw3.lib
* opengl32.lib
* glew32.lib
* Glu32.lib
* opencv_world3414d.lib
* User32.lib
* Shell32.lib
* Gdi32.lib
* winmm.lib