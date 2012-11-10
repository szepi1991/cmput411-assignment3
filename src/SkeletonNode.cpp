/*
 * SkeletonNode.cpp
 *
 *  Created on: 2012-10-16
 *      Author: david
 */


#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include "SkeletonNode.h"
#include "tools.h"

#include <cmath>

/* Recursively creates the skeleton based on the info in descr.
 * Reads out just as much info from the stream as needed.
 *
 * descr should contain info in the format
 * BoneName
 * {
 *     OFFSET f f f
 *     CHANNELS ----
 *     JOINT <info about inner node>
 * }
 * Or sometimes:
 * BoneName
 * {
 *     OFFSET f f f
 *     CHANNELS ----
 *     End Site
 *     {
 *         OFFSET f f f
 *     }
 * }
 */
SkeletonNode::SkeletonNode(std::ifstream& descr) throw(ParseException) {
	myCounter = nodeCounter++;
	std::string token;
	descr >> token;
	if (token.compare("{") != 0) {
		name = token;
		descr >> token;
		confirmParse(token, "{");
	} else {
		name = "no-name";
	}

	if (MYINFO)
		std::cout << "Parsing joint " << getDescr() << std::endl;

	descr >> token;
	confirmParse(token, "OFFSET");
	descr >> offset[0] >> offset[1] >> offset[2];
	descr >> token;
	confirmParse(token, "CHANNELS");

	descr >> channelNum;
	// we may assume that this line is either
	// CHANNELS 6 Xposition Yposition Zposition Zrotation Yrotation Xrotation
	// CHANNELS 3 Zrotation Yrotation Xrotation
	if (channelNum == 6) {
		descr >> token; confirmParse("Xposition", token);
		descr >> token; confirmParse("Yposition", token);
		descr >> token; confirmParse("Zposition", token);
		descr >> token; confirmParse("Zrotation", token);
		descr >> token; confirmParse("Yrotation", token);
		descr >> token; confirmParse("Xrotation", token);
	} else if (channelNum == 3) {
		descr >> token; confirmParse("Zrotation", token);
		descr >> token; confirmParse("Yrotation", token);
		descr >> token; confirmParse("Xrotation", token);
	} else {
		std::stringstream ss;
		ss << "CHANNELS " << channelNum;
		throw ParseException("CHANNELS [3|6]", ss.str());
	}

	descr >> token;
	while (token.compare("}") != 0) {
		if (token.compare("End") == 0) {
			descr >> token;
			confirmParse(token, "Site");
			descr >> token;
			confirmParse(token, "{");
			descr >> token;
			confirmParse(token, "OFFSET");
			boost::array<float, 3> offs;
			descr >> offs[0] >> offs[1] >> offs[2];
			children.push_back(SkeletonNode(offs));
			descr >> token;
			confirmParse(token, "}");
		} else if (token.compare("JOINT") == 0) {
			children.push_back(SkeletonNode(descr));
		} else {
			assert(false); // should be impossible
		}
		descr >> token;
	}

}

/* Use this constructor for leaf nodes.
 */
SkeletonNode::SkeletonNode(boost::array<float, 3> const & offsets) {
	myCounter = nodeCounter++;
	name = "leaf";
	if (MYINFO)
		std::cout << "Created " << getDescr() << std::endl;
	offset = offsets;
	channelNum = 0;
}

std::string SkeletonNode::getDescr() const {
	std::stringstream ss;
	ss << name << "-" << myCounter;
	return ss.str();
}

SkeletonNode::~SkeletonNode() {
	if (MYINFO)
		std::cout << getDescr() << " is now dying." << std::endl;
}

/* If this is a leaf then it has no endpoint .. we throw 0 */
boost::array<float, 3> SkeletonNode::getEndPoint() const throw(int) {
	if (children.size() == 0) throw 0;
	else return children[0].offset;
}

