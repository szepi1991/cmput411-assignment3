/*
 * Mesh.cpp
 *
 *  Created on: 2012-11-10
 *      Author: david
 */

#include "Mesh.h"
#include "sparseMatrixHelp.h"
#include <string>
#include <fstream>
#include <sstream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif
#include <Eigen/Dense>


void Mesh::loadModel(char* modelFile) throw (ParseException) {
	// each vertex is consecutive 3 coord
	std::vector<float> vertexCoords; // translated and scaled

	if (debug::ison(debug::LITTLE)) std::cout << "Loading " << modelFile << std::endl;
	std::string line, lineType;
	unsigned int vertex, normal;
	float x, y, z;
	std::ifstream myfile(modelFile);
	if (myfile.is_open()) {
		while (myfile.good()) {
			getline(myfile, line);

			if (line.size() == 0)
				break;

			// parse each line
			std::stringstream ss(line);
			ss >> lineType;
			if (lineType.compare("v") == 0) {
				ss >> x >> y >> z;
				if (debug::ison(debug::EVERYTHING))
					std::cout << "Created vertex " << x << " " << y << " " << z << std::endl;
				vertices.push_back(Point(x, y, z));
//				vertices.push_back(x);
//				vertices.push_back(y);
//				vertices.push_back(z);
			} else if (lineType.compare("vn") == 0) {
				ss >> x >> y >> z;
				if (debug::ison(debug::EVERYTHING))
					std::cout << "Created normal vector " << x << " " << y << " " << z << std::endl;
				normalize(&x, &y, &z);
				normals.push_back(Point(x, y, z));
//				normals.push_back(x);
//				normals.push_back(y);
//				normals.push_back(z);
			} else if (lineType.compare("f") == 0) {

				Face oneFace;
				std::string term;
				if (debug::ison(debug::EVERYTHING))
					std::cout << "face ";
//				std::cout << ss.str();
				while (ss >> term) {
					// this should happen exactly 3 times here..
					// now vertex looks like verNum//norNum
					std::istringstream termStr(term);
					std::string v, t, n;
					std::getline(termStr, v, '/');
					std::getline(termStr, t, '/');
					std::getline(termStr, n, ' ');
//					std::cout << "v=" << v << ", t=" << t << ", n=" << n << " ---->" ;
					std::istringstream verStr(v), norStr(n);
					verStr >> vertex;
					norStr >> normal;
					vertex--; normal--; // uses 1 based indices..
					if (debug::ison(debug::EVERYTHING))
						std::cout << "v" << vertex << " n" << normal << ", ";
//					if (vertex*3 > vertices.size() || normal*3 > normals.size()) {
					if (vertex > vertices.size() || normal > normals.size()) {
						std::stringstream ss;
						ss		<< std::endl
								<< "__ The face '" << std::endl << "\t" << line
								<< "' is referencing vertex " << vertex
								<< "and normal " << normal
								<< ", one of which has not been defined yet. (normals="
								<< normals.size() << ", vertices="
								<< vertices.size() << ") __"
								<< std::endl;
						throw ParseException("", ss.str());
					}
					oneFace.push_back(std::make_pair(vertex, normal));
				}
				faces.push_back(oneFace);
				if (debug::ison(debug::EVERYTHING))
					std::cout << std::endl;

			} else {
				if (debug::ison(debug::EVERYTHING))
					std::cout << "Ignoring line: " << line << std::endl;
			}
		}
		myfile.close();
	} else {
		throw ParseException("", "__ Unable to open the specified .obj file __");
	}

	typedef Eigen::Triplet<double> Tr;
	std::vector<Tr> adjTripletList;
	adjTripletList.reserve(getNumVertices()*4);

	for (std::vector<Face>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
		unsigned vNums[3];
		vNums[0] = (*it)[0].first;
		vNums[1] = (*it)[1].first;
		vNums[2] = (*it)[2].first;
		// create the triangles corresponding to the faces;
		facesTr.push_back(Triangle(	vertices[vNums[0]],
									vertices[vNums[1]],
									vertices[vNums[2]]));

		// create  adjacency matrix; for each pair of i,j adj(vNums[i], vNums[j]) = 1;
		for (unsigned i = 0; i < 3; ++i) {
			for (unsigned j = 0; j < 3; ++j) {
				if (i == j) continue;
				adjTripletList.push_back(Tr(vNums[i], vNums[j], 1));
			}
		}
	}

	adjacencyMatrix.resize(getNumVertices(), getNumVertices());
	adjacencyMatrix.reserve(adjTripletList.size()*1.5);
	adjacencyMatrix.setFromTriplets(adjTripletList.begin(), adjTripletList.end());

	findLaplacian();
}

