/*
 * Camera.h
 *
 *  Created on: 2012-10-17
 *      Author: david
 */


#ifndef CAMERA_H_
#define CAMERA_H_

#include "Matrix4x4.h"

class Camera {
private:
//	double eyeXP, eyeYP, eyeZP;
//	double centerXP, centerYP, centerZP;
//	double upXP, upYP, upZP;

	double cameraTrans[16];
	float near, far;

public:
	Camera();
	virtual ~Camera();

	bool controlSpec(int key);
	bool control(unsigned char key);
	void view();
	void reset();
	void makeVisible(float xMin, float xMax,
						float yMin, float yMax, float zMin, float zMax);

	void rotateCamera(double angle, double x, double y, double z);
	void translateCamera(double x, double y, double z);

	float getNear() { return near; }
	float getFar() { return far; }
};

#endif /* CAMERA_H_ */
