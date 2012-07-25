/**
 *  \file RMF/FileLock.cpp
 *  \brief Handle read/write of Model data from/to files.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#include <RMF/FileLock.h>

// waiting
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>

namespace RMF {

FileLock::FileLock(FileConstHandle h, double retry_milliseconds): h_(h) {
  while (!h_.set_is_locked(true)) {
    boost::this_thread::sleep(boost::posix_time
                              ::milliseconds(retry_milliseconds));
  }
}

FileLock::~FileLock() {
  h_.set_is_locked(false);
}

} /* namespace RMF */
