/*
 * Camera.cpp
 *
 *  Created on: 2012-10-17
 *      Author: david
 */

#include "Camera.h"
#include "tools.h"

#include <cassert>
#include <cmath>
#include <iostream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#define PI 3.14159265359

Camera::Camera() {
	near = 3.0f;
	far = 150.0f;
//	reset();
//	//set the default camera: looking at origin from +5 on z axis
//	eyeXP = 8.0, eyeYP = 10.0, eyeZP = 15.0;
//	centerXP = 0, centerYP = 0, centerZP = 0.0;
//	upXP = 0, upYP = 1, upZP = 0;

}

// this resets the camera to the origin facing in default things blabla
// NEEDS TO BE CALLED IN SETUP!!
void Camera::reset() {
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glGetDoublev(GL_MODELVIEW_MATRIX, cameraTrans); // initialize it to identity --> move to resetScene()
	glPopMatrix();
}

// makes sure that the axis aligned box is visible in the camera (and y is up)
void Camera::makeVisible(float xMin, float xMax,
					float yMin, float yMax, float zMin, float zMax) {
	// TODO we can actually use the gluLookAt here and save the tansformation we get out of it
	float yPos = (yMax + yMin) / 2; // just the average
	float xPos, zPos;
	// find position (x, z) st when camera is facing along one of these axes
	// the box is fully in the view but and as big as possible
	float distX = xMax - xMin, distZ = zMax - zMin;
	float xAvr = ( xMax + xMin ) / 2;
	float zAvr = ( zMax + zMin ) / 2;
	if (distX > distZ) {
		xPos = xAvr;
		zPos = zMin - distX/2;
		// TODO check if y fits? --not--> move z more negative
		while (distX/2 + distZ > far) { far += 10; } // make sure it's in the picture
		// add a little extra so we can see the man from an angle
		xPos += 20;
		zPos -= 20;
	} else {
		zPos = zAvr;
		xPos = xMin - distZ/2;
		// TODO check if y fits? --not--> move x more negative
		while (distZ/2 + distX > far) { far += 10; } // make sure it's in the picture
		// add a little extra so we can see the man from an angle
		zPos += 20;
		xPos -= 20;
	}

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	gluLookAt(xPos, yPos, zPos, xAvr, yPos, zAvr, 0, 1.0, 0);
	glGetDoublev(GL_MODELVIEW_MATRIX, cameraTrans); // initialize it to identity --> move to resetScene()
	glPopMatrix();
}

/* Sets up the view. To be called before any drawing! */
void Camera::view() {
	glMatrixMode(GL_MODELVIEW);

//	if (DEBUG) {
//		std::cout << std::fixed;
//		std::cout.precision(6);
//		for (int i = 0; i < 4; ++i) {
//			for (int j = 0; j < 4; ++j) {
//				std::cout << "\t" << cameraTrans[i*4+j];
//			}
//			std::cout << std::endl;
//		}
//	}

	glMultMatrixd(cameraTrans);

}

// translate the camera by the given amounts (with respect to its local frame)
void Camera::translateCamera(double x, double y, double z) {

	glMatrixMode(GL_MODELVIEW); // make sure we have the right matrix selected
	glPushMatrix(); // make a copy
	glLoadIdentity(); // do nothing
	glTranslated(-x, -y, -z); // add the translation
	glMultMatrixd(cameraTrans); // now add already existing camera transformations, from the left!
	glGetDoublev(GL_MODELVIEW_MATRIX, cameraTrans); // save new rotation
	glPopMatrix(); // reset modelview to original settings

}

// rotate the camera (with respect to its local frame) by the given amount, about the given vector
void Camera::rotateCamera(double angle, double x, double y, double z) {

	glMatrixMode(GL_MODELVIEW); // make sure we have the right matrix selected
	glPushMatrix(); // make a copy
	glLoadIdentity(); // do nothing
	glRotated(-angle, x, y, z);
	glMultMatrixd(cameraTrans); // now add already existing camera transformations, from the left!
	glGetDoublev(GL_MODELVIEW_MATRIX, cameraTrans); // save new rotation
	glPopMatrix(); // reset modelview to original settings

}


// returns true iff the key had some effect
bool Camera::controlSpec(int key) {
	switch (key) {
	case GLUT_KEY_UP:
		translateCamera(0, 0.1, 0);
		break;
	case GLUT_KEY_DOWN:
		translateCamera(0, -0.1, 0);
		break;
	case GLUT_KEY_LEFT:
		translateCamera(-0.1, 0, 0);
		break;
	case GLUT_KEY_RIGHT:
		translateCamera(0.1, 0, 0);
		break;
	default:
		return false;
	}
	return true;
}

bool Camera::control(unsigned char key) {
	switch (key) {
	case 'I':
		translateCamera(0, 0, 0.1);
		break;
	case 'i':
		translateCamera(0, 0, -0.1);
		break;

	case 't':
		rotateCamera(-10.0, 1.0, 0.0, 0.0);
		break;
	case 'T':
		rotateCamera(10.0, 1.0, 0.0, 0.0);
		break;

	case 'a':
		rotateCamera(-10.0, 0.0, 1.0, 0.0);
		break;
	case 'A':
		rotateCamera(10.0, 0.0, 1.0, 0.0);
		break;

	case 'c':
		rotateCamera(-10.0, 0.0, 0.0, 1.0);
		break;
	case 'C':
		rotateCamera(10.0, 0.0, 0.0, 1.0);
		break;

	default:
		return false;
	}

	return true;
}

Camera::~Camera() {
}

