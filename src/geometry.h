/*
 * geometry.h
 *
 *  Created on: 2012-11-11
 *      Author: david
 */

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

#include <cmath>
#include <Eigen/Dense>

#include "tools.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


class Triangle;
class Point;
class Sphere;
class LineSegment;

class Point {
private:
	float mx, my, mz;
public:
	Point (float x, float y, float z) : mx(x), my(y), mz(z) {};
	Point (Point const& o) : mx(o.mx), my(o.my), mz(o.mz) {};
	Point (Eigen::Vector3f const & p) : mx(p(0,0)), my(p(1,0)), mz(p(2,0)) {};
	// makes 0 point
	Point () : mx(0), my(0), mz(0) {};
	float x() const {return mx;}
	float y() const {return my;}
	float z() const {return mz;}
	virtual ~Point() {};

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

	friend bool intersectLineSegWithTriangle(LineSegment const & l, Triangle const & t);
	friend std::ostream& operator<< (std::ostream &out, Point const& pt);
};

inline std::ostream& operator<<(std::ostream& os, const Point& pt) {
	os << "(" << pt.mx << ", " << pt.my << ", " << pt.mz << ")";
	return os;
}

class Sphere {
private:
	Point c;
	float rad;
public:
	Sphere(Point const& center, float radius) : c(center), rad(radius) {};
	virtual ~Sphere() {};
	// if it returns false, they don't intersect. That's all we guarantee!
	bool intersects(Sphere const& o) const {
		return ((c-o.c).getLength() > (rad+o.rad));
	}
};

class LineSegment {
private:
	Point p0, d;
	Sphere bounding;
public:
	// if endpoints == true, e1 and e2 are taken to be the endpoints of the line segment.
	// If it's false, then e1 is the translation to one endpoint, e2 is the
	// direction and length of the segment
	LineSegment(Point const& e1, Point const& e2, bool endPoints = true) : bounding(e1, 1000.0f) { // fake
		if (endPoints) {
			p0 = e1;
			d = e2-e1;
		} else {
			p0 = e1;
			d = e2;
		}
//		bounding = Sphere(p0, d.getLength()); // FIXME actually need this!
		// FIXME PROBLEM!!!!!!!!!!! when setting the above bounding thing to 0 or 1000 running speed doesn't change!
	};
	virtual ~LineSegment() {};
	friend bool intersectLineSegWithTriangle(LineSegment const & l, Triangle const & t);
	friend std::ostream& operator<< (std::ostream &out, LineSegment const& l);
	// always recalculate..
	Sphere const& getBoundSphere() const { return bounding; }
	Point const& getTrans() const { return p0; }
	Point const& getShift() const { return d; }

	void display() const {
		Point e2 = p0 + d;
		glBegin(GL_LINES);
			glVertex3f(p0.x(), p0.y(), p0.z());
			glVertex3f(e2.x(), e2.y(), e2.z());
		glEnd();
	}
};

inline std::ostream& operator<< (std::ostream &out, LineSegment const& l) {
	out <<  "L[" << l.p0 << "+t*" << l.d << "]";
	return out;
}

class Triangle {
private:
	Point v1, v2, v3;
	Sphere bounding;
public:
	Triangle(Point const& e1, Point const& e2, Point const& e3) :
		v1(e1), v2(e2), v3(e3), bounding(v1, std::sqrt((v1-v2).getLengthSqr()+(v2-v3).getLengthSqr())) {};
	virtual ~Triangle() {};
	friend bool intersectLineSegWithTriangle(LineSegment const & l, Triangle const & t);
	friend std::ostream& operator<< (std::ostream &out, Triangle const& t);
	Sphere const& getBoundSphere() const {return bounding;}

	Point const& getPoint(unsigned i) const {
		switch (i) {
		case 0: return v1; break;
		case 1: return v2; break;
		case 2: return v3; break;
		default: throw(0);
		}
	}
};

inline std::ostream& operator<< (std::ostream &out, Triangle const& t) {
	out <<  "T[" << t.v1 << "," << t.v2 << ", " << t.v3 << "]";
	return out;
}


namespace interSectMatr {
	static Eigen::Matrix3f A;
	static Eigen::Matrix3f Ainv;
	static Eigen::Vector3f b;
	static Eigen::Vector3f x;
	static bool invertable;
}

