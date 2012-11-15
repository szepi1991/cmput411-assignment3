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
#include <Eigen/Dense>

#include "SkeletonNode.h"
#include "Mesh.h"
#include "myexceptions.h"

class LineSegment;

class Animation {
public:
	enum AttachMatrix {SIMPLE_M, VISIBLE_M};

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
	AttachMatrix displayOnMeshType;

	// matricies for skin-bone attachment
	boost::shared_ptr<Mesh> model; // TODO should be const Mesh?? and all other places too
	// see: http://stackoverflow.com/questions/8793703/const-correctness-with-smart-pointers
	Eigen::SparseMatrix<double> simpleConMat;
	Eigen::SparseMatrix<double> visConMat;
	Eigen::VectorXd importances;
	Eigen::MatrixXd attachWeight;

	// TODO testing
	std::vector< std::vector<LineSegment> > intersectingAtt;
	std::vector< std::vector<LineSegment> > connectedAtt;

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
		importances.resize(model->getNumVertices()); // this does not look like a good place for this..
		attachBonesToMesh(); // FIXME testing
//		attachWeight.resize(model->getNumVertices(), SkeletonNode::getNumberOfNodes());
//		attachWeight = Eigen::MatrixXd::Identity(model->getNumVertices(), SkeletonNode::getNumberOfNodes());
		precalculateMesh();
	}
	void printAttachedMatrix(std::ostream& out, AttachMatrix mType) const throw(WrongStateException);
	void printImportances(std::ostream& out) const throw(WrongStateException);
	void printFinalAttachMatrix(std::ostream& out) const throw(WrongStateException);

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
	void nextConnectionDisplayType() {
		displayOnMeshType = AttachMatrix ((int) displayOnMeshType + 1);
		if ((int) displayOnMeshType > (int) VISIBLE_M) displayOnMeshType = (AttachMatrix) 0; // do not forget to update this
		std::cout << "On mesh we are now displaying attachments of type ";
		switch (displayOnMeshType) {
		case SIMPLE_M: std::cout << "SIMPLE"; break;
		case VISIBLE_M: std::cout << "VISIBLE"; break;
		default: throw(0);
		}
		std::cout << std::endl;
		updateMeshSelected();
	}

private:
	void attachBonesToMesh();
	void findFinalAttachmentWeights(Eigen::SparseMatrix<double>* connMatrixToUse);
	void updateMeshSelected();
	void precalculateMesh();
	bool tryLoadingAttached();

};

#endif /* ANIMATION_H_ */
