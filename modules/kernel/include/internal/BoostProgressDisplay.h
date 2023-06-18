/**
 *  \file internal/BoostProgressDisplay.h
 *  \brief Get the most appropriate Boost class to Display progress
 *
 *  Copyright 2007-2023 IMP Inventors. All rights reserved.
 */

#ifndef IMPKERNEL_INTERNAL_BOOST_PROGRESS_DISPLAY_H
#define IMPKERNEL_INTERNAL_BOOST_PROGRESS_DISPLAY_H

#include <IMP/kernel_config.h>
#include <boost/version.hpp>
#if BOOST_VERSION >= 107200
# include <boost/timer/progress_display.hpp>
#else
# include <boost/progress.hpp>
#endif

IMPKERNEL_BEGIN_INTERNAL_NAMESPACE

#if BOOST_VERSION >= 107200
typedef boost::timer::progress_display BoostProgressDisplay;
#else
typedef boost::progress_display BoostProgressDisplay;
#endif

IMPKERNEL_END_INTERNAL_NAMESPACE

#endif  /* IMPKERNEL_INTERNAL_BOOST_PROGRESS_DISPLAY_H */
