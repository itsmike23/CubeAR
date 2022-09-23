#include <GLFW/glfw3.h>

#include <Windows.h>
#include <iostream>
#include <iomanip>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

#include "PoseEstimation.h"
#include "MarkerTracker.h"

using namespace std;

cv::VideoCapture cap;

// Camera settings
const int camera_width = 640;
const int camera_height = 480;
const int virtual_camera_angle = 30;
unsigned char bkgnd[camera_width * camera_height * 3];

GLfloat rotationX;
GLfloat rotationY;
bool polygonMode;

/* Keyboard control */
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	std::cout << key << std::endl;

	const GLfloat rotationSpeed = 10;

	// actions are GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT
	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		switch (key)
		{
		case GLFW_KEY_W:
			rotationX -= rotationSpeed;
			break;
		case GLFW_KEY_S:
			rotationX += rotationSpeed;
			break;
		case GLFW_KEY_D:
			rotationY += rotationSpeed;
			break;
		case GLFW_KEY_A:
			rotationY -= rotationSpeed;
			break;
		case GLFW_KEY_T:
			if (!polygonMode) {
				polygonMode = true;
			}
			else {
				polygonMode = false;
			}
			break;
		default:
			break;
		}
	}
}

/* Draw a Cube */
void DrawCube(GLfloat centerPosX, GLfloat centerPosY, GLfloat centerPosZ, GLfloat edgeLength)
{
	GLfloat halfSideLength = edgeLength * 0.5f;

	GLfloat vertices[] =
	{
		// front face
		centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
		centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top right
		centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom right
		centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left

		// back face
		centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top left
		centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
		centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
		centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom left

		// left face
		centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
		centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
		centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
		centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left

		// right face
		centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
		centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
		centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
		centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // bottom left

		// top face
		centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // top left
		centerPosX - halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // top right
		centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ - halfSideLength, // bottom right
		centerPosX + halfSideLength, centerPosY + halfSideLength, centerPosZ + halfSideLength, // bottom left

		// top face
		centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength, // top left
		centerPosX - halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // top right
		centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ - halfSideLength, // bottom right
		centerPosX + halfSideLength, centerPosY - halfSideLength, centerPosZ + halfSideLength  // bottom left
	};

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertices);

	glDrawArrays(GL_QUADS, 0, 24);

	glDisableClientState(GL_VERTEX_ARRAY);
	
}

/* Enable video or webcam stream */
void initVideoStream(cv::VideoCapture& cap) {
	if (cap.isOpened())
		cap.release();

	cap.open(1);
	if (cap.isOpened() == false) {
		std::cout << "No webcam found, using a video file" << std::endl;
		cap.open("MarkerMovie.MP4");
		if (cap.isOpened() == false) {
			std::cout << "No video file found. Exiting." << std::endl;
			exit(0);
		}
	}
}

/* Program & OpenGL initialization */
void initGL(int argc, char* argv[]) {

	// Render 3D objects in video frame (OpenCV/OpenGL) - Start *******************************************************
	
	// For our connection between OpenCV/OpenGL
	// Pixel storage/packing stuff -> how to handle the pixel on the graphics card
	// For glReadPixels -> Pixel representation in the frame buffer
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	// For glTexImage2D -> Define the texture image representation
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Turn the texture coordinates from OpenCV to the texture coordinates OpenGL
	glPixelZoom(1.0, -1.0);

	// Render 3D objects in video frame (OpenCV/OpenGL) - End *******************************************************

	// Enable and set colors
	glEnable(GL_COLOR_MATERIAL);
	glClearColor(0, 0, 0, 1.0);

	// Enable and set depth parameters
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	//Light parameters
	GLfloat light_amb[] = { 0.2, 0.2, 0.2, 1.0 };
	GLfloat light_pos[] = { 1.0, 1.0, 1.0, 0.0 };
	GLfloat light_dif[] = { 0.7, 0.7, 0.7, 1.0 };

	// Enable lighting
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_amb);
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_dif);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

