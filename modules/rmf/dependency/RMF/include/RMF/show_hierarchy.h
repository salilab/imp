/**
 *  \file RMF/show_hierarchy.h
 *  \brief Functions to show the hierarchy.
 *
 *  Copyright 2007-2019 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_NODE_SHOW_HIERARCHY_H
#define RMF_NODE_SHOW_HIERARCHY_H

#include <RMF/config.h>
#include <iostream>

RMF_ENABLE_WARNINGS

namespace RMF {
class NodeConstHandle;
class FileConstHandle;

/** \name Showing the hierarchy

These functions print out the hierarchy with various extra information
for each node. Each prints out an ASCII tree designed for human inspection.
@{ */

/** Print out just the node names and types.
 */
RMFEXPORT void show_hierarchy(NodeConstHandle root,
                              std::ostream& out = std::cout);

/** Print out all attributes for each node for the current frame.
 */
RMFEXPORT void show_hierarchy_with_values(NodeConstHandle root,
                                          std::ostream& out = std::cout);

/** Print out the list of decorators that can be used to decorate each node
    based on the current attributes in the loaded frame..
 */
RMFEXPORT void show_hierarchy_with_decorators(NodeConstHandle root,
                                              bool verbose = false,
                                              std::ostream& out = std::cout);

/** Show the frame hierarchy rather than the node hierarchy. */
RMFEXPORT void show_frames(FileConstHandle fh, std::ostream& out = std::cout);

/** @} */
} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_NODE_SHOW_HIERARCHY_H */
