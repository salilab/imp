/**
 *  \file IMP/container/ClassnameContainerStatistics.h
 *  \brief A container for Classnames.
 *
 *  BLURB
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 */

#ifndef IMPCONTAINER_CLASSNAME_CONTAINER_STATISTICS_H
#define IMPCONTAINER_CLASSNAME_CONTAINER_STATISTICS_H

#include <IMP/container/container_config.h>
#include <IMP/ClassnameContainer.h>
#include <IMP/ScoreState.h>
#include <IMP/score_state_macros.h>
#include <IMP/compatibility/set.h>

IMPCONTAINER_BEGIN_NAMESPACE

//! Track statistics on a ClassnameContainer
/** The current statistics are average and min/max occupancy. Other
    statistics can be added on request, but we probably want to
    restrict it to ones that are cheap to gather. */
class IMPCONTAINEREXPORT ClassnameContainerStatistics : public ScoreState
{
  base::Pointer<ClassnameContainer> container_;
  unsigned int total_;
  unsigned int checks_;
  unsigned int max_;
  unsigned int min_;
  bool track_unique_;
  IMP::compatibility::set<VARIABLETYPE> unique_;
public:
  ClassnameContainerStatistics(ClassnameContainerAdaptor c);
  void show_statistics(std::ostream &out) const;
  /** Keeping track of the number of unique entries seen is
      expensive, so it is not done by default.
  */
  void set_track_unique(bool tf);
  IMP_SCORE_STATE(ClassnameContainerStatistics);
};


IMPCONTAINER_END_NAMESPACE

#endif  /* IMPCONTAINER_CLASSNAME_CONTAINER_STATISTICS_H */
