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
#include "tools.h"
#include "geometry.h"
//#include <utility>

// each face is a list of vertex//normal pairs
typedef std::vector< std::pair< unsigned, unsigned> > Face;

class Mesh {
private:
	std::vector<Point> vertices;
	std::vector<Point> normals;
	std::vector<Face> faces;

	// for speeding things up
	std::vector<Triangle> facesTr;

	// optional
	boost::shared_ptr< std::set<unsigned> > selected;

	// TODO only for testing
	std::vector<std::pair<LineSegment, Triangle> > intersections;
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

	Mesh() : selectedIntersection(0) {
		selected.reset( new std::set<unsigned> );
	}
	void loadModel(char* inputfile) throw (ParseException);
	void display() const;
	void printMesh(std::ostream& out) const;
	virtual ~Mesh();
	unsigned getNumVertices() const { return vertices.size(); }
	void setSelectedVerts(boost::shared_ptr< std::set<unsigned> > const& sel) {
		selected = sel;
	}

	bool intersects(LineSegment const & l);

	// return NULL if bad index. TODO note that we should use shared_ptr instead..
	const Point * getVertex(unsigned ind) const {
		if (ind < 0 || ind >= vertices.size()) return NULL;
		return &vertices[ind];
	}
};

#endif /* MESH_H_ */
