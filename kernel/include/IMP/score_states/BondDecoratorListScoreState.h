/**
 *  \file BondDecoratorListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef __IMP_BOND_DECORATOR_LIST_SCORE_STATE_H
#define __IMP_BOND_DECORATOR_LIST_SCORE_STATE_H

#include <vector>
#include "BondedListScoreState.h"
#include "../decorators/bond_decorators.h"

namespace IMP
{

class BondDecoratorListScoreState;
typedef Index<BondDecoratorListScoreState> BondDecoratorListIndex;

//! Keep track of particles that are connected by BondDecorator bonds.
/** We also may want to add lazy rescanning of bonds rather than doing
    it every update call and a faster lookup of bonds.
    \ingroup bond
 */
class IMPDLLEXPORT BondDecoratorListScoreState: public BondedListScoreState
{
  std::vector<BondDecorator> bonds_;
  Particles ps_;
public:
  //! Find bonds amongst the following points. 
  /** \param [in] ps The set of particles to use.
   */
  BondDecoratorListScoreState(const Particles &ps);
  virtual ~BondDecoratorListScoreState(){}

  virtual void set_particles(const Particles &ps);

  virtual bool are_bonded(Particle *a, Particle *b) const;

  virtual void update();

  //! This iterates through the pairs of bonded particles
  /** \note update() must be called first for this to be valid.
   */
  typedef std::vector<BondDecorator>::const_iterator BondIterator;
  BondIterator bonds_begin() const {
    return bonds_.begin();
  }
  BondIterator bonds_end() const {
    return bonds_.end();
  }
};

} // namespace IMP

#endif  /* __IMP_BOND_DECORATOR_LIST_SCORE_STATE_H */
