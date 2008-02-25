/**
 *  \file ConnectivityRestraint.h    \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_CONNECTIVITY_RESTRAINT_H
#define __IMP_CONNECTIVITY_RESTRAINT_H

#include <list>

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../ScoreFuncParams.h"
#include "DistanceRestraint.h"

namespace IMP
{
class PairScore;

//! Ensure that several sets of particles remain connected with one another.
/** The restraint takes several sets (added using the add_set method)
    and ensures that the sets of points are connected.

    More precisely, the restraint scores based on a minimum spanning
    tree on the set of particles in the union of the sets. Edges
    between two particles from the same set have weight 0. Edges
    between two nodes from different sets have weights given by the
    PairScore for that pair. The edges within a set are ignored for
    the purposes of computing derivatives.

    \ingroup restraint
 */
class IMPDLLEXPORT ConnectivityRestraint : public Restraint
{
public:
  ConnectivityRestraint(PairScore* ps);

  //! Add a new set of particles.
  void add_set(const Particles &ps);

  //! Clear all the sets and start over
  void clear_sets();

  virtual ~ConnectivityRestraint();

  IMP_RESTRAINT("0.5", "Daniel Russel")

protected:

    internal::ObjectPointer<PairScore, true> ps_;

  //! The indices for the first particle in each set
  /** set_offset_[i] is the first index of set i and set_offset_[i+1] is 
      the first index not in i. It is always defined if i is valid.
   */
  std::vector<unsigned int> set_offsets_;
};

} // namespace IMP

#endif /* __IMP_CONNECTIVITY_RESTRAINT_H */
