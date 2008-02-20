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

#include <iostream>

namespace IMP
{

class PairScore;

//! Applies a PairScore to each Particle in a list.
/** \ingroup restraint
 */
class IMPDLLEXPORT PairListRestraint : public Restraint
{
public:
  //! Create the list restraint.
  /** \param[in] ps The list of particle pairs to use in the restraint.
      \param[in] ss The function to apply to each particle.
   */
  PairListRestraint(const ParticlePairs &ps, PairScore *ss);
  virtual ~PairListRestraint();

  IMP_RESTRAINT("0.5", "Daniel Russel");

  void add_particle_pair(ParticlePair p);
  void clear_particle_pairs();
  void add_particle_pairs(const ParticlePairs &ps);

protected:
  std::auto_ptr<PairScore> ss_;
};

} // namespace IMP

#endif  /* __IMP_PAIR_LIST_RESTRAINT_H */
