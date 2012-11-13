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

	bool operator<(Attachment const& o) const {
		return (distance < o.distance);
	}

	SkeletonNode const& getEndJoint() const { return *endJoint; }
	Point const& getAttachPoint() const { return attachPoint; }
	float getDistance() const {return distance;}

};

#endif /* ATTACHMENT_H_ */
