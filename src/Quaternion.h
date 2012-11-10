/*
 * Quaternion.h
 *
 *  Created on: 2012-10-21
 *      Author: david
 */

#ifndef QUATERNION_H_
#define QUATERNION_H_

class Quaternion {
private:
	float w, x, y, z; // q = w + xi + yj + zk

public:
	Quaternion();
	Quaternion(float angleRad, float x, float y, float z);
	Quaternion(Quaternion const & toCopy);
	virtual ~Quaternion();

	void normalize();

	Quaternion & operator*= (const Quaternion &rhs);
	Quaternion operator* (const Quaternion &other) const {
		Quaternion n(*this);
		n *= other;
		return n;
	}

	Quaternion & operator+= (const Quaternion &rhs);
	Quaternion operator+ (const Quaternion &other) const {
		Quaternion n(*this);
		n += other;
		return n;
	}

	Quaternion & operator*= (float scalar);
	Quaternion operator* (float scalar) const {
		Quaternion n(*this);
		n *= scalar;
		return n;
	}

	void getRotation(float * matrix);
	void printRotMatrix();

	static float dotProd(Quaternion const &a, Quaternion const &b) {
		return (a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z);
	}

	static void slerp(Quaternion const &a, Quaternion b, float t, Quaternion &ret);
	static float getAngle(Quaternion const &a, Quaternion const &b);
};

#endif /* QUATERNION_H_ */
