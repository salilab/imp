/**
 *  \file RMF/utility.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 *
 */

#ifndef RMF_UTILITY_H
#define RMF_UTILITY_H

#include "NodeConstHandle.h"
#include "RMF/config.h"
#include "RMF/internal/errors.h"
#include "Vector.h"
#include <array>

RMF_ENABLE_WARNINGS

namespace RMF {

class FileConstHandle;
class FileHandle;
class NodeConstHandle;
class NodeHandle;

/** \name Copy functions

    See the example cloning.py for a usage example.

    @{
 */

/** Copy the hierarchy structure and set structure from one RMF
    file to another.*/
RMFEXPORT void clone_file_info(FileConstHandle input, FileHandle output);

/** Copy the hierarchy structure and set structure from one RMF
    file to another.*/
RMFEXPORT void clone_hierarchy(FileConstHandle input, FileHandle output);

/** Copy the data of a single frame from between two files.*/
RMFEXPORT void clone_loaded_frame(FileConstHandle input, FileHandle output);

/** Copy the data of a single frame from between two files. Parts missing
    in the output file will be skipped.*/
RMFEXPORT void clone_static_frame(FileConstHandle input, FileHandle output);
/** @} */

/** Return true of the two have the same structure.*/
RMFEXPORT bool get_equal_structure(FileConstHandle input,
                                   FileConstHandle output,
                                   bool print_diff = false);
/** Return true of the two have the same values in the current frame.*/
RMFEXPORT bool get_equal_current_values(FileConstHandle input,
                                        FileConstHandle out);

/** Return true if the two have the same values in the static frame.*/
RMFEXPORT bool get_equal_static_values(FileConstHandle input,
                                       FileConstHandle out);

/** This function simply throws an exception. It is here for testing.*/
RMFEXPORT void test_throw_exception();

/** Return a lower bound/upper bound pair that bounds the data stored in the
 * tree.
 */
RMFEXPORT std::array<RMF::Vector3, 2> get_bounding_box(NodeConstHandle root);

/** Return the diameter of the system. Unlike bounding box, this one can
    be called from python. */
RMFEXPORT float get_diameter(NodeConstHandle root);

} /* namespace RMF */

RMF_DISABLE_WARNINGS

#endif /* RMF_UTILITY_H */
