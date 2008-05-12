/**
 *  \file ChainBondedListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_CHAIN_BONDED_LIST_SCORE_STATE_H
#define __IMP_CHAIN_BONDED_LIST_SCORE_STATE_H

#include "BondedListScoreState.h"
#include "../decorators/bond_decorators.h"
#include "../internal/Vector.h"

namespace IMP
{

//! Exclude consecutive particles in a chain.
/** \ingroup bond
 */
class IMPDLLEXPORT ChainBondedListScoreState: public BondedListScoreState
{
  std::vector<internal::Vector<Particle*> > chains_;
  IntKey cik_;
  unsigned int next_index_;
public:
  //! Find bonds amongst the following points. 
  /** \param [in] ps The set of particles to use.
   */
  ChainBondedListScoreState();
  virtual ~ChainBondedListScoreState() {}

  void add_chain(const Particles &ps);

  void clear_chains();

  virtual bool are_bonded(Particle *a, Particle *b) const;

  virtual void update();
};

} // namespace IMP

#endif  /* __IMP_CHAIN_BONDED_LIST_SCORE_STATE_H */
