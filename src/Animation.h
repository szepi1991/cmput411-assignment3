/*
 * Animation.h
 *
 *  Created on: 2012-10-16
 *      Author: david
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <Eigen/Sparse>

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


#include "SkeletonNode.h"
#include "Mesh.h"
#include "myexceptions.h"

class Animation {
private:
	static const float WIDTH = 5;

	std::string filename;
	std::vector<SkeletonNode> roots;

	// these next 2 should NOT change!
	unsigned frameNum;
	double stdFrameTime; // in seconds

	double curFrameFrac; // which frame are we at exactly

	bool animating;
	double stdFPS;
	double virtFPS;

	long timeOfPreviousCall;

	float figureSize;

	// --------
	int selectedBone;

	// matricies for skin-bone attachment
	boost::shared_ptr<Mesh> model;
	Eigen::SparseMatrix<double> simpleConMat;

public:
	Animation(char *filename) throw(ParseException);
	virtual ~Animation();

	std::string getFileName() {return filename;}
	double getStdFrameTime() {return stdFrameTime;}
	float getVirtualFPS() { return virtFPS; }
	void display(bool showSelBone = false);
	void addToTime(double timediff);

	void startAnim() {
		animating = true;
		if (curFrameFrac == -1) { // we mean to compare unsigned and signed here
			// if we are in the inital state get out of it artificially first!
			curFrameFrac = 0;
		}
		timeOfPreviousCall = glutGet(GLUT_ELAPSED_TIME);
//		lastTime = boost::posix_time::microsec_clock::universal_time();
	}
	void stopAnim() {animating = false;}
	void reset();
	void addFPS(double diff) {virtFPS += diff;}

	void outputBVH(std::ostream&);
	void closestFit(float&, float&, float&, float&, float&, float&);
	float getFigureSizeBox();

	void printBoneStruct(std::ostream& out) const { roots[0].getBoneSubTree(out); }
	void getBoneDescr(std::ostream& out, int boneNum) const {
		roots[0].getBoneDescr(out, boneNum);
	}

	void setModel(boost::shared_ptr<Mesh> const & m) {
		model = m;
		std::cout << "Attaching model to skeleton.." << std::endl;
		simpleAttachBones();
		std::cout << "* simple attachment calculated" << std::endl;
	}
	void printSimpleAttachedMatrix(std::ostream& out) const throw(WrongStateException);
	void updateMeshSelected();

	void selectNextBone() {
		selectedBone++;
		if (selectedBone+1 >= (int) SkeletonNode::getNumberOfNodes()) selectedBone = 0;
		printSelectedBone();
		updateMeshSelected();
	}
	void selectPrevBone() {
		selectedBone--;
		if (selectedBone < 0) selectedBone = SkeletonNode::getNumberOfNodes()-2; // -1?
		printSelectedBone();
		updateMeshSelected();
	}
	void printSelectedBone() {
		if (debug::ison(debug::LITTLE)) {
			std::cout << "Selected bone: ";
			getBoneDescr(std::cout, selectedBone);
			std::cout << std::endl;
		}
	}
private:
	void simpleAttachBones();

};

#endif /* ANIMATION_H_ */
