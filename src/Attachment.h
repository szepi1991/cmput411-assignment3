/*
 * Attachment.h
 *
 *  Created on: 2012-11-13
 *      Author: david
 */

#ifndef ATTACHMENT_H_
#define ATTACHMENT_H_

#include "geometry.h"
#include <VoxBits/StrongPtr.h>

class SkeletonNode;

class Attachment {
private:
	VoxBits::strong_ptr<SkeletonNode> endJoint; // TODO think about what happens when this is copied
	Point attachPoint;
	float distance;

public:
	Attachment(SkeletonNode const& endJoint_, Point const& attachPoint_, float distance_);
	virtual ~Attachment() {};

	bool operator<(Attachment const& o) const;

	SkeletonNode const& getEndJoint() const { return *endJoint; }
	Point const& getAttachPoint() const { return attachPoint; }
	float getDistance() const {return distance;}
	friend std::ostream& operator<< (std::ostream &out, Attachment const& a);
};

std::ostream& operator<<(std::ostream& os, const Attachment& a);

#endif /* ATTACHMENT_H_ */