/* Extracts the animation info for the next frame from the stream. */
void SkeletonNode::addAnimationFrame(std::ifstream& descr) {
	// if this is a leaf just return
	if (children.size() == 0) return;

	// extract info for this node
	double xPos, yPos, zPos;
	if (channelNum == 6) {
		descr >> xPos >> yPos >> zPos;
	}
	double xRot, yRot, zRot;
	descr >> zRot >> yRot >> xRot;
	if (MYINFO) std::cout << "Transformation matrix " << getDescr() << ", frame " << motion.size() << std::endl;
	if (channelNum == 6) {
		motion.push_back(MotionFrame(zRot, yRot, xRot, xPos, yPos, zPos));
	} else if (channelNum == 3) {
		motion.push_back(MotionFrame(zRot, yRot, xRot));
	} else {
		std::cerr << "channelNum of " << getDescr() << " is " << channelNum << std::endl;
		assert (false);
	}

	// now extract info for all the children
	for (std::vector<SkeletonNode>::iterator it = children.begin();
											it != children.end(); ++it) {
		it->addAnimationFrame(descr);
	}
}


/* Displays frame 'frame'. If the argument is -1, it displays the initial pose
 * (This is the default value). */
void SkeletonNode::display(double frame = -1) const {
	if (children.size() == 0) return;

	glPushMatrix();
	// do all the drawing here
	glTranslatef(offset[0], offset[1], offset[2]);

	if (frame >= 0) {
		double intPart, fracPart;
		fracPart = modf(frame, &intPart); // both positive since frame positive
		unsigned curFrame = (unsigned) intPart;

		// if almost integer just display frame
		if (fracPart < 0.0001 || fracPart+0.0001 > 1) {
			motion[curFrame].applyTransformation();
		} else {
			// interpolate between intPart and next (watch out far next = 0 case!!)
			unsigned nextFrame = curFrame + 1;
			if (nextFrame >= motion.size()) nextFrame = 0;
			MotionFrame toUse;
			motion[curFrame].interpolate(motion[nextFrame], fracPart, toUse);
			toUse.applyTransformation();
		}

	}

	boost::array<float, 3> endP = getEndPoint();
    glBegin(GL_LINES);
       glVertex3f(0.0f, 0.0f, 0.0f);
	   glVertex3f(endP[0], endP[1], endP[2]);
    glEnd();

    for (unsigned i = 0; i < children.size(); ++i) {
    	children[i].display(frame);
    }

	glPopMatrix();
}


// x, y, z in both cases; this is a large upper bound!!
void SkeletonNode::offsetBounds(float * mins, float * maxs) const {
	// add on my offsets
	for (unsigned i = 0; i < 3; ++i) {
		if (offset[i] < 0) {
			mins[i] += offset[i];
		} else {
			maxs[i] += offset[i];
		}
	}
	// call for children
	for (std::vector<SkeletonNode>::const_iterator it = children.begin();
											it != children.end(); ++it) {
		it->offsetBounds(mins, maxs);
	}
}

inline void printRepeated(std::ostream& out, unsigned reps, char const * pattern) {
	for (unsigned i = 0; i < reps; ++i) out << pattern;
}

/* Prints the names of the subtree rooted at this node, with level number of
 * "- " thingies in front of it.
 */
void SkeletonNode::printNames(unsigned level) const {
	printRepeated(std::cout, level, "- ");
//	for (unsigned i = 0; i < level; ++i) std::cout << "- ";
	std::cout << name << ": \t "
			<< offset[0] << ", "
			<< offset[1] << ", "
			<< offset[2] << ", " << std::endl;
	for (std::vector<SkeletonNode>::const_iterator it = children.begin();
											it != children.end(); ++it) {
		it->printNames(level+1);
	}
}

/* print the tree recursively in the BVH file format, into 'out'. 'level' shows
 * how deep in the tree this node is (we assume roots are not leaves! that wouldn't make sense)
 *
 * This function prints ROOT/JOINT name { .... } (or change this appropriately for leaf)
 */