inline bool intersectLineSegWithTriangle(LineSegment const & l, Triangle const & t) {
	// TODO later actually pass in a bounding box for the line
//	if (!l.getBoundSphere().intersects(t.getBoundSphere())) return false;

	Point col1 = t.v1-t.v2, col2 = t.v1-t.v3;
//	interSectMatr::A << col1.mx, col1.my, col1.mz,
//			col2.mx, col2.my, col2.mz,
//			l.d.mx, l.d.my, l.d.mz;
//	// actual matrix is transpose of this!!!
//	interSectMatr::A.transposeInPlace();
	interSectMatr::A << col1.mx, col2.mx, l.d.mx,
			col1.my, col2.my, l.d.my,
			col1.mz, col2.mz, l.d.mz;

	interSectMatr::A.computeInverseWithCheck(interSectMatr::Ainv, interSectMatr::invertable);
	if (!interSectMatr::invertable) return false;

	col1 = t.v1-l.p0;
	interSectMatr::b << col1.mx, col1.my, col1.mz;
	interSectMatr::x = interSectMatr::Ainv * interSectMatr::b;

	// let x = [b, g, l]. Then intersects iff
	// 0 <= b, g. b+g <= 1. 0 < l < 1
	return (interSectMatr::x(0, 0) >= 0 && interSectMatr::x(1,0) >= 0 &&
			interSectMatr::x(0,0) + interSectMatr::x(1,0) <= 1 &&
			EPS < interSectMatr::x(2,0) && interSectMatr::x(2,0) < 1-EPS ); // TODO or use 0?
}


//inline bool intersectLineSegWithTriangle(LineSegment const & l, Triangle const & t) {
////	if (debug::ison(debug::EVERYTHING))
////		std::cout << "intersecting " << l << " with " << t << ". Formed matrix is:" << std::endl;
//	Point col1 = t.v1-t.v2, col2 = t.v1-t.v3;
//	Eigen::Matrix3f A; // = Eigen::Matrix3f::Zero();
//	A << col1.mx, col1.my, col1.mz.
//			col2.mx, col2.my, col2.mz,
//			l.d.mx(), l.d.my(), l.d.mz;
//	// actual matrix is transpose of this!!!
//
////	if (debug::ison(debug::EVERYTHING))
////		std::cout << "A=" << std::endl << A << std::endl;
//
//	if (std::abs(A.determinant()) < EPS) {
////		if (debug::ison(debug::EVERYTHING))
////			std::cout << "--> Determinant 0." << std::endl;
//		return false;
//	}
//	// since det of trans(A) = A, we can do it now
//	A.transposeInPlace();
//
//	Eigen::Vector3f b;
//	col1 = t.v1-l.p0;
//	b << col1.mx, col1.my, col1.mz;
//
////	if (debug::ison(debug::EVERYTHING))
////		std::cout << "b=" << std::endl << b << std::endl;
//
//
//	Eigen::Vector3f x = A.partialPivLu().solve(b);
////	Eigen::Vector3f x = A.colPivHouseholderQr().solve(b);
//	// let x = [b, g, l]. Then intersects iff
//	// 0 <= b, g. b+g <= 1. 0 < l < 1
////	if (debug::ison(debug::EVERYTHING)) {
////		std::cout << "intersection resulted in parameters:" << std::endl
////				<< "\tBeta=" << x(0,0) << ", Gamma=" << x(1,0) << ", Lambda=" << x(2, 0) << std::endl;
////	}
//	return (x(0, 0) >= 0 && x(1,0) >= 0 && x(0,0)+x(1,0) <= 1
//			&& 0 < x(2,0) && x(2,0) < 1 );
//}

inline void testLineSegWithTriangleIntersection() {

	Triangle t(Point(-1, -1, 0), Point(1, -1, 0), Point(0, 1, 0));

	LineSegment l(Point(-12, 3, 0), Point(0,0, 0));
	assert( !intersectLineSegWithTriangle(l, t) ); // in same plane

	// by endpoint
	l = LineSegment(Point(-1, -1, -1), Point(1, 1, 1));
	assert( intersectLineSegWithTriangle(l, t) ); // through (0,0,0)

	l = LineSegment(Point(-3, -3, -1), Point(-2, -2, 1));
	assert( !intersectLineSegWithTriangle(l, t) ); // too far

	l = LineSegment(Point(-1, -1, -1), Point(0, 0, 1));
	assert( intersectLineSegWithTriangle(l, t) ); // ??

	l = LineSegment(Point(3, 2, -1), Point(1, 1, 1));
	assert( !intersectLineSegWithTriangle(l, t) ); // too far

	l = LineSegment(Point(3, 2, -1), Point(14, 1123, 14));
	assert( !intersectLineSegWithTriangle(l, t) ); // too far

	// by translation + segment
	l = LineSegment(Point(-1, -1, -1), Point(2, 2, 2), false);
	assert( intersectLineSegWithTriangle(l, t) ); // through (0,0,0)

	l = LineSegment(Point(-3, -3, -1), Point(1, 1, 1.3), false);
	assert( !intersectLineSegWithTriangle(l, t) ); // too far

}

#endif /* GEOMETRY_H_ */

