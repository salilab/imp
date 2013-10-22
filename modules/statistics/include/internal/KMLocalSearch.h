/**
 *  \file KMLocalSearch.h   \brief Generic algorithm from k-means
 *                                 clustering by local search
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPSTATISTICS_INTERNAL_KM_LOCAL_SEARCH_H
#define IMPSTATISTICS_INTERNAL_KM_LOCAL_SEARCH_H

#include <IMP/statistics/statistics_config.h>
#include "KMFilterCenters.h"
#include "KMCenters.h"
#include "KMTerminationCondition.h"
#include <IMP/base_types.h>
#include <vector>

IMPSTATISTICS_BEGIN_INTERNAL_NAMESPACE

#ifndef IMP_DOXYGEN

//! KMLocalSearch
/** A generic algorithm for k-means clustering by local search.
The generic algorithm begins by generating an initial solution "curr" and
saving it in "best".  The value of "curr" reflects the current solution and
"best" reflects the best solution seen so far.
The algorithm consists of some number of basic iterations, called "stages"
(for example one stage of Lloyd algorithm). Stages are grouped into "runs".
Intuitively, a run involves a (small) number of stages in search of a better
solution.  A run might end, say, because a better solution was found or a fixed
number of stages have been performed without any improvement.
After a run is finished, we check to see whether we want to "accept" the
solution. Presumably this happens if the cost is lower, but it may happen even
if the cost is inferior in other circumstances (e.g., as part of a simulated
annealing approach). Accepting a solution means copying the current solution to
the saved solution. In some cases, the acceptance may involve reseting the
current solution to a random solution.
The generic algorithm:
\verbatim
   reset()                 // resets curr and best
   while ( !is_done() ) {  // while not done
     begin_run()                                // begin a new run
     do{                                        // do while run is not done
       begin_stage()                            // end of stage processing
       preform_stage()                          //apply a stage
       end_stage()                              // end of stage processing
     } while ( !is_run_done() )                 // while run is not done
     try_acceptance()                           // accept if appropriate
     endRun()                                   // end of run processing
   }
   return best                                  // return best solution
\endverbatim
\unstable{KMLocalSearch}
*/
class IMPSTATISTICSEXPORT KMLocalSearch {
 public:
  KMLocalSearch(KMFilterCenters *sol, KMTerminationCondition *term);

  virtual ~KMLocalSearch() {}
  //! Execute the k-mean clustering
  virtual void execute();
  //!Return total number of stages
  int get_total_number_of_stages() const { return stage_num_; }
  const KMFilterCentersResults &get_best() const { return best_; }

 protected:
  virtual void reset();
  virtual bool is_done() const;
  virtual void begin_run();
  // apply stage
  virtual void preform_stage() {}
  virtual void begin_stage() {}
  //! End of stage processing
  virtual void end_stage();
  virtual bool is_run_done() { return is_done(); }
  virtual void end_run() {}
  //! Test acceptance
  virtual void try_acceptance();
  virtual void log_stage(std::ostream &out = std::cout);

  Int num_of_data_points_;
  Int num_of_centers_;
  Int dim_;
  Int stage_num_;
  Int run_init_stage_;           // the stage at which a new run started
  KMFilterCenters *curr_;        // current solution
  KMFilterCentersResults best_;  // best solution so far
  KMTerminationCondition *term_;
};

#endif

IMPSTATISTICS_END_INTERNAL_NAMESPACE
#endif /* IMPSTATISTICS_INTERNAL_KM_LOCAL_SEARCH_H */
