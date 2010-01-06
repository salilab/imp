/**
 *  \file IMP/benchmark/macros.h
 *  \brief Various general useful macros for IMP.
 *
 *  Copyright 2007-2010 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPBENCHMARK_MACROS_H
#define IMPBENCHMARK_MACROS_H

#include <boost/timer.hpp>


//! Time the given command and assign the time of one iteration to the variable
/** The units for the time are in seconds. See also IMP_TIME_N */
#define IMP_TIME(block, timev)                  \
  {                                             \
    boost::timer imp_timer;                     \
    unsigned int imp_reps=0;                    \
    do {                                        \
      block;                                    \
      ++imp_reps;                               \
    } while (imp_timer.elapsed() < 2.5);        \
    timev= imp_timer.elapsed()/imp_reps;        \
  }

//! Time the given command and assign the time of one iteration to the variable
/** The units for the time are in seconds. The bit of code is run
    exact N times. See also IMP_TIME */
#define IMP_TIME_N(block, timev, N)             \
  {                                             \
    boost::timer imp_timer;                     \
    for (unsigned int i=0; i< (N); ++i) {       \
      block;                                    \
    }                                           \
    timev= imp_timer.elapsed()/(N);             \
  }


#endif  /* IMPBENCHMARK_MACROS_H */
