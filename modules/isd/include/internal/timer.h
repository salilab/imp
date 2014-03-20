/**
 *  \brief A timer class to time function calls
 *
 *  Copyright 2007-2014 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPISD_INTERNAL_TIMER_H
#define IMPISD_INTERNAL_TIMER_H

#include <IMP/isd/isd_config.h>

#include <boost/date_time/posix_time/posix_time.hpp>

IMPISD_BEGIN_INTERNAL_NAMESPACE

using boost::posix_time::microsec_clock;
using boost::posix_time::ptime;
using boost::posix_time::time_duration;

// struct for call statistics
struct func_stats {
  ptime start;
  ptime stop;
  time_duration total, firstcall, lastcall;
  unsigned ncalls;
};

/** Simple class to monitor runtime duration of some functions
 * */
template <unsigned nfuncs>
class CallTimer {

 private:
  func_stats stats_[nfuncs];

 private:
  void start_helper(int name) {
    stats_[name].start = microsec_clock::local_time();
  }

  void stop_helper(int name) {
    stats_[name].stop = microsec_clock::local_time();
    stats_[name].lastcall = stats_[name].stop - stats_[name].start;
    stats_[name].total += stats_[name].lastcall;
    if (stats_[name].ncalls == 0)
      stats_[name].firstcall = stats_[name].lastcall;
    stats_[name].ncalls += 1;
  }

 public:
  CallTimer() {
    for (unsigned i = 0; i < nfuncs; i++) {
      stats_[i].ncalls = 0;
      stats_[i].total = time_duration(0, 0, 0);
    }
  }

  void start(int name) const {
    const_cast<CallTimer *>(this)->start_helper(name);
  }

  void stop(int name) const {
    const_cast<CallTimer *>(this)->stop_helper(name);
  }

  void stats(int name, std::string dispname) const {
    std::cout << dispname << "\t" << stats_[name].total.total_milliseconds();
    if (stats_[name].ncalls > 0) {
      std::cout << "\t" << stats_[name].total.total_microseconds() /
                               stats_[name].ncalls;
    } else {
      std::cout << "\t---";
    }
    std::cout << "\t" << stats_[name].firstcall.total_microseconds() << "\t"
              << stats_[name].lastcall.total_microseconds() << "\t"
              << stats_[name].ncalls << std::endl;
  }

  void stats(std::string dispnames[]) const {
    std::cout
        << "Function\tTotal(ms)\tMean(mus)\tFirst(mus)\tLast(mus)\tNumber\n";
    for (unsigned i = 0; i < nfuncs; i++) stats(i, dispnames[i]);
  }

  func_stats stats_data(int name) { return stats_[name]; }
};

IMPISD_END_INTERNAL_NAMESPACE

#endif /* IMPISD_INTERNAL_TIMER_H */
