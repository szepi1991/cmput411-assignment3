///*
// * Matrix.h
// *
// *  Created on: 2012-10-21
// *      Author: david
// */
//
//#ifndef MATRIX_H_
//#define MATRIX_H_
//
//#include <iostream>
//#include <algorithm>
//
//// for square matrices!
//template<class T>
//class Matrix4x4 {
//public:
//	// the data stored like so: [col1; col2; col3; col4]
//	T data[16];
//	static T e1[]; // don't change!!
//	static T e2[]; // don't change!!
//	static T e3[]; // don't change!!
//
//	Matrix4x4() {
//		for (int i = 0; i < 4; ++i) {
//			for (int j = 0; j < 4; ++j) {
//				if (i == j)
//					data[i*4+j] = 1;
//				else
//					data[i*4+j] = 0;
//			}
//		}
//	}
//
//	// note this function makes a copy!
//	Matrix4x4(T const * init) {
//		setTo(init);
//	}
//
//	virtual ~Matrix4x4() {}
//
//
//	// note this function makes a copy of the passed in thing!
//	void setTo(T const * init) {
//		std::copy ( init, init+16, data );
//	}
//
//	// vec = M*vec.
//	void multVector(T * vec) const {
//		T res[] = {0, 0, 0, 0};
//		for (int j = 0; j < 4; ++j) { // row
//			for (int i = 0; i < 4; ++i) {
//				res[i] += data[i*4+j]*vec[j];
//			}
//		}
//		for (int i = 0; i < 4; ++i)
//			vec[i] = res[i];
//	}
//
//	void printMatrix() const {
//		std::cout << std::fixed;
//		std::cout.precision(4);
//		for (int j = 0; j < 4; ++j) {
//			for (int i = 0; i < 4; ++i) {
//				std::cout << "\t" << data[i*4+j];
//			}
//			std::cout << std::endl;
//		}
//	}
//
//	// 4 x 1
//	static void printVector(T const * vec) {
//		std::cout << std::fixed;
//		std::cout.precision(4);
//		for (int j = 0; j < 4; ++j) {
//			std::cout << "\t" << vec[j];
//		}
//		std::cout << std::endl;
//	}
//
//	// only works for openGL transformation matricies with a rot and a translation only!
//	// assume it's invertible
//	Matrix4x4<T> getInverse() const {
//		// http://stackoverflow.com/questions/1148309/inverting-a-4x4-matrix
//		T inv[16], det;
//		int i;
//
//		inv[0] = data[5]  * data[10] * data[15] -
//				 data[5]  * data[11] * data[14] -
//				 data[9]  * data[6]  * data[15] +
//				 data[9]  * data[7]  * data[14] +
//				 data[13] * data[6]  * data[11] -
//				 data[13] * data[7]  * data[10];
//
//		inv[4] = -data[4]  * data[10] * data[15] +
//				  data[4]  * data[11] * data[14] +
//				  data[8]  * data[6]  * data[15] -
//				  data[8]  * data[7]  * data[14] -
//				  data[12] * data[6]  * data[11] +
//				  data[12] * data[7]  * data[10];
//
//		inv[8] = data[4]  * data[9] * data[15] -
//				 data[4]  * data[11] * data[13] -
//				 data[8]  * data[5] * data[15] +
//				 data[8]  * data[7] * data[13] +
//				 data[12] * data[5] * data[11] -
//				 data[12] * data[7] * data[9];
//
//		inv[12] = -data[4]  * data[9] * data[14] +
//				   data[4]  * data[10] * data[13] +
//				   data[8]  * data[5] * data[14] -
//				   data[8]  * data[6] * data[13] -
//				   data[12] * data[5] * data[10] +
//				   data[12] * data[6] * data[9];
//
//		inv[1] = -data[1]  * data[10] * data[15] +
//				  data[1]  * data[11] * data[14] +
//				  data[9]  * data[2] * data[15] -
//				  data[9]  * data[3] * data[14] -
//				  data[13] * data[2] * data[11] +
//				  data[13] * data[3] * data[10];
//
//		inv[5] = data[0]  * data[10] * data[15] -
//				 data[0]  * data[11] * data[14] -
//				 data[8]  * data[2] * data[15] +
//				 data[8]  * data[3] * data[14] +
//				 data[12] * data[2] * data[11] -
//				 data[12] * data[3] * data[10];
//
//		inv[9] = -data[0]  * data[9] * data[15] +
//				  data[0]  * data[11] * data[13] +
//				  data[8]  * data[1] * data[15] -
//				  data[8]  * data[3] * data[13] -
//				  data[12] * data[1] * data[11] +
//				  data[12] * data[3] * data[9];
//
//		inv[13] = data[0]  * data[9] * data[14] -
//				  data[0]  * data[10] * data[13] -
//				  data[8]  * data[1] * data[14] +
//				  data[8]  * data[2] * data[13] +
//				  data[12] * data[1] * data[10] -
//				  data[12] * data[2] * data[9];
//
//		inv[2] = data[1]  * data[6] * data[15] -
//				 data[1]  * data[7] * data[14] -
//				 data[5]  * data[2] * data[15] +
//				 data[5]  * data[3] * data[14] +
//				 data[13] * data[2] * data[7] -
//				 data[13] * data[3] * data[6];
//
//		inv[6] = -data[0]  * data[6] * data[15] +
//				  data[0]  * data[7] * data[14] +
//				  data[4]  * data[2] * data[15] -
//				  data[4]  * data[3] * data[14] -
//				  data[12] * data[2] * data[7] +
//				  data[12] * data[3] * data[6];
//
//		inv[10] = data[0]  * data[5] * data[15] -
//				  data[0]  * data[7] * data[13] -
//				  data[4]  * data[1] * data[15] +
//				  data[4]  * data[3] * data[13] +
//				  data[12] * data[1] * data[7] -
//				  data[12] * data[3] * data[5];
//
//		inv[14] = -data[0]  * data[5] * data[14] +
//				   data[0]  * data[6] * data[13] +
//				   data[4]  * data[1] * data[14] -
//				   data[4]  * data[2] * data[13] -
//				   data[12] * data[1] * data[6] +
//				   data[12] * data[2] * data[5];
//
//		inv[3] = -data[1] * data[6] * data[11] +
//				  data[1] * data[7] * data[10] +
//				  data[5] * data[2] * data[11] -
//				  data[5] * data[3] * data[10] -
//				  data[9] * data[2] * data[7] +
//				  data[9] * data[3] * data[6];
//
//		inv[7] = data[0] * data[6] * data[11] -
//				 data[0] * data[7] * data[10] -
//				 data[4] * data[2] * data[11] +
//				 data[4] * data[3] * data[10] +
//				 data[8] * data[2] * data[7] -
//				 data[8] * data[3] * data[6];
//
//		inv[11] = -data[0] * data[5] * data[11] +
//				   data[0] * data[7] * data[9] +
//				   data[4] * data[1] * data[11] -
//				   data[4] * data[3] * data[9] -
//				   data[8] * data[1] * data[7] +
//				   data[8] * data[3] * data[5];
//
//		inv[15] = data[0] * data[5] * data[10] -
//				  data[0] * data[6] * data[9] -
//				  data[4] * data[1] * data[10] +
//				  data[4] * data[2] * data[9] +
//				  data[8] * data[1] * data[6] -
//				  data[8] * data[2] * data[5];
//
//		det = data[0] * inv[0] + data[1] * inv[4] + data[2] * inv[8] + data[3] * inv[12];
//
//		if (det == 0)
//			return 0;
//
//		det = 1.0 / det;
//
//		for (i = 0; i < 16; i++)
//		        inv[i] = inv[i] * det;
//
//		return Matrix4x4(inv);
//	}
//
//};
//
//template <class T> T Matrix4x4<T>::e1[] = {1.0, 0.0, 0.0, 0.0};
//template <class T> T Matrix4x4<T>::e2[] = {0.0, 1.0, 0.0, 0.0};
//template <class T> T Matrix4x4<T>::e3[] = {0.0, 0.0, 1.0, 0.0};
//
//#endif /* MATRIX_H_ */
