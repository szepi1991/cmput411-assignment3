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
#include <set> // maybe we should use unordered list..
#include "tools.h"
#include "geometry.h"

// each face is a list of vertices//normal pairs
// Note that the number specified --> 3*num in the vertices and normals vectors
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
public:
	Mesh() { selected.reset( new std::set<unsigned> ); }
	void loadModel(char* inputfile) throw (ParseException);
	void display() const;
	void printMesh(std::ostream& out) const;
	virtual ~Mesh();
	unsigned getNumVertices() const { return vertices.size(); }
	void setSelectedVerts(boost::shared_ptr< std::set<unsigned> > const& sel) {
		selected = sel;
	}

	bool intersects(LineSegment const & l) const {
		return false; // FIXME testing
		for (std::vector<Triangle>::const_iterator it = facesTr.begin();
								it != facesTr.end(); ++it) {
			if (intersectLineSegWithTriangle(l, *it)) return true;
		}
		return false;
	}

	// return NULL if bad index. TODO note that we should use shared_ptr instead..
	const Point * getVertex(unsigned ind) const {
		if (ind < 0 || ind >= vertices.size()) return NULL;
		return &vertices[ind];
	}
};

#endif /* MESH_H_ */
