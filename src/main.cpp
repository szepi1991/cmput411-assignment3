/*
 * main.cpp
 *
 *  Created on: 2012-10-16
 *      Author: david
 */

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


#include <iostream>
#include <boost/shared_ptr.hpp>

#include "Animation.h"
#include "ParseException.h"
#include "Camera.h"
#include "tools.h"
#include "Mesh.h"

#include "Quaternion.h"

using namespace std;


unsigned SkeletonNode::nodeCounter=0;
static boost::shared_ptr<Animation> anim;
static Camera cam;
static Mesh model;

static const unsigned SCR_WIDTH = 800, SCR_HEIGHT = 600;


void drawText(float x, float y, float z, char const *string) {
	glRasterPos3f(x, y, z);
	char const *c;
	for (c = string; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c); }
}

void setup(int argc, char **argv) throw (int) {

	if (argc != 3) {
		cerr << "ERROR: this program takes exactly 2 argument: first a wavefront .obj file, then a .bvh file to load." << endl;
		throw 1;
	}

	cout << "Reading in file now." << endl;
	try {
		model.loadModel(argv[1]);

		anim.reset(new Animation(argv[2]));
		cout << "The name of the loaded file is " << anim->getFileName() << endl;

		float xMin, xMax, yMin, yMax, zMin, zMax;
		anim->closestFit(xMin, xMax, yMin, yMax, zMin, zMax);

		float extra = 1; // TODO maybe based on figure size? (our upper bound is too big)
		cam.makeVisible(xMin-extra, xMax+extra,
				yMin-extra, yMax+extra, zMin-extra, zMax+extra);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(90, ((double)SCR_WIDTH)/((double)SCR_HEIGHT), cam.getNear(), cam.getFar());

	} catch (ParseException& e) {
		cerr << e.what() << endl;
		throw 2;
	}

	cout << "debug message level:" << endl;
	cout << "- NONE is " << debug::ison(debug::NONE) << endl;
	cout << "- LITTLE is " << debug::ison(debug::LITTLE) << endl;
	cout << "- DETAILED is " << debug::ison(debug::DETAILED) << endl;
	cout << "- EVERYTHING is " << debug::ison(debug::EVERYTHING) << endl;

}

/* OpenGL window reshape routine.
 * Don't have to do much, since the projection is set before each drawing
 */
void resize(int w, int h)
{
	// Set viewport size to be entire OpenGL window.
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, ((float) w)/((float) h), cam.getNear(), cam.getFar());
}


void drawAxes(float length) {
	// draw the 3 axis (x, y, z) <--> (red, green, blue)
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(length, 0, 0);
	glEnd();
	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0, length, 0);
	glEnd();
	glColor3f(0.0, 0.0, 1.0);
	glBegin(GL_LINES);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, length);
	glEnd();
	glColor3f(0.0, 0.0, 0.0);
}

// Drawing (display) routine.
void drawScene(void)
{
	// Clear screen to background color.
	glClear(GL_COLOR_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	std::stringstream fpsStream;
	fpsStream << "virtual fps: " << anim->getVirtualFPS();
	// Set text color.
//	glColor3f(0.0, 0.0, 0.0);
	glColor3f(1.0, 1.0, 1.0);
	drawText(-37, 27, -30, fpsStream.str().c_str());

	cam.view();

	if (MYINFO) drawAxes(10);

	// Set stickman color.
	glColor3f(1.0,1.0,0.1);
	anim->display();
	// wireframe
	glColor3f(1.0, 1.0, 1.0);
	model.display();

	glutSwapBuffers();
}


void keyInput(unsigned char key, int x, int y) {
	switch (key) {
	case 'q':
		exit(0);
		break;
	case 'p':
		anim->startAnim();
		break;
	case 'P':
		anim->stopAnim();
		break;
	case 's':
		anim->reset();
		break;
	case '+':
		anim->addFPS(10);
		break;
	case '-':
		anim->addFPS(-10);
		break;
	case 'w':
	{
		ofstream outfile1("motionout.bvh");
		anim->outputBVH(outfile1);
		outfile1.close();
		ofstream outfile2("meshout.obj");
		model.printMesh(outfile2);
		outfile2.close();
		cout << "Finished writing output files." << endl;
		break;
	}
	default:
		cam.control(key);
		break;
	}
}

// Special keyboard input processing routine.
void specialKeyInput(int key, int x, int y) {
	cam.controlSpec(key);
}

//void animate(int arg) {
//
//	glutPostRedisplay();
//	glutTimerFunc((unsigned) (anim->getStdFrameTime()*SECtoMSEC), animate, 0);
//}

void doWhenIdle() {
	glutPostRedisplay();
}


void testCode() {


	vector<unsigned> vec;
	for (unsigned i = 1; i < 10; i++) {
		vec.push_back(i);
	}

	for (vector<unsigned>::iterator it = vec.begin(); it != vec.end(); ++it) {
		cout << *it;
	}
	cout << endl;

	for (vector<unsigned>::iterator it = vec.begin(); it != vec.end(); ) {
//		cout << "v " << *(it++) << " " << *(it++)  << " " << *(it++) << std::endl;
		cout << "v " << *(it++);
		cout << " " << *(it++);
		cout << " " << *(it++) << std::endl;
	}

//	float a, b, c;
//	a = b = c = 0.3;
//	cout << a << " " << b << " " << c << " -> ";
//	normalize(&a, &b, &c);
//	cout << a << " " << b << " " << c << endl;
//
//	a = 0.3; b = 0.1; c = -1;
//	cout << a << " " << b << " " << c << " -> ";
//	normalize(&a, &b, &c);
//	cout << a << " " << b << " " << c << endl;

//	Quaternion q1(PI/2, 1, 0, 0);
//	q1.printRotMatrix();

//	Matrix4x4<float> identity;
//	float vecf[] = { 1, 0, 2, -1};
//	cout << "identity" << endl;
//	identity.printMatrix();
//	cout << "vector" << endl;
//	identity.printVector(vecf);
//	identity.multVector(vecf);
//	cout << "multiplied" << endl;
//	identity.printVector(vecf);
//
//	double data[] = { 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 6, 13, 14, 15, 16};
//	double vecd[] = { 1, 0, 2, -1};
//	Matrix4x4<double> matrix(data);
//	cout << "matrix" << endl;
//	matrix.printMatrix();
//	cout << "vector" << endl;
//	matrix.printVector(vecd);
//	cout << "multiplied" << endl;
//	matrix.multVector(vecd);
//	matrix.printVector(vecd);
}

int main(int argc, char **argv) {

//	testCode();
//	return 0;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("box.cpp");

	// Set background (or clearing) color.
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// Initialize.
	try {
		setup(argc, argv);
	} catch (int e) {
		cerr << "The program is going to terminate now." << endl;
		return 1;
	}

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyInput);
	glutSpecialFunc(specialKeyInput);
	glutIdleFunc(doWhenIdle); // we'll prob have to use this now
//	glutTimerFunc((unsigned) anim->getStdFrameTime()*1000000, animate, 0);
	glutMainLoop();


	cout << "Exit!" << endl;
	return 0;
}



