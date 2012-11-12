/*
 * SkeletonNode.h
 *
 *  Created on: 2012-10-16
 *      Author: david
 */

#ifndef SKELETONNODE_H_
#define SKELETONNODE_H_

#include "myexceptions.h"
#include "Quaternion.h"
#include "geometry.h"
#include "Mesh.h"

#include <string>
#include <vector>
#include <fstream>
//#include <boost/array.hpp>
#include <boost/shared_ptr.hpp>
#include <Eigen/Dense>

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
	// all these are just set up once then never should be changed!

	static unsigned nodeCounter;
	int myNodeNum;

	std::string name;
	std::vector<SkeletonNode> children;
	boost::shared_ptr<Point> offset;
	// where this node is located (with the offset!) in world coordinates.
	// that is the location of the endpoint of the bone
	Point worldOffset;
	unsigned channelNum;
	std::vector<MotionFrame> motion;

	// this corresponds to the incoming bone!
	Eigen::Matrix3f projToBoneM;

public:
	SkeletonNode(std::ifstream& descr) throw(ParseException);
	SkeletonNode(boost::shared_ptr<Point> const & offsets);
	virtual ~SkeletonNode();
	std::string getDescr() const;
	void getBoneSubTree(std::ostream& out) const {
		for (std::vector<SkeletonNode>::const_iterator it = children.begin();
												it != children.end(); ++it) {
			out << it->getUpperBoneNum() << " " << name << " " << it->name << std::endl;
			it->getBoneSubTree(out);
			// NOTE boneNum = childs myNodeNum-1
		}
	}

	void setWorldOffsetRec(Point const & parentOffset) {
		worldOffset = parentOffset + *offset;
		for (std::vector<SkeletonNode>::iterator it = children.begin();
												it != children.end(); ++it) {
			it->setWorldOffsetRec(worldOffset);
		}
	}

	// the boneNum corresponding to the node ending at this node
	int getUpperBoneNum() const { return myNodeNum-1; }

	void getBoneDescr(std::ostream& out, int boneNum) const {
		for (std::vector<SkeletonNode>::const_iterator it = children.begin();
												it != children.end(); ++it) {
			if (it->getUpperBoneNum() == boneNum) {
				out << it->getUpperBoneNum() << " " << name << " " << it->name;
				return;
			} else {
				it->getBoneDescr(out, boneNum);
			}
		}
	}

	void printNames(unsigned level) const;
	void display(double, int) const;
	void addAnimationFrame(std::ifstream& descr);
	const boost::shared_ptr<Point> getEndPoint() const throw(int);

	void printTreeBVH(std::ostream& out, unsigned level) const;
	void printFrameBVH(std::ostream& out, unsigned frame) const {
		if (children.size() == 0) return; // leafs have no animation transformations
		motion[frame].printFrame(out);
		for (std::vector<SkeletonNode>::const_iterator it = children.begin();
												it != children.end(); ++it) {
			it->printFrameBVH(out, frame);
		}
	}

//	void getClosestBones(Point p, float& minDist, std::vector<SkeletonNode> & closests) const;
	void getClosestBones(Point p, float& minDist, std::vector<SkeletonNode> & closests,
			boost::shared_ptr<Mesh> const & model) const;

	// enlarges the axis-aligned box defined by the parameters so that each translated
	// point fits into the box
	void closestAnimationFit(float& xMin, float& xMax,
			float& yMin, float& yMax, float& zMin, float& zMax) const {
		// we assume only roots have translations!
		MotionFrame::closestFit(motion, xMin, xMax, yMin, yMax, zMin, zMax);
	}
	void offsetBounds(float * mins, float * maxs) const;

	// node this only works as expected if we never delete a node!!
	unsigned static getNumberOfNodes() {return nodeCounter;}
};

#endif /* SKELETONNODE_H_ */
