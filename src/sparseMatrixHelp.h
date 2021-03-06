/*
 * sparesMatrixHelp.h
 *
 *  Created on: 2012-11-12
 *      Author: david
 */

#ifndef SPARSEMATRIXHELP_H_
#define SPARSEMATRIXHELP_H_

#include <Eigen/Sparse>
#include "geometry.h"

// prints "rowNum indOfNonzero indOfNonzero etc"
template <class T>
inline void printSparseRow(std::ostream& out, Eigen::SparseMatrix<T> const & toPrint, int row) {
//	const int* a = toPrint.outerIndexPtr();
//	for (int i = 0; i < toPrint.outerSize(); ++i) {
//
//	}
	// probably not the fastest..
	out << row;
	for (int col = 0; col < toPrint.cols(); ++col) {
		if (toPrint.coeff(row, col) != 0) out << " " << col;
	}
	out << std::endl;
}

//// vec needs exactly 1  col
//template <class T>
//inline Eigen::SparseMatrix<T> delta(Eigen::MatrixBase<T> const& vec ) {
//	if (vec.cols() != 1) throw 0;
//	unsigned r = vec.rows();
//
//	typedef Eigen::Triplet<double> Tr;
//	std::vector<Tr> deltaTriplets;
//	deltaTriplets.reserve(r);
//	for (unsigned i = 0; i < r; ++i) {
//		deltaTriplets.push_back(Tr(i, i, vec(i)));
//	}
//
//	Eigen::SparseMatrix<T> delta(r, r);
//	delta.reserve(r);
//	delta.setFromTriplets(deltaTriplets.begin(), deltaTriplets.end());
//	return delta;
//}

// vec needs exactly 1  col
inline Eigen::SparseMatrix<double> delta(Eigen::VectorXd const& vec ) {
	unsigned r = vec.rows();

	typedef Eigen::Triplet<double> Tr;
	std::vector<Tr> deltaTriplets;
	deltaTriplets.reserve(r);
	for (unsigned i = 0; i < r; ++i) {
		deltaTriplets.push_back(Tr(i, i, vec(i)));
	}

	Eigen::SparseMatrix<double> delta(r, r);
	delta.reserve(r);
	delta.setFromTriplets(deltaTriplets.begin(), deltaTriplets.end());
	return delta;
}

inline Eigen::Vector4f getVectorFormPoint(Point const& p) {
	Eigen::Vector4f v;
	v << p.x(), p.y(), p.z(), 1;
	return v;
}
inline Eigen::Vector4f getVectorFormDirection(Point const& p) {
	Eigen::Vector4f v;
	v << p.x(), p.y(), p.z(), 0;
	return v;
}

#endif /* SPARSEMATRIXHELP_H_ */
