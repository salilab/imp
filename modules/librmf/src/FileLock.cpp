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
#if BOOST_VERSION < 103500
#include <boost/thread/xtime.hpp>
#endif

namespace RMF {

FileLock::FileLock(FileConstHandle h, double retry_milliseconds): h_(h) {
  while (!h_.set_is_locked(true)) {
#if BOOST_VERSION < 103500
    boost::xtime xt;
    boost::xtime_get(&xt, boost::TIME_UTC);
    xt.nsec += 1000000 * retry_milliseconds;
    boost::thread::sleep(xt);
#else
    boost::this_thread::sleep(boost::posix_time
                              ::milliseconds(retry_milliseconds));
#endif
  }
}

FileLock::~FileLock() {
  h_.set_is_locked(false);
}

} /* namespace RMF */
