/**
 *  \file RMF/Category.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#include "RMF/enums.h"

RMF_ENABLE_WARNINGS

namespace RMF {
const NodeType INVALID_NODE_TYPE(-1, "inv");
const NodeType ROOT(0, "root");
const NodeType oREPRESENTATION(1, "rep");
const NodeType REPRESENTATION(1, "representation");
const NodeType oGEOMETRY(2, "geom");
const NodeType GEOMETRY(2, "geometry");
const NodeType oFEATURE(3, "feat");
const NodeType FEATURE(3, "feature");
const NodeType ALIAS(4, "alias");
const NodeType CUSTOM(5, "custom");
const NodeType BOND(6, "bond");
const NodeType ORGANIZATIONAL(7, "organizational");
const NodeType LINK(8, "link");
const NodeType PROVENANCE(9, "provenance");

const FrameType INVALID_FRAME_TYPE(-1, "inv");
const FrameType STATIC(0, "static");
const FrameType FRAME(1, "frame");
const FrameType MODEL(2, "model");
const FrameType CENTER(3, "center");
const FrameType FRAME_ALIAS(4, "alias");
const FrameType ALTERNATE(5, "alternate");

const RepresentationType PARTICLE(0, "particle");
const RepresentationType GAUSSIAN_PARTICLE(1, "gaussian_particle");

} /* namespace RMF */

RMF_DISABLE_WARNINGS
