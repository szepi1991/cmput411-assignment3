/*
 * Animation.cpp
 * Reads a Biovision .bvh file describing a simplified figure skeleton
 * represented by a hierarchy of line segments (limbs) connected by joints.
 * Allows for the playing of this animation
 *
 *  Created on: 2012-10-16
 *      Author: david
 */

#include "Animation.h"
#include "SkeletonNode.h"
#include "tools.h"
#include "sparseMatrixHelp.h"

#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>

Animation::Animation(char *filename) throw(ParseException) : figureSize(0), selectedBone(0) {

	std::ifstream infile(filename);
	// read stuff in
	std::string word;
	while (infile >> word && word.compare("HIERARCHY") != 0) {
	}
	// now description starts
	infile >> word;
	while (word.compare("ROOT") == 0) {
		roots.push_back(SkeletonNode(infile));
		std::cout << "The tree structure we read in is:" << std::endl;
		(roots.end()-1)->printNames(0);
		infile >> word;
	}

	if (word.compare("MOTION") != 0)
		throw ParseException("MOTION", word);

	std::cout << "Loading the animation description." << std::endl;

	// parse the motion stuff
	infile >> word;
	confirmParse("Frames:", word);
	infile >> frameNum;
	infile >> word;
	confirmParse("Frame", word);
	infile >> word;
	confirmParse("Time:", word);
	infile >> stdFrameTime;

	// assume that the animation for the 2 roots are interleaved
	for (unsigned f = 0; f < frameNum; ++f) {
		for (std::vector<SkeletonNode>::iterator rootIt = roots.begin();
											rootIt != roots.end(); ++rootIt) {
			rootIt->addAnimationFrame(infile);
		}
	}

	// there should be nothing more in the file
	while (!infile.eof()) {
		std::getline(infile, word);
		if (word.begin() != remove_if(word.begin(), word.end(), isspace)) {
			std::cerr << "Unexpected term '" << word << "' at the end of the bvh file" << std::endl;
		}
	}
	infile.close();

	std::cout << "Finished." << std::endl;
	this->filename = filename;
	animating = false;

	stdFPS = floor((1.0 / stdFrameTime) + 0.5);
	virtFPS = stdFPS;

	curFrameFrac = -1;
}

Animation::~Animation() {
	// because of shared_ptr the node that root is pointed to gets deleted.
}

// calculates the axis-aligned (roughly) smallest box that will fit the animation
void Animation::closestFit(float& xMin, float& xMax,
					float& yMin, float& yMax, float& zMin, float& zMax) {
	// set all the same
//	xMin = yMin = zMin = std::numeric_limits<float>::max();
//	xMax = yMax = zMax = - std::numeric_limits<float>::max();
	// should encompass the initial figure
	float figBoxSize = getFigureSizeBox();
	xMin = yMin = zMin = -figBoxSize;
	xMax = yMax = zMax = figBoxSize;

	// initialize the vars
	for (unsigned f = 0; f < frameNum; ++f) {
		for (std::vector<SkeletonNode>::iterator rootIt = roots.begin();
											rootIt != roots.end(); ++rootIt) {
			rootIt->closestAnimationFit(xMin, xMax, yMin, yMax, zMin, zMax);
		}
	}
}

float Animation::getFigureSizeBox() {
	if (figureSize == 0) { // no need to recalculate
		// initial value
		float mins [] = {0, 0, 0}; // we assume the origin is always in the picture
		float maxs [] = {0, 0, 0}; // we assume the origin is always in the picture
		for (std::vector<SkeletonNode>::iterator rootIt = roots.begin();
											rootIt != roots.end(); ++rootIt) {
			rootIt->offsetBounds(mins, maxs);
		}
		float diffs[3];
		for (unsigned i = 0; i < 3; ++i) diffs[i] = (maxs[i] - mins[i]);
		figureSize = (*std::max_element(diffs, diffs+3)) / 2;
		figureSize -= 2; // technically we shouldn't do this.. but
	}
	return figureSize;
}

// timediff is in milliseconds!!
void Animation::addToTime(double timeDiff) {
//	if (MYINFO) 	std::cout << "Elapsed time (ms): " << timeDiff << std::endl;
	timeDiff /= SECtoMSEC; // now timeDiff is in seconds

	// calculate which frame we moved ahead to - depends on virtual fps
	curFrameFrac += timeDiff * virtFPS;
	while (curFrameFrac > frameNum) curFrameFrac -= frameNum; // TODO make it >= ?
	while (curFrameFrac < 0) curFrameFrac += frameNum; // so negative case is also handled

}

