/**
 *  \file PairChainRestraint.h   
 *  \brief Restrain pairs of particles in chains.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_PAIR_CHAIN_RESTRAINT_H
#define __IMP_PAIR_CHAIN_RESTRAINT_H

#include "../IMP_config.h"
#include "../Restraint.h"
#include "../internal/kernel_version_info.h"
#include "../internal/ObjectPointer.h"
#include "../PairScore.h"

#include <vector>

namespace IMP
{

//! Restrain each pair of consecutive particles in each chain.
/** \ingroup restraint
 */
class IMPDLLEXPORT PairChainRestraint : public Restraint
{
public:
  //! Create the pair restraint.
  /** \param[in] pair_score Pair score to apply.
   */
  PairChainRestraint(PairScore* pair_score);
  virtual ~PairChainRestraint(){}

  IMP_RESTRAINT(internal::kernel_version_info)

  //! Add a chain of particles
  /** Each two successive particles are restrained.
   */
  void add_chain(const Particles &ps);

  //! Clear all the stored chains
  void clear_chains();

protected:
  internal::ObjectPointer<PairScore, true> ts_;
  std::vector<unsigned int> chain_splits_;
};

} // namespace IMP

#endif  /* __IMP_PAIR_CHAIN_RESTRAINT_H */
