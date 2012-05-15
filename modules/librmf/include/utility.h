/**
 *  \file RMF/utility.h
 *  \brief Helper functions for manipulating RMF files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_UTILITY_H
#define IMPLIBRMF_UTILITY_H

#include "RMF_config.h"
#include "internal/intrusive_ptr_object.h"
#include "internal/errors.h"
namespace RMF {

  class FileConstHandle;
  class FileHandle;

/** Turn on and off printing of hdf5 error messages. They can help in
      diagnostics, but, for the moment, can only be output to standard
      error and so are off by default.
*/
inline void set_show_hdf5_errors(bool tf) {
  internal::show_hdf5_errors=tf;
}


  /** Copy the hierarchy structure and set structure from one rmf
      file to another.*/
RMFEXPORT void copy_structure(FileConstHandle in, FileHandle out);


  /** Copy the hierarchy structure and set structure from one rmf
      file to another.*/
  RMFEXPORT void copy_frame(FileConstHandle in, FileHandle out,
                            unsigned int inframe,
                            unsigned int outframe);

  /** Return true of the two have the same structure.*/
  RMFEXPORT bool get_equal_structure(FileConstHandle in, FileConstHandle out,
                                     bool print_diff=false);
  /** Return true of the two have the same structure.*/
  RMFEXPORT bool get_equal_frame(FileConstHandle in, FileConstHandle out,
                                 unsigned int inframe, unsigned int outframe,
                                 bool print_diff=false);


} /* namespace RMF */

#endif /* IMPLIBRMF_UTILITY_H */