void SkeletonNode::printTreeBVH(std::ostream& out, unsigned level) const {
	printRepeated(out, level, "\t");
	if (children.size() == 0) {
		out << "End Site" << std::endl;
		printRepeated(out, level, "\t");
		out << "{" << std::endl;
		level++;
		printRepeated(out, level, "\t");
		out << "OFFSET " << offset[0] << " " << offset[1] << " " << offset[2] << std::endl;
	} else {
		if (level == 0) {
			out << "ROOT ";
		} else {
			out << "JOINT ";
		}
		out << name << std::endl;
		printRepeated(out, level, "\t");
		out << "{" << std::endl;
		level++;
		printRepeated(out, level, "\t");
		out << "OFFSET " << offset[0] << " " << offset[1] << " " << offset[2] << std::endl;
		printRepeated(out, level, "\t");
		out << "CHANNELS " << channelNum << " ";
		if (channelNum == 6) {
			out << "Xposition Yposition Zposition ";
		}
		// and we always have the rotations
		out << "Zrotation Yrotation Xrotation" << std::endl;
		for (std::vector<SkeletonNode>::const_iterator it = children.begin();
												it != children.end(); ++it) {
			it->printTreeBVH(out, level); // level was already incremented
		}
	}
	// level was incremented
	printRepeated(out, level-1, "\t");
	out << "}" << std::endl;
}



// enlarges the axis-aligned box defined by the parameters so that each translated
// point fits into the box
void MotionFrame::closestFit(std::vector<MotionFrame> const & frames,
		float& xMin, float& xMax, float& yMin, float& yMax, float& zMin, float& zMax)  {
	for (std::vector<MotionFrame>::const_iterator it = frames.begin();
			it != frames.end(); ++it) {
		if (it->xPos < xMin) xMin = it->xPos;
		if (it->xPos > xMax) xMax = it->xPos;
		if (it->yPos < yMin) yMin = it->yPos;
		if (it->yPos > yMax) yMax = it->yPos;
		if (it->zPos < zMin) zMin = it->zPos;
		if (it->zPos > zMax) zMax = it->zPos;
	}
}


// interpolates between this frame and nextFrame. We are fracPart into the next frame
// simply put the calculated new semi-frame into ret
void MotionFrame::interpolate(MotionFrame const & nextFrame, double fracPart, MotionFrame & ret) const {
	// first calculate the position:
	double o = 1-fracPart;
	ret.xPos = fracPart * nextFrame.xPos + o * xPos;
	ret.yPos = fracPart * nextFrame.yPos + o * yPos;
	ret.zPos = fracPart * nextFrame.zPos + o * zPos;

	// now calculate the interpolated angle
	Quaternion::slerp(rotations, nextFrame.rotations, fracPart, ret.rotations);

	// now just update the modelTrans of ret:
	ret.rotations.getRotation(ret.modelTrans);
	if (channels == 6) {
		ret.modelTrans[12] = ret.xPos;
		ret.modelTrans[13] = ret.yPos;
		ret.modelTrans[14] = ret.zPos;
	}
}

// generates the transformation matrix for this frame
void MotionFrame::genMatrix() {

	// Quaternion stuff
	Quaternion    rot(degToRad(zRot), 0, 0, 1);
	rot *= Quaternion(degToRad(yRot), 0, 1, 0);
	rot *= Quaternion(degToRad(xRot), 1, 0, 0);
	rot.getRotation(modelTrans);
	if (channels == 6) {
		modelTrans[12] = xPos;
		modelTrans[13] = yPos;
		modelTrans[14] = zPos;
	}
	rotations = rot;

	if (MYINFO) print4x4Matrix(modelTrans);

}


// applies the transformation matrix corresponding to this frame
void MotionFrame::applyTransformation() const {
	glMultMatrixf(modelTrans);
}




