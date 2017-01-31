/**
 *  \file RMF/enums.h
 *  \brief The various enums used in RMF.
 *
 *  Copyright 2007-2017 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_ENUMS_H
#define RMF_ENUMS_H

#include "RMF/config.h"
#include "Enum.h"
#include <string>
#include <iostream>

RMF_ENABLE_WARNINGS

namespace RMF {
/** \name Node Types
    \anchor nodetypes

    The types nodes can take in an RMF file.
    @{
*/

extern RMFEXPORT const NodeType INVALID_NODE_TYPE;
//! The root node
extern RMFEXPORT const NodeType ROOT;

//! Represent part of a molecule
extern RMFEXPORT const NodeType REPRESENTATION;
//! Store a geometric object
extern RMFEXPORT const NodeType GEOMETRY;
//! Store information about some feature of the system
/** For example, the particles involved in scoring
  functions and their score can be encoded as feature
  nodes.
*/
extern RMFEXPORT const NodeType FEATURE;
/** Store a reference to another node. This node should
    be an alias decorator node and have no other data,
    at least for now. Aliases should be thought of as simply referencing
    existing objects in the scene, not creating new objects.
 */
extern RMFEXPORT const NodeType ALIAS;
//! Arbitrary data that is not standardized
/** Programs can use these keys to store any extra data
    they want to put into the file.
 */
extern RMFEXPORT const NodeType CUSTOM;
//! A link between two atoms
/** These are mostly for display purposes e.g. to show a wireframe
    view of the molecule. */
extern RMFEXPORT const NodeType BOND;
//! A node that is purely there for organizational purposes
/** This includes nodes that are just RMF::ReferenceFrame nodes.
 */
extern RMFEXPORT const NodeType ORGANIZATIONAL;
#ifndef RMF_DOXYGEN
//! An internal link to another node
extern RMFEXPORT const NodeType LINK;
#endif
/**@} */

/** \name Frame Types
    \anchor frametypes

    The types frames can take in an RMF file.
    @{
*/
extern RMFEXPORT const FrameType INVALID_FRAME_TYPE;
//! The root frame
extern RMFEXPORT const FrameType STATIC;
//! A frame in a sequence of frames
extern RMFEXPORT const FrameType FRAME;
//! An independent model
extern RMFEXPORT const FrameType MODEL;
//! A cluster center
extern RMFEXPORT const FrameType CENTER;
//! An alias for another frame
extern RMFEXPORT const FrameType FRAME_ALIAS;
//! An alternate state for a frame or model
extern RMFEXPORT const FrameType ALTERNATE;
/** @} */

/** \name Representation Types
    \anchor representationtypes

    The types that representations used in the decorator::Alternatives decorator
    can take.
    @{
*/
/** Representation via decorator::Particle nodes, the default. */
extern RMFEXPORT const RepresentationType PARTICLE;

/** Represent the structure with at set of decorator::GaussianParticle nodes. */
extern RMFEXPORT const RepresentationType GAUSSIAN_PARTICLE;
/** @} */

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_ENUMS_H */
