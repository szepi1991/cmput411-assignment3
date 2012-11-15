/*
 * Mesh.h
 *
 *  Created on: 2012-11-10
 *      Author: david
 */

#ifndef MESH_H_
#define MESH_H_

#include <vector>
#include <boost/shared_ptr.hpp>
#include <set> // TODO maybe we should use unordered list..
#include <Eigen/Sparse>

#include "tools.h"
#include "geometry.h"

// each face is a list of vertex//normal pairs
typedef std::vector< std::pair< unsigned, unsigned> > Face;

class Mesh {
private:
	float lightPos[4];

	std::vector< std::vector<Point> > verticesList; // as read from the file
	std::vector< std::vector<Point> > normalsList; // as read from the file
	std::vector<Face> faces;

	bool wireFrame;

	// for speeding things up
	std::vector<Triangle> facesTr; // the original ones!

	Eigen::SparseMatrix<double> adjacencyMatrix;
	Eigen::SparseMatrix<double> laplacian;

	// optional
	boost::shared_ptr< std::set<unsigned> > selected;

	// TODO only for testing
	std::vector<std::pair<LineSegment, Triangle> > intersections;

	void findLaplacian();
public:
	unsigned selectedIntersection; // == intersections.size() means none
	void nextIntersection(bool message = true) {
		selectedIntersection++;
		if (selectedIntersection > intersections.size()) selectedIntersection -= intersections.size();
		if (!message) return;
		if (selectedIntersection != intersections.size()) {
			std::cout << "Intersection: "
					<< intersections[selectedIntersection].second << " with "
					<< intersections[selectedIntersection].first << std::endl;
		} else {
			std::cout << "No intersection selected" << std::endl;
		}
	}

	Mesh() : wireFrame(true), selectedIntersection(0) {
		lightPos[0] = 0.0;
		lightPos[1] = 10.5;
		lightPos[2] = 13.0;
		lightPos[3] = 1.0;

		selected.reset( new std::set<unsigned> );
	}
	void loadModel(char* inputfile) throw (ParseException);
	void display(int = -1) const;
	void printOrigMesh(std::ostream& out) const;
	void printAdjMatrix(std::ostream& out) const;
	void printLaplacian(std::ostream& out) const;
	Eigen::SparseMatrix<double> const& getLaplacian() const { return laplacian; }
	virtual ~Mesh();
	unsigned getNumVertices() const { return verticesList[0].size(); }
	void setSelectedVerts(boost::shared_ptr< std::set<unsigned> > const& sel) {
		selected = sel;
	}

	bool intersects(LineSegment const & l);

	// return NULL if bad index. TODO note that we should use shared_ptr instead..
	const Point * getOrigVertex(unsigned ind) const {
		if (ind < 0 || ind >= verticesList[0].size()) return NULL;
		return &verticesList[0][ind];
	}

	std::vector<Point> const& getOrigVertices() { return verticesList[0]; }
	std::vector<Point> const& getOrigNormals() { return normalsList[0]; }
	void addFrame(std::vector<Point> const& verts, std::vector<Point> const& normals) {
		verticesList.push_back(verts);
		normalsList.push_back(normals);
		if (debug::ison(debug::EVERYTHING)) {
			std::cout << "Now " << verticesList.size() << "," << normalsList.size() << " frames in the Mesh." << std::endl;
		}
	}

	void setWireFrame(bool val) { wireFrame = val; }
};

#endif /* MESH_H_ */
