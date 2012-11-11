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

	if (debug::ison(debug::DETAILED))
		std::cout << "Parsing joint " << getDescr() << std::endl;

	descr >> token;
	confirmParse(token, "OFFSET");
	float offs[3];
	descr >> offs[0] >> offs[1] >> offs[2];
	offset.reset(new Point(offs[0], offs[1], offs[2]));
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
//			boost::array<float, 3> offs;
			float offs[3];
			descr >> offs[0] >> offs[1] >> offs[2];
			boost::shared_ptr<Point> leafOff(new Point(offs[0], offs[1], offs[2]));
			children.push_back(SkeletonNode(leafOff));
			descr >> token;
			confirmParse(token, "}");
		} else if (token.compare("JOINT") == 0) {
			children.push_back(SkeletonNode(descr));
		} else {
			assert(false); // should be impossible
		}
		descr >> token;
	}

	// calculate projToBone here.
	boost::shared_ptr<Point> bone = getEndPoint();
//	Eigen::Vector3f b(bone->x(), bone->y(), bone->z());
	Eigen::Matrix3f b = Eigen::Matrix3f::Zero();
	b(0,0) = bone->x();
	b(1,0) = bone->y(),
	b(2,0) = bone->z();
	// so now it's like a column vector and 0s after it
	projToBoneM = Eigen::Matrix3f::Identity() - (b * b.transpose()) * (1/b.squaredNorm());
}

/* Use this constructor for leaf nodes.
 */
SkeletonNode::SkeletonNode(boost::shared_ptr<Point> const & offsets) {
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
	if (debug::ison(debug::EVERYTHING))
		std::cout << getDescr() << " is now dying." << std::endl;
}

/* If this is a leaf then it has no endpoint .. we throw 0 */
const boost::shared_ptr<Point> SkeletonNode::getEndPoint() const throw(int) {
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
	if (debug::ison(debug::EVERYTHING)) std::cout << "Transformation matrix " << getDescr() << ", frame " << motion.size() << std::endl;
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
 * (This is the default value).
 * selectedBone should be drawn with red.
 * */
void SkeletonNode::display(double frame, unsigned selectedbone = -1) const {
	if (children.size() == 0) return;

	glPushMatrix();
	// do all the drawing here
	glTranslatef(offset->x(), offset->y(), offset->z());

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

	float currentColor[4];
	if (selectedbone == myCounter) {
		glGetFloatv(GL_CURRENT_COLOR,currentColor);
    	glColor3f(1.0, 0, 0); // red
	}

	const boost::shared_ptr<Point> endP = getEndPoint();
    glBegin(GL_LINES);
       glVertex3f(0.0f, 0.0f, 0.0f);
	   glVertex3f(endP->get(0), endP->get(1), endP->get(2));
    glEnd();

    // reset color
    if (selectedbone == myCounter) {
    	glColor4fv(currentColor);
    }

    for (unsigned i = 0; i < children.size(); ++i) {
    	children[i].display(frame, selectedbone);
    }

	glPopMatrix();
}


// x, y, z in both cases; this is a large upper bound!!
void SkeletonNode::offsetBounds(float * mins, float * maxs) const {
	// add on my offsets
	for (unsigned i = 0; i < 3; ++i) {
		if (offset->get(i) < 0) {
			mins[i] += offset->get(i);
		} else {
			maxs[i] += offset->get(i);
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
			<< offset->get(0) << ", "
			<< offset->get(1) << ", "
			<< offset->get(2) << ", " << std::endl;
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
		out << "OFFSET " << offset->get(0) << " " << offset->get(1) << " " << offset->get(2) << std::endl;
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
		out << "OFFSET " << offset->get(0) << " " << offset->get(1) << " " << offset->get(2) << std::endl;
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

/**Returns the list of bones that are closest (within EPS distance) to the given point.
 * The coordinates of p are given in the frame of the parent of this node.
 * For root this means world coordinates.
 */
void SkeletonNode::getClosestBones(Point p, float minDist, std::vector<SkeletonNode> & closests) const {
	if (children.size() == 0) return;

	// transform point so that bone is at (0,0)
	if (debug::ison(debug::EVERYTHING))
		std::cout << name << " " << p << " -> ";
	p -= Point(offset->get(0), offset->get(1), offset->get(2));
	if (debug::ison(debug::EVERYTHING))
		std::cout << p << std::endl;
	boost::shared_ptr<Point> bone = getEndPoint();
	float pb = p.dot(*bone);
	float dist;
	if (pb <= 0) dist = p.getLength();
	else if (pb >= bone->getLengthSqr()) dist = (p - *bone).getLength();
	else {
		Eigen::Vector3f v(p.x(),p.y(),p.z());
		dist = std::sqrt(v.transpose()*projToBoneM*v);
	}

	if (std::abs(dist - minDist) < EPS) {
		closests.push_back(*this);
	} else if (dist < minDist) {
		closests.clear();
		closests.push_back(*this);
		minDist = dist;
	}

	for (std::vector<SkeletonNode>::const_iterator it = children.begin();
											it != children.end(); ++it) {
		it->getClosestBones(p, minDist, closests);
	}

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

	if (debug::ison(debug::EVERYTHING)) print4x4Matrix(modelTrans);

}


// applies the transformation matrix corresponding to this frame
void MotionFrame::applyTransformation() const {
	glMultMatrixf(modelTrans);
}




