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

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif


#include "SkeletonNode.h"
#include "Mesh.h"
#include "ParseException.h"

class Animation {
private:
	std::string filename;
	std::vector<SkeletonNode> roots;

	// these next 2 should NOT change!
	unsigned frameNum;
	double stdFrameTime; // in seconds

	double curFrameFrac; // which frame are we at exactly

	bool animating;
	double stdFPS;
	double virtFPS;

	static const float WIDTH = 5;

	long timeOfPreviousCall;
//	boost::posix_time::ptime lastTime;

	float figureSize;

public:
	Animation(char *filename) throw(ParseException);
	virtual ~Animation();

	std::string getFileName() {return filename;}
	double getStdFrameTime() {return stdFrameTime;}
	float getVirtualFPS() { return virtFPS; }
	void display();
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

	void attachBones(Mesh const& model) const;
};

#endif /* ANIMATION_H_ */
