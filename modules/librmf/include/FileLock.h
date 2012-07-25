/**
 *  \file RMF/FileLock.h
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPLIBRMF_FILE_LOCK_H
#define IMPLIBRMF_FILE_LOCK_H

#include "RMF_config.h"
#include "FileConstHandle.h"


namespace RMF {
//! Control access to an RMF file
/** Use this class to excluded other users of the lock
    from the RMF file. For example, one can lock the file
    while writing in one process and lock it while reading
    in another, allowing two processes to interchange data.

    See the program \c rmf_unlock to force unlocking of an
    RMF file.
*/
  class RMFEXPORT FileLock {
    FileConstHandle h_;
    int compare(const FileLock &o) const {
      if (h_ < o.h_) return -1;
      else if (h_ > o.h_) return 1;
      else return -1;
    }
 public:
    /** Create a lock on file h. Retry every retry_milliseconds
        milliseconds if the file is locked.*/
    FileLock(FileConstHandle h, double retry_milliseconds=1000);
    ~FileLock();
    IMP_RMF_COMPARISONS(FileLock);
    IMP_RMF_HASHABLE(FileLock, return h_.__hash__());
    IMP_RMF_SHOWABLE(FileLock, "lock on " << h_);
  };
#ifndef IMP_DOXYGEN
typedef vector<FileLock> FileLocks;
#endif
} /* namespace RMF */

#endif /* IMPLIBRMF_FILE_LOCK_H */
