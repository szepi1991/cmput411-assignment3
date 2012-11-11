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
static bool wireFrame = true;

static float lightPos[] = { 0.0, 10.5, 13.0, 1.0 };

static const unsigned SCR_WIDTH = 800, SCR_HEIGHT = 600;


void drawText(float x, float y, float z, char const *string) {
	glRasterPos3f(x, y, z);
	char const *c;
	for (c = string; *c != '\0'; c++) { glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c); }
}

void loadThings(int argc, char **argv) throw (int) {

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

void graphicsSetup() {
	// Set background (or clearing) color.
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST); // Enable depth testing.

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, ((double)SCR_WIDTH)/((double)SCR_HEIGHT), cam.getNear(), cam.getFar());

	// Light property vectors.
//	float globAmb[] = { 0.2, 0.2, 0.2, 1.0 };
//	float globAmb[] = { 0.9, 0.9, 0.9, 1.0 };
//	float lightAmb[] = { 0.0, 0.0, 0.0, 1.0 };
	float lightAmb[] = { 0.5, 0.5, 0.5, 1.0 };
//	float lightDifAndSpec[] = { 1.0, 1.0, 1.0, 1.0 };

	// Light properties.
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
//	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);
//	glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec);

	glEnable(GL_LIGHT0); // Enable particular light source.
//	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb); // Global ambient light.
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE); // Enable two-sided lighting.
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE); // Enable local viewpoint.

//	glShadeModel(GL_FLAT);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);

//	glCullFace(GL_BACK);
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (wireFrame)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

	// mesh
	glColor3f(1.0, 1.0, 1.0);
	if (!wireFrame) {
		glEnable(GL_LIGHTING);
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, skinCol);
	}
	model.display();
	glDisable(GL_LIGHTING);

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
	case 'l':
		wireFrame = true;
		glDisable(GL_CULL_FACE);
//		glDisable(GL_LIGHTING);
//		glDisable(GL_COLOR_MATERIAL);
		break;
	case 'L':
		wireFrame = false;
		glEnable(GL_CULL_FACE);
//		glEnable(GL_LIGHTING);
//		glEnable(GL_COLOR_MATERIAL);
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("box.cpp");

	// Initialize.
	try {
		loadThings(argc, argv);
	} catch (int e) {
		cerr << "The program is going to terminate now." << endl;
		return 1;
	}
	graphicsSetup();

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