// reset to initial pose
void Animation::reset() {
	animating = false;
	curFrameFrac = -1;
	virtFPS = stdFPS;
}

void Animation::outputBVH(std::ostream& out) {
	out << std::fixed;
	out.precision(5);
	// first output the tree
	out << "HIERARCHY" << std::endl;
	for (std::vector<SkeletonNode>::iterator rootIt = roots.begin();
										rootIt != roots.end(); ++rootIt) {
		rootIt->printTreeBVH(out, 0);
	}

	// now output the animation code
	out.precision(7);
	out << "MOTION" << std::endl;
	out << "Frames: " << frameNum << std::endl;
	out << "Frame Time: " << stdFrameTime << std::endl;
	out.precision(4);
	for (unsigned f = 0; f < frameNum; ++f) {
		for (std::vector<SkeletonNode>::iterator rootIt = roots.begin();
											rootIt != roots.end(); ++rootIt) {
			rootIt->printFrameBVH(out, f);
		}
		out << std::endl;
	}
}

/** calculates an attachment to the bones of the specified model.
 * Note: here we assume there's one root only. */
void Animation::simpleAttachBones() {

	typedef Eigen::Triplet<double> Tr;
	std::vector<Tr> tripletList;
	unsigned numVert = model->getNumVertices();
	tripletList.reserve(numVert*2);

	// first version: for each vertex find the closest bone
	// -- if k tie for closest, then assign 1/k to each
	const Point * vertex;
	unsigned vNum = 0;
	while (vertex = model->getVertex(vNum), vertex != NULL) {
		std::vector<SkeletonNode> closests;
		roots[0].getClosestBones(Point(*vertex), std::numeric_limits<float>::max(), closests);
		for (std::vector<SkeletonNode>::const_iterator it = closests.begin(); it != closests.end(); ++it) {
			tripletList.push_back(Tr(vNum, it->getUpperBoneNum(), 1/double(closests.size()) ));
		}
		vNum++;
	}

	if (debug::ison(debug::EVERYTHING)) {
		std::cout << "num of triplets: " << tripletList.size() << std::endl;
		for (std::vector<Tr>::const_iterator it = tripletList.begin(); it != tripletList.end(); it++) {
			if (it->row() >= numVert || it->col() >= SkeletonNode::getNumberOfNodes()) {
				std::cerr << "issue with: " << it->row() << ", " << it->col() << std::endl;
			}
		}
	}

	simpleConMat.resize(numVert, SkeletonNode::getNumberOfNodes());
	simpleConMat.reserve(tripletList.size()*1.5);
	simpleConMat.setFromTriplets(tripletList.begin(), tripletList.end());
}

void Animation::updateMeshSelected() {
	if (model) {
		// want all the i's st (i, selectedBone) is nonzero in simpleConMat
		boost::shared_ptr< std::set<unsigned> > sel( new std::set<unsigned> );

		// not fast way:
		for (int row = 0; row < simpleConMat.rows(); ++row) {
			if (simpleConMat.coeff(row, selectedBone) != 0) sel->insert((unsigned) row);
		}

		model->setSelectedVerts(sel);
	}
}

void Animation::printSimpleAttachedMatrix(std::ostream& out) const throw(WrongStateException) {
	if (!model)
		throw WrongStateException("Tried to print the simple attached matrix before setting a model for the skeleton");

	for (unsigned v = 0; v < model->getNumVertices(); ++v) {
		printSparseRow(out, simpleConMat, v);
//		Eigen::SparseInnerVectorSet<float> curRow;
//		out << v << " " << simpleConMat.row(v);
	}
}


// displays the current frame (that has been already calculated from curTime)
// selectedbone is going to be drawn with red
void Animation::display(bool showSelBone) {

    glLineWidth(WIDTH);

	long curTime = glutGet(GLUT_ELAPSED_TIME);
	if (animating) {
		addToTime( curTime - timeOfPreviousCall ); // automatically converted to double
	}
	timeOfPreviousCall = curTime;

//	if (MYINFO) std::cout << "Drawing Frame " << curFrameFrac << std::endl;
	for (unsigned i = 0; i < roots.size(); ++i)
		roots[i].display(curFrameFrac, selectedBone);

    glLineWidth(1); // assume it's 1 by def

}