void Mesh::findLaplacian() {
	Eigen::VectorXd ones = Eigen::VectorXd::Ones(getNumVertices());
	Eigen::VectorXd res = adjacencyMatrix*ones;

//	// TODO slow way -- see online tutorial on sparse matrices "Iterating over the nonzero coefficients"
//	std::ofstream outres("A1.out");
//	for (int i = 0; i < res.rows(); ++i) {
//		outres << " " << res(i);
//		outres << std::endl;
//	}
//	outres.close();

//	typedef Eigen::Triplet<double> Tr;
//	std::vector<Tr> deltaTriplets;
//	deltaTriplets.reserve(getNumVertices());
//	for (unsigned i = 0; i < getNumVertices(); ++i) {
//		deltaTriplets.push_back(Tr(i, i, res(i)));
//	}
//
//	Eigen::SparseMatrix<double> delta(getNumVertices(), getNumVertices());
//	delta.reserve(getNumVertices());
//	delta.setFromTriplets(deltaTriplets.begin(), deltaTriplets.end());

	Eigen::SparseMatrix<double> deltaM = delta(res);

//	// TODO slow way -- see online tutorial on sparse matrices "Iterating over the nonzero coefficients"
//	std::ofstream outdelta("delta.out");
//	for (int i = 0; i < delta.rows(); ++i) {
//		outdelta << i;
//		for (int j = 0; j < delta.cols(); ++j) {
//			if (delta.coeff(i,j) != 0) outdelta << " " << j << ":" << delta.coeff(i,j);
//		}
//		outdelta << std::endl;
//	}
//	outdelta.close();

	laplacian = deltaM - adjacencyMatrix;
}


void Mesh::printAdjMatrix(std::ostream& out) const {
	// TODO slow way -- see online tutorial on sparse matrices "Iterating over the nonzero coefficients"
	for (int i = 0; i < adjacencyMatrix.rows(); ++i) {
		out << i;
		for (int j = 0; j < adjacencyMatrix.cols(); ++j) {
			if (abs(adjacencyMatrix.coeff(i,j)) > EPS) out << " " << j;
		}
		out << std::endl;
	}
}

void Mesh::printLaplacian(std::ostream& out) const {
	// TODO slow way -- see online tutorial on sparse matrices "Iterating over the nonzero coefficients"
	for (int i = 0; i < laplacian.rows(); ++i) {
		out << i;
		for (int j = 0; j < laplacian.cols(); ++j) {
			if (abs(laplacian.coeff(i,j)) > EPS) out << " " << j << " " << laplacian.coeff(i,j);
		}
		out << std::endl;
	}
}

void Mesh::printMesh(std::ostream& out) const {
	out << std::fixed;
	out.precision(6);
	for (std::vector<Point>::const_iterator it = vertices.begin(); it != vertices.end(); ++it) {
		out << "v " << it->x() << " " << it->y() << " " << it->z() << std::endl;
	}
	for (std::vector<Point>::const_iterator it = normals.begin(); it != normals.end(); ++it) {
		out << "v " << it->x() << " " << it->y() << " " << it->z() << std::endl;
	}

	for (std::vector<Face>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
		out << "f";
		// std::vector< std::pair<unsigned, unsigned> >
		for (unsigned i = 0; i < it->size(); ++i) {
			out << " " << (*it)[i].first + 1 << "//" << (*it)[i].second + 1;
			// need to add those 1s because it's 1 indexed!
		}
		out << std::endl;
	}
}

bool Mesh::intersects(LineSegment const & l) {
	for (std::vector<Triangle>::const_iterator it = facesTr.begin();
							it != facesTr.end(); ++it) {
		if (intersectLineSegWithTriangle(l, *it)) {
			intersections.push_back(std::make_pair(l, *it));
			nextIntersection(false);
			return true;
		}
	}
	return false;
}


void Mesh::display() const {
	// TODO optimize the selected stuff! maybe boost unordered_set
	for (std::vector<Face>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
		glBegin(GL_TRIANGLES);
			for (unsigned vn = 0; vn < 3; ++vn) { // each face is a triangle
				Point n = normals[(*it)[vn].second];
				Point v = vertices[(*it)[vn].first];
				if (selected->find( (*it)[vn].first ) != selected->end() ) {
					glColor4f(1.0, 0.0, 0.0, 0.5); // red
				} else {
					glColor4f(1.0, 1.0, 1.0, 0.5);
				}
				glNormal3f(n.x(), n.y(), n.z());
				glVertex3f(v.x(), v.y(), v.z());
			}
		glEnd();
		if (debug::ison(debug::EVERYTHING)) {
			// now also draw the normals..
			glColor3f(0.0, 0.0, 1.0);
			for (unsigned vn = 0; vn < 3; ++vn) { // each face is a triangle
				Point n = normals[(*it)[vn].second];
				Point v = vertices[(*it)[vn].first];
				glBegin(GL_LINES);
					glVertex3f(v.x(), v.y(), v.z());
					glVertex3f(v.x()+n.x(), v.y()+n.y(), v.z()+n.z());
				glEnd();
			}
		}
	}

	if (selectedIntersection < intersections.size()) {
		GLint polyMode;
		glGetIntegerv(GL_POLYGON_MODE, &polyMode);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		Triangle t = intersections[selectedIntersection].second;
		LineSegment l = intersections[selectedIntersection].first;
		glColor3f(0.0, 0.0, 1.0);
		glBegin(GL_TRIANGLES);
			for (unsigned i = 0; i < 3; ++i) {
				const Point e = t.getPoint(i);
				glVertex3f(e.x(), e.y(), e.z());
			}
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glLineWidth(3);
		glColor3f(1.0, 0.0, 0.0);
		const Point e1 = l.getTrans();
		const Point e2 = e1+l.getShift();
		glBegin(GL_LINES);
			glVertex3f(e1.x(), e1.y(), e1.z());
			glVertex3f(e2.x(), e2.y(), e2.z());
		glEnd();
		glLineWidth(1);
		glPolygonMode(GL_FRONT_AND_BACK, polyMode);
	}
}

Mesh::~Mesh() {
}

