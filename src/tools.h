/*
 * tools.h
 *
 *  Created on: 2012-10-16
 *      Author: david
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include "myexceptions.h"

#include <string>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <iostream>


#define MYINFO true
#define SECtoMSEC 1000.0f // normal speed
//#define SECtoMSEC 10000.0f

#define PI 3.14159265359
#define EPS 0.00001

#define degToRad(x) (x*PI/180)
#define radToDeg(x) (x*180/PI)

#define sqr(x) (x*x)


// TODO optimize this perhaps..
namespace debug {
	enum InfoLevel {NONE, LITTLE, DETAILED, EVERYTHING};
	static const InfoLevel level = DETAILED;
	inline bool ison(InfoLevel a) {
		return ((unsigned) level >= (unsigned) a );
	}
}

inline std::string strToUpper(std::string strToConvert) {
    std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), ::toupper);

    return strToConvert;
}

inline void confirmParse(std::string expected, std::string got) throw(ParseException) {
	if (expected.compare(got) != 0) throw (ParseException(expected, got));
}

template <class T>
inline void normalize(T* a, T* b, T* c) {
	T lenSqr = sqr(*a) + sqr(*b) + sqr(*c);
	if (lenSqr < 1+EPS && lenSqr > 1+EPS) return;
	T len = sqrt(lenSqr);
	*a /= len;
	*b /= len;
	*c /= len;
}

// prints a 4x4 matrix
inline void print4x4Matrix(float * toPrint) {
	std::cout << std::fixed;
	std::cout.precision(6);
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			std::cout << "\t" << toPrint[i*4+j];
		}
		std::cout << std::endl;
	}
}

inline bool same4x4Matrix(float * a, float * b) {
	for (unsigned i = 0; i < 16; ++i) {
		if (abs(a[i] - b[i]) > EPS) return false;
	}
	return true;
}


#endif /* TOOLS_H_ */
