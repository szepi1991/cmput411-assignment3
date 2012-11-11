/*
 * geometry.h
 *
 *  Created on: 2012-11-11
 *      Author: david
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <cmath>

class Point {
private:
	float mx, my, mz;
public:
	Point (float x, float y, float z) : mx(x), my(y), mz(z) {};
	Point (Point const& o) : mx(o.mx), my(o.my), mz(o.mz) {};
	Point () {};
	float x() const {return mx;}
	float y() const {return my;}
	float z() const {return mz;}

	Point & operator*=(float a) { mx*=a; my*=a; mz*=a; return *this; }
	const Point operator*(float a) const { return Point(*this) *= a; }
	Point & operator+=(Point const& o) { mx += o.mx; my += o.my; mz += o.mz; return *this; }
	const Point operator+(Point const& o) const { return Point(*this) += o; }
	Point & operator-=(Point const& o) { return (*this)+=o*(-1); }
	const Point operator-(Point const& o) const { return Point(*this) -= o; }

	float dot(Point const& o) const { return (mx*o.mx + my*o.my + mz*o.mz); }
	float getLengthSqr() const { return dot(*this); }
	float getLength() const {return std::sqrt( getLengthSqr() ); }

	// TODO throws 0 if index is out of bounds
	float get(unsigned i) throw(int) {
		switch (i) {
		case 0: return mx;
		case 1: return my;
		case 2: return mz;
		default: throw(0);
		}
	}

	// TODO this is not suggested apparently.. but we'll see
	std::ostream& operator<<(std::ostream& os) {
		os << "(" << mx << ", " << my << ", " << mz << ")";
		return os;
	}
};



#endif /* GEOMETRY_H_ */
