/*
 * SkeletonNode.h
 *
 *  Created on: 2012-10-16
 *      Author: david
 */

#ifndef SKELETONNODE_H_
#define SKELETONNODE_H_

#include "ParseException.h"
#include "Quaternion.h"

#include <string>
#include <vector>
#include <fstream>
#include <boost/array.hpp>

class MotionFrame {
	// these can't be const since they are put in a vector where the elements
	// need to be assignable, .. but they really should not be changed!
private:
	unsigned channels;
	float zRot, yRot, xRot;
	float xPos, yPos, zPos;
	float modelTrans[16]; // all transformations of model -- in matrix format
	Quaternion rotations;


public:

	// in general don't use this one.. leaves the frame in undefined state
	MotionFrame() {};

	MotionFrame(double zRot, double yRot, double xRot,
			double xPos, double yPos, double zPos) :
				zRot(zRot), yRot(yRot), xRot(xRot),
				xPos(xPos), yPos(yPos), zPos(zPos) {
		channels = 6;
		genMatrix();
	}
	MotionFrame(double zRot, double yRot, double xRot) :
				zRot(zRot), yRot(yRot), xRot(xRot) {
		xPos = 0; yPos = 0; zPos = 0;
		channels = 3;
		genMatrix();
	}

	void genMatrix();
	void applyTransformation() const;
	void printFrame(std::ostream& out) const {
		// using assumption here
		if (channels == 6) {
			out << xPos << " " << yPos << " " << zPos << " ";
		}
		out << zRot << " " << yRot << " " << xRot << " ";
	}
	static void closestFit(std::vector<MotionFrame> const & frames,
			float& xMin, float& xMax, float& yMin, float& yMax, float& zMin, float& zMax);

	void interpolate(MotionFrame const & nextFrame, double fracPart, MotionFrame & ret) const;
};




class SkeletonNode {

private:
	static unsigned nodeCounter;
	unsigned myCounter;

	std::string name;
	std::vector<SkeletonNode> children;
	boost::array<float, 3> offset;
	unsigned channelNum;
	std::vector<MotionFrame> motion;

public:
	SkeletonNode(std::ifstream& descr) throw(ParseException);
	SkeletonNode(boost::array<float, 3> const &);
	virtual ~SkeletonNode();
	std::string getDescr() const;

	void printNames(unsigned level) const;
	void display(double) const;
	void addAnimationFrame(std::ifstream& descr);
	boost::array<float, 3> getEndPoint() const throw(int);

	void printTreeBVH(std::ostream& out, unsigned level) const;
	void printFrameBVH(std::ostream& out, unsigned frame) const {
		if (children.size() == 0) return; // leafs have no animation transformations
		motion[frame].printFrame(out);
		for (std::vector<SkeletonNode>::const_iterator it = children.begin();
												it != children.end(); ++it) {
			it->printFrameBVH(out, frame);
		}
	}

	// enlarges the axis-aligned box defined by the parameters so that each translated
	// point fits into the box
	void closestAnimationFit(float& xMin, float& xMax,
			float& yMin, float& yMax, float& zMin, float& zMax) const {
		// we assume only roots have translations!
		MotionFrame::closestFit(motion, xMin, xMax, yMin, yMax, zMin, zMax);
	}
	void offsetBounds(float * mins, float * maxs) const;
};

#endif /* SKELETONNODE_H_ */
