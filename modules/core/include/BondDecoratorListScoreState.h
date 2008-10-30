/**
 *  \file BondDecoratorListScoreState.h
 *  \brief Allow iteration through pairs of a set of atoms.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_DECORATOR_LIST_SCORE_STATE_H
#define IMPCORE_BOND_DECORATOR_LIST_SCORE_STATE_H

#include "core_exports.h"
#include "BondedListScoreState.h"
#include "bond_decorators.h"

#include <vector>

IMPCORE_BEGIN_NAMESPACE

// for SWIG
class BondDecorator;
class BondedDecorator;

class BondDecoratorListScoreState;
typedef Index<BondDecoratorListScoreState> BondDecoratorListIndex;

//! Keep track of particles that are connected by BondDecorator bonds.
/** We also may want to add lazy rescanning of bonds rather than doing
    it every update call and a faster lookup of bonds.
    \ingroup bond
 */
class IMPCOREEXPORT BondDecoratorListScoreState: public BondedListScoreState
{
  std::vector<BondDecorator> bonds_;
  Particles ps_;
public:
  //! Find bonds amongst the following points.
  /** \param [in] ps The set of particles to use.
   */
  BondDecoratorListScoreState(const Particles &ps= Particles());
  virtual ~BondDecoratorListScoreState(){}

  virtual void set_particles(const Particles &ps);

  virtual bool are_bonded(Particle *a, Particle *b) const;

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

  unsigned int get_number_of_bonds() const {
    return bonds_.size();
  }

protected:
  virtual void do_before_evaluate();
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_DECORATOR_LIST_SCORE_STATE_H */
