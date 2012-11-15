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
#include "Attachment.h"

#ifdef __APPLE__
#  include <GLUT/glut.h>
#else
#  include <GL/glut.h>
#endif

#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>
#include <ctime>

bool Animation::test59Vert = false;

Animation::Animation(char *filename) throw(ParseException) :
					figureSize(0), selectedBone(0), displayOnMeshType(SIMPLE_M) {

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

	for (std::vector<SkeletonNode>::iterator rootIt = roots.begin();
										rootIt != roots.end(); ++rootIt) {
		rootIt->setWorldOffsetRec(Point());
	}

	std::cout << "Finished." << std::endl;
	this->filename = filename;
	animating = false;

	stdFPS = floor((1.0 / stdFrameTime) + 0.5);
	virtFPS = stdFPS;

	curFrameFrac = -1;

	// TODO test: setup attachment vectors to have correct size
	for (unsigned i = 0; i < SkeletonNode::getNumberOfNodes(); ++i) {
		std::vector<LineSegment> empty;
		intersectingAtt.push_back(empty);
		connectedAtt.push_back(empty);
	}
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
 * Note: here we assume there's one root only.
 */
void Animation::attachBonesToMesh() {

	std::cout << "Starting to attach bones.." << std::endl;
	time_t start,end;

	typedef Eigen::Triplet<double> Tr;
	std::vector<Tr> simpleTripletList;
	std::vector<Tr> visibleTripletList;
	unsigned numVert = model->getNumVertices();
	simpleTripletList.reserve(numVert*2); // TODO try changing this and see what happens
	visibleTripletList.reserve(numVert*2);

	if (debug::ison(debug::LITTLE)) std::cout << "verts=" << numVert << ": ";

	// first version: for each vertex find the closest bone
	// -- if k tie for closest, then assign 1/k to each
	const Point * vertex;
	unsigned vNum = 0;

	std::ofstream logfile("attachments.log");
	std::ofstream sdistsfile("Sdists.log");

	time (&start);
	while (vertex = model->getOrigVertex(vNum), vertex != NULL
//			&& vNum < 10 // FIXME test
										) {
		if (debug::ison(debug::LITTLE)) {
			std::cout << vNum << " ";
			std::flush(std::cout);
		}
		if (vNum == 59) test59Vert = true;
		else test59Vert = false;

		std::set<Attachment> attachments;
		if (debug::ison(debug::EVERYTHING))
			std::cout << "+ Studying point " << vNum << " that is " << *vertex << std::endl;
		roots[0].getClosestBones(Point(*vertex), attachments);

		if (attachments.size()+1 != SkeletonNode::getNumberOfNodes()) {
			std::cout << "attachments != bones: " << attachments.size() << ", " << SkeletonNode::getNumberOfNodes() << std::endl;
			assert(false);
		}

		// TODO testing
		logfile << "---- Vertex " << vNum << ": " << *vertex << " ----" << std::endl;
		for (std::set<Attachment>::const_iterator setIt = attachments.begin();
				setIt != attachments.end(); ++setIt) {
			logfile << *setIt << std::endl;
		}

		// now find the list of closest attachments (attachments is ordered so easy)
		std::vector<SkeletonNode> closests;
		float minSimpleDist = attachments.begin()->getDistance(); // there's always at least one
		for (std::set<Attachment>::const_iterator setIt = attachments.begin();
				setIt != attachments.end(); ++setIt) {
			if (setIt->getDistance() > minSimpleDist+EPS) break; // no other can be good
			closests.push_back(setIt->getEndJoint());
		}
		for (std::vector<SkeletonNode>::const_iterator it = closests.begin(); it != closests.end(); ++it) {
			simpleTripletList.push_back(Tr(vNum, it->getUpperBoneNum(), 1.0/double(closests.size()) ));
		}

		// TODO testing
		sdistsfile << vNum << " " << minSimpleDist << std::endl;

		// now find the list of closest VISIBLE attachments (attachments is ordered so easy)
		std::vector<SkeletonNode> closestsVis;
		float minVisDist = std::numeric_limits<float>::max()-2*EPS; // not smaller than any element
		bool distSet = false;
		for (std::set<Attachment>::const_iterator setIt = attachments.begin();
				setIt != attachments.end(); ++setIt) {
			unsigned boneNum = setIt->getEndJoint().getUpperBoneNum();

			LineSegment attachLine(setIt->getAttachPoint(), *vertex);
			if (model->intersects(attachLine)) {
				intersectingAtt[boneNum].push_back(attachLine); // TODO TESTING
				continue; // not visible
			}

			if (setIt->getDistance() > minVisDist+EPS) break; // no other can be good
			if (!distSet) {
				distSet = true;
				minVisDist = setIt->getDistance();
			}
			connectedAtt[boneNum].push_back(attachLine); // TODO TESTING
			closestsVis.push_back(setIt->getEndJoint());
		}
		if (closestsVis.size() != 0) {
			importances(vNum, 0) = 1.0 / sqr(minVisDist);
		} else {
			importances(vNum, 0) = 0;
		}
		for (std::vector<SkeletonNode>::const_iterator it = closestsVis.begin(); it != closestsVis.end(); ++it) {
			visibleTripletList.push_back(Tr(vNum, it->getUpperBoneNum(), 1.0/double(closestsVis.size()) ));
		}

		vNum++;
	}
	if (debug::ison(debug::LITTLE)) std::cout << std::endl;
	time (&end);

	logfile.close();
	sdistsfile.close();
	std::cout << "Simple and visible attachment matrices created in " << difftime(end,start) << "s" << std::endl;

//	if (debug::ison(debug::EVERYTHING)) {
//		std::cout << "num of triplets: " << simpleTripletList.size() << std::endl;
//		for (std::vector<Tr>::const_iterator it = simpleTripletList.begin(); it != simpleTripletList.end(); it++) {
//			if (it->row() >= numVert || it->col() >= SkeletonNode::getNumberOfNodes()) {
//				std::cerr << "issue with: " << it->row() << ", " << it->col() << std::endl;
//			}
//		}
//	}

	simpleConMat.resize(numVert, SkeletonNode::getNumberOfNodes());
	simpleConMat.reserve(simpleTripletList.size()*1.5);
	simpleConMat.setFromTriplets(simpleTripletList.begin(), simpleTripletList.end());

	visConMat.resize(numVert, SkeletonNode::getNumberOfNodes());
	visConMat.reserve(visibleTripletList.size()*1.5);
	visConMat.setFromTriplets(visibleTripletList.begin(), visibleTripletList.end());

	findFinalAttachmentWeights(&simpleConMat);
}

void Animation::findFinalAttachmentWeights(Eigen::SparseMatrix<double>* connMatrixToUse) {
	// we do it for each column separately
	std::cout << "Calculating W.." << std::endl;
	int verts = model->getNumVertices();
	int bones = SkeletonNode::getNumberOfNodes();
	attachWeight.resize(verts, bones);

	typedef Eigen::SparseMatrix<double> SpMat;
	SpMat Dh = delta(importances);
	SpMat A = model->getLaplacian() + Dh;
	Eigen::SimplicialCholesky<SpMat> chol(A); // performs a Cholesky factorization of A
	// for each column of visConMat do it separately:
	for (int curCol = 0; curCol < visConMat.cols(); ++curCol) {
		Eigen::VectorXd col = visConMat.col(curCol);
		Eigen::VectorXd b = Dh*col;
		attachWeight.col(curCol) = chol.solve(b);
//		Eigen::VectorXd x = chol.solve(b); // use the factorization to solve for the given right hand side
	}
	std::cout << "Done." << std::endl;

	// check correctness
	Eigen::MatrixXd res = attachWeight * Eigen::VectorXd::Ones(bones);
	std::cout << "These all should be ones in the next line:" << std::endl << "\t";
	for (int i = 0; i < verts; ++i) {
		std::cout << " " << res(i);
	}
	std::cout << std::endl;
}

void Animation::updateMeshSelected() {
	if (model) {
		// want all the i's st (i, selectedBone) is nonzero in simpleConMat
		boost::shared_ptr< std::set<unsigned> > sel( new std::set<unsigned> );
		Eigen::SparseMatrix<double>* matToUse;
		switch (displayOnMeshType) {
		case SIMPLE_M: matToUse = &simpleConMat; break;
		case VISIBLE_M: matToUse = &visConMat; break;
		default: throw(0);
		}

		// not fast way:
		for (int row = 0; row < matToUse->rows(); ++row) {
			if (matToUse->coeff(row, selectedBone) != 0) sel->insert((unsigned) row);
		}

		if (debug::ison(debug::EVERYTHING)) {
			std::cout << sel->size() << " vertices attached to current bone. These are" << std::endl;
			for (std::set<unsigned>::const_iterator it = sel->begin();
													it != sel->end(); ++it) {
				std::cout << "\t" << *it << ": " << *(model->getOrigVertex(*it)) << std::endl;
			}
		}
		model->setSelectedVerts(sel);
	}
}

void Animation::printAttachedMatrix(std::ostream& out, AttachMatrix mType) const throw(WrongStateException) {
	if (!model)
		throw WrongStateException("Tried to print the attached matrix before setting a model for the skeleton");

	for (unsigned v = 0; v < model->getNumVertices(); ++v) {
		switch (mType) {
		case SIMPLE_M:
			printSparseRow(out, simpleConMat, v);
			break;
		case VISIBLE_M:
			printSparseRow(out, visConMat, v);
			break;
		default:
			throw 0;
		}
	}
}

void Animation::printFinalAttachMatrix(std::ostream& out) const throw(WrongStateException) {
	if (!model)
		throw WrongStateException("Tried to print the attached matrix before setting a model for the skeleton");
//	out << attachWeight;
	for (int i = 0; i < attachWeight.rows(); ++i) {
		out << i;
		for (int j = 0; j < attachWeight.rows(); ++j) {
			out << " " << attachWeight(i,j);
		}
		out << std::endl;
	}
}

void Animation::printImportances(std::ostream& out) const throw(WrongStateException) {
	if (!model)
		throw WrongStateException("Tried to print the attached matrix before setting a model for the skeleton");

	for (int i = 0; i < importances.rows(); ++i) {
		out << i << " " << importances(i, 0);
		out << std::endl; //for some reason eclipse does not like it in the previous row
	}
}

void Animation::precalculateMesh() {
	std::cout << "Pre-calculating mesh animation..";
	flush(std::cout);

	const std::vector<Point> oPoints = model->getOrigVertices();
	const unsigned bones = attachWeight.cols();
	if (bones != SkeletonNode::getNumberOfNodes()) {
		std::cout << "bones vs nodeNum = " << bones << " vs " << SkeletonNode::getNumberOfNodes() << std::endl;
		assert(false);
	}

	std::vector<Point> newNormals; // fake one

	// for each frame
	for (unsigned f = 0; f < frameNum; ++f) {
		//for each original point (// FIXME later handle normals too)
		std::vector<Point> newPoints;
		for (unsigned vNum = 0; vNum < oPoints.size(); ++vNum) {
			Point newPoint(0,0,0);
			for (unsigned cBone = 0; cBone < bones; ++cBone) {
				if (attachWeight(vNum, cBone) > EPS) {
					Eigen::Vector4d oldLoc = getVectorForm(oPoints[vNum]);
					roots[0].getLocation(oldLoc, cBone, f);
					newPoint += Point(oldLoc(0), oldLoc(1), oldLoc(2)) * attachWeight(vNum, cBone);
				}
			}
			newPoints.push_back(newPoint);
		}
		model->addFrame(newPoints, newNormals);
	}

	std::cout << "Done" << std::endl;
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

	int frame = int(curFrameFrac); // should be ok .. handles -1 and positive values?
	model->display(-1); // FIXME later this should be frame

//	if (MYINFO) std::cout << "Drawing Frame " << curFrameFrac << std::endl;
	for (unsigned i = 0; i < roots.size(); ++i) {
		roots[i].display(curFrameFrac, selectedBone); // TODO perhaps swtich to frame here too??
	}

	if (debug::ison(debug::EVERYTHING)) {
		float currentColor[4];
		glGetFloatv(GL_CURRENT_COLOR,currentColor);

	    glLineWidth(2);
	    // also draw the good and bad attachements focrresponding to currently selected bone
		glColor3f(0.0, 0.0, 1.0); // blue
		for (std::vector<LineSegment>::const_iterator it = intersectingAtt[selectedBone].begin();
				it != intersectingAtt[selectedBone].end(); ++it) {
			it->display();
		}
		// draw good connections
		glColor3f(0.0, 1.0, 0.0); // green
		for (std::vector<LineSegment>::const_iterator it = connectedAtt[selectedBone].begin();
				it != connectedAtt[selectedBone].end(); ++it) {
			it->display();
		}

		glColor4fv(currentColor); // reset
	}

    glLineWidth(1); // assume it's 1

}


