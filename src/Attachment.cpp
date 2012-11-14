/*
 * Attachment.cpp
 *
 *  Created on: 2012-11-13
 *      Author: david
 */

#include "SkeletonNode.h"
#include "Attachment.h"

Attachment::Attachment(SkeletonNode const& endJoint_, Point const& attachPoint_, float distance_)
		: endJoint(new SkeletonNode(endJoint_)), attachPoint(attachPoint_), distance(distance_) {}

bool Attachment::operator<(Attachment const& o) const {
	if (distance < o.distance) return true;
	if (distance > o.distance) return false;
	return (endJoint->getUpperBoneNum() < o.endJoint->getUpperBoneNum()); // differentiates between each attachment!
}


std::ostream& operator<<(std::ostream& os, const Attachment& a) {
	os << "Attach[boneNum=" << a.endJoint->getUpperBoneNum()
			<< ";endName=" << a.endJoint->getDescr()
			<< ";attachPoint=" << a.attachPoint
			<< ";dist=" << a.distance << "]";
	return os;
}



