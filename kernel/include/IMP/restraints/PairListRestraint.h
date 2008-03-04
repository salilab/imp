/**
 *  \file PairListRestraint.h   
 *  \brief Apply a PairScore to each particle pair in a list.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PAIR_LIST_RESTRAINT_H
#define __IMP_PAIR_LIST_RESTRAINT_H

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../Particle.h"
#include "../internal/kernel_version_info.h"
#include "../internal/ObjectPointer.h"

#include <iostream>

namespace IMP
{

class PairScore;

//! Applies a PairScore to each pair of particles in a list.
/** \ingroup restraint
 */
class IMPDLLEXPORT PairListRestraint : public Restraint
{
public:
  //! Create the list restraint.
  /** \param[in] ss The function to apply to each particle.
      \param[in] ps The list of particle pairs to use in the restraints
   */
  PairListRestraint(PairScore *ss, const ParticlePairs &ps=ParticlePairs());
  virtual ~PairListRestraint();

  IMP_RESTRAINT(internal::kernel_version_info)

  void add_particle_pair(ParticlePair p);
  void clear_particle_pairs();
  void add_particle_pairs(const ParticlePairs &ps);

protected:
  internal::ObjectPointer<PairScore, true> ss_;
};

} // namespace IMP

#endif  /* __IMP_PAIR_LIST_RESTRAINT_H */
