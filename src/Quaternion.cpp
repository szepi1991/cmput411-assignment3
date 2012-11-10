/*
 * Quaternion.cpp
 *
 *  Created on: 2012-10-21
 *      Author: david
 */

#include "Quaternion.h"
#include "tools.h"
#include <cmath>

Quaternion::Quaternion() {
	w = 1;
	x = y = z = 0;
}

Quaternion::Quaternion(float angleRad, float x, float y, float z) {
	w = cos(angleRad/2);
	float s = sin(angleRad/2);
	this->x = x*s;
	this->y = y*s;
	this->z = z*s;
}

Quaternion::Quaternion(Quaternion const & toCopy) {
	w = toCopy.w;
	x = toCopy.x;
	y = toCopy.y;
	z = toCopy.z;
}

Quaternion & Quaternion::operator*= (const Quaternion &rhs) {
	float tw, tx, ty, tz;
	tw = w*rhs.w - x*rhs.x - y*rhs.y - z*rhs.z;
	tx = w*rhs.x + x*rhs.w + y*rhs.z - z*rhs.y;
	ty = w*rhs.y + y*rhs.w - x*rhs.z + z*rhs.x;
	tz = w*rhs.z + z*rhs.w + x*rhs.y - y*rhs.x;
	w = tw; x = tx; y = ty; z = tz;
	return *this;
}

Quaternion & Quaternion::operator+= (const Quaternion &rhs) {
	w += rhs.w;
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	return *this;
}

// NOTE: not unit length any more...!
Quaternion & Quaternion::operator*= (float scalar) {
	w *= scalar;
	x *= scalar;
	y *= scalar;
	z *= scalar;
	return *this;
}

void Quaternion::normalize() {
	// length:
	double length = sqrt(w*w + x*x + y*y + z*z);

	if (abs(length - 1) < EPS) return; // we need this so we don't get into
										// an infinite loop!
	else (*this) *= (1/length);
}


float Quaternion::getAngle(Quaternion const &a, Quaternion const &b) {
	return (acos(a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z));
}

// puts result in 'ret'
void Quaternion::slerp(Quaternion const &a, Quaternion b, float t, Quaternion &ret){

	// reverse sign if dot prod < 0
	if (dotProd(a, b) < 0) { b *= -1; }
	float angle = getAngle(a, b);
	float sc1, sc2;

	// like suggested in the book, for small angles we use the sin(a) = a appr.
	if (angle > 0.00001) {
		sc1 = sin( (1-t)*angle ) / sin( angle );
		sc2 = sin( t*angle ) / sin( angle );
	} else {
		sc1 = 1 - t;
		sc2 = t;
	}

	ret.w = sc1*a.w + sc2*b.w;
	ret.w =  sc1*a.w + sc2*b.w;
	ret.x =  sc1*a.x + sc2*b.x;
	ret.y =  sc1*a.y + sc2*b.y;
	ret.z =  sc1*a.z + sc2*b.z;
}

// first normalizes this quaternion!
// fills up the 4x4 matrix with the rotation corresponding to this quaternion
void Quaternion::getRotation(float * matrix) {
	normalize();

	// remember opengl matrix is column first
	matrix[0] = w*w + x*x - y*y - z*z;
	matrix[1] = 2*x*y + 2*w*z;
	matrix[2] = 2*x*z - 2*w*y;
	matrix[3] = 0;

	matrix[4] = 2*x*y - 2*w*z;
	matrix[5] = w*w - x*x + y*y - z*z;
	matrix[6] = 2*y*z + 2*w*x;
	matrix[7] = 0;

	matrix[8] = 2*x*z + 2*w*y;
	matrix[9] = 2*y*z - 2*w*x;
	matrix[10] = w*w - x*x - y*y + z*z;
	matrix[11] = 0;

	matrix[12] = 0;
	matrix[13] = 0;
	matrix[14] = 0;
	matrix[15] = 1;
}

// normalizes this quaternion if needed
void Quaternion::printRotMatrix() {
	float temp[16];
	getRotation(temp);
	print4x4Matrix(temp);
}


Quaternion::~Quaternion() {
}
