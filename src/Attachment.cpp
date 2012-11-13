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