/* OpenGL Rendering */
void display(GLFWwindow* window, const cv::Mat& img_bgr, float resultMatrix[16]) {

	// Render 3D objects in video frame (OpenCV/OpenGL) - Start *****************************************************************

	// Copy picture data into bkgnd array
	memcpy(bkgnd, img_bgr.data, sizeof(bkgnd));

	// Render 3D objects in video frame (OpenCV/OpenGL) - End *****************************************************************

	// Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Needed for rendering the real camera image
	glMatrixMode(GL_MODELVIEW);
	// No position changes
	glLoadIdentity();

	// Render 3D objects in video frame (OpenCV/OpenGL) - Start *****************************************************************

	// Not needed since we are dealing with our background image
	glDisable(GL_DEPTH_TEST);

	// Specifies which matrix stack is the target for subsequent matrix operations
	// -> Three values are accepted: GL_MODELVIEW, GL_PROJECTION, and GL_TEXTURE
	glMatrixMode(GL_PROJECTION);
	// Push the projection matrix (frustum) -> frustum will be saved on the stack
	glPushMatrix();
	glLoadIdentity();
	// In the ortho view all objects stay the same size at every distance
	glOrtho(0.0, camera_width, 0.0, camera_height, -1, 1);

	// -> Render the camera picture as background texture
	// Making a raster of the image -> -1 otherwise overflow
	glRasterPos2i(0, camera_height - 1);
	// Load and render the camera image -> unsigned byte because of bkgnd as unsigned char array
	// bkgnd 3 channels -> pixelwise rendering
	glDrawPixels(camera_width, camera_height, GL_BGR_EXT, GL_UNSIGNED_BYTE, bkgnd);

	// Go back to the previous projection -> frustum
	glPopMatrix();

	// Activate depth -> that snowman can be scaled with depth
	glEnable(GL_DEPTH_TEST);

	// Move to marker-position
	glMatrixMode(GL_MODELVIEW);

	// -> Transpose the Modelview Matrix
	float resultTransposedMatrix[16];
	for (int x = 0; x < 4; ++x) {
		for (int y = 0; y < 4; ++y) {
			// Change columns to rows
			resultTransposedMatrix[x * 4 + y] = resultMatrix[y * 4 + x];
		}
	}

	// Load the transpose matrix
	glLoadMatrixf(resultTransposedMatrix);

	// Rotate 90 desgress in x-direction
	glRotatef(-90, 1, 0, 0);
	// Scale down!
	glScalef(0.03, 0.03, 0.03);

	// Render 3D objects in video frame (OpenCV/OpenGL) - End *****************************************************************

	// Draw Cube
	if (polygonMode) {
		glColor3f(1.0f, 0.5f, 0.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glColor3f(1.0f, 0.5f, 0.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	glPushMatrix();
	glTranslatef(0, 0, 0);
	glRotatef(rotationX, 1, 0, 0);
	glRotatef(rotationY, 0, 1, 0);
	DrawCube(0, 0, 0, 1);
	glPopMatrix();
}

/* Setup Frustum */
void reshape(GLFWwindow* window, int width, int height) {
	// Set a whole-window viewport
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);

	// Create a perspective projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// The camera should be calibrated -> a calibration results in the projection matrix -> then load the matrix
	// -> into GL_PROJECTION
	// -> adjustment of FOV is needed for each camera
	float ratio = (GLfloat)width / (GLfloat)height;

	float near_ = 0.01f, far_ = 100.f;
	float top = tan((double)(virtual_camera_angle * M_PI / 360.0f)) * near_;
	float bottom = -top;
	float left = ratio * bottom;
	float right = ratio * top;
	glFrustum(left, right, bottom, top, near_, far_);
}


int main(int argc, char* argv[]) {
	GLFWwindow* window;

	// Initialize the library
	if (!glfwInit())
		return -1;

	// Initialize the window system
	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(camera_width, camera_height, "CubeAR", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	// Set callback functions for GLFW
	glfwSetFramebufferSizeCallback(window, reshape);
	// Define where the rendering-thread should render the GLFW context
	glfwMakeContextCurrent(window);
	// The minimum number of screen updates to wait for until the buffers are swapped by glfwSwapBuffers
	glfwSwapInterval(1);

	// enable Keyboard control
	glfwSetKeyCallback(window, keyCallback);

	int window_width, window_height;
	glfwGetFramebufferSize(window, &window_width, &window_height);
	reshape(window, window_width, window_height);

	// Initialize the GL library
	initGL(argc, argv);

	// Setup OpenCV
	cv::Mat img_bgr;
	// Get video stream
	initVideoStream(cap);
	// [m]
	const double kMarkerSize = 0.045;
	// Constructor with the marker size
	MarkerTracker markerTracker(kMarkerSize);

	float resultMatrix[16];
	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window)) {
		// Capture here
		cap >> img_bgr;

		// Make sure that we got a frame -> otherwise crash
		if (img_bgr.empty()) {
			std::cout << "Could not query frame. Trying to reinitialize." << std::endl;
			initVideoStream(cap);
			// Wait for one sec.
			cv::waitKey(1000);
			continue;
		}

		// Track a marker and get the pose of the marker
		markerTracker.findMarker(img_bgr, resultMatrix);

		// Render here
		display(window, img_bgr, resultMatrix);

		// Swap front and back buffers
		glfwSwapBuffers(window);

		// Poll for and process events
		glfwPollEvents();
	}

	// Important -> Avoid memory leaks!
	glfwTerminate();

	return 0;
}
