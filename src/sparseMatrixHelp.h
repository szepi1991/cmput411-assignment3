/*
 * sparesMatrixHelp.h
 *
 *  Created on: 2012-11-12
 *      Author: david
 */

#ifndef SPARESMATRIXHELP_H_
#define SPARSEMATRIXHELP_H_

#include <Eigen/Sparse>

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


#endif /* SPARESMATRIXHELP_H_ */
