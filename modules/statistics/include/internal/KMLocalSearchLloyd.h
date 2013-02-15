/**
 *  \file KMLocalSearchLloyd.h
 *  \brief Lloyd's algorithm with random restarts
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_KM_LOCAL_SEARCH_LLOYD_H
#define IMPSTATISTICS_INTERNAL_KM_LOCAL_SEARCH_LLOYD_H

#include "KMLocalSearch.h"
#include <IMP/statistics/statistics_config.h>
#include "IMP/base_types.h"

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE
#ifndef IMP_DOXYGEN

//! KMLocalSearchLloyd
/** Lloyd's algorithm with random restarts.
Each run is broken into trails, we keep to prefrom trails as long as we improve
the global distortion. The first failed trail ( a trail in which we could not
improve the total distortion) finalized the run as well. Each trail is broekn
into few lloyd stages.
A run starts by sampling center points at random.
Each run is provided two parameters, a maximum number
of runs per stage (maxRunStage) and a minimum accumulated
relative distortion loss (minAccumRDL).  If the accumulated RDL
for the run exceeds this value, then the run ends in success.
If the number of stages is exceeded before this happens, the run
ends in failure.
\unstable{KMLocalSearchLlouyd}
*/
class IMPSTATISTICSEXPORT KMLocalSearchLloyd : public KMLocalSearch {
public:
  KMLocalSearchLloyd(KMFilterCenters *sol, KMTerminationCondition *term)
    : KMLocalSearch(sol,term) {}
protected:
  //!Get the relative distortion loss for a trail
  double get_accumulated_rdl() {
    return (init_trail_dist_ - curr_->get_distortion()) / init_trail_dist_;
  }
  void log_stage(std::ostream &out=std::cout);
  void log_run() {
    IMP_LOG_VERBOSE("<Generating new random centers>" << std::endl);
  }
  //! Do base class resetting. Initialize is_new_phase to false and save
  //! the initial run distortion.
  virtual void reset();
  //! Do base class processing.  If there has been an improvement in distortion,
  //! save the current solution.
  void end_stage();
  //! Checks if the run is done
  /** If the number of stages exceeds the maximum stages (total or per run),
  then we are done.  If this is the first stage of the run, then we are not
  done, and we do beginning of run processing. Otherwise, if the relative
  distortion loss (RDL) is greater than minimum val then we are done (success).
  */
  virtual bool is_run_done();
  //! End the run
  /** If the accumulated RDL is smaller that the minimum predefined accumulated
  RDL then the run has ended unsuccessfully and we request the start of a new
  run. Otherwise the run has ended successfully, and we start a new run by
  saving the current run distortion.
  */
  void end_run();
  void preform_stage();
  double init_trail_dist_; // initial distortion for a trail
  bool is_new_trail_;
};

#endif

IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_KM_LOCAL_SEARCH_LLOYD_H */
