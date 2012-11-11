/*
 * Mesh.h
 *
 *  Created on: 2012-11-10
 *      Author: david
 */

#ifndef MESH_H_
#define MESH_H_

#include <vector>
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
public:
	Mesh() {};
	void loadModel(char* inputfile) throw (ParseException);
	void display() const;
	void printMesh(std::ostream& out) const;
	virtual ~Mesh();
};

#endif /* MESH_H_ */
