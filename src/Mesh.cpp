/*
 * Mesh.cpp
 *
 *  Created on: 2012-11-10
 *      Author: david
 */

#include "Mesh.h"
#include <string>
#include <fstream>
#include <sstream>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


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


void Mesh::display() const {
	// TODO optimize the selected stuff! maybe boost unordered_set
	for (std::vector<Face>::const_iterator it = faces.begin(); it != faces.end(); ++it) {
		glBegin(GL_TRIANGLES);
			for (unsigned vn = 0; vn < 3; ++vn) { // each face is a triangle
				Point n = normals[(*it)[vn].second];
				Point v = vertices[(*it)[vn].first];
				if (selected->find( (*it)[vn].first ) != selected->end() ) {
					glColor3f(1.0, 0.0, 0.0); // red
				} else {
					glColor3f(1.0, 1.0, 1.0);
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
}

Mesh::~Mesh() {
}

