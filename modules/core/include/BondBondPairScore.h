/**
 *  \file BondBondPairScore.h
 *  \brief Score on the distance between a pair of bonds.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_BOND_PAIR_SCORE_H
#define IMPCORE_BOND_BOND_PAIR_SCORE_H

#include "config.h"

#include <IMP/PairScore.h>
#include <IMP/algebra/Vector3D.h>
#include <IMP/UnaryFunction.h>
#include <utility>

IMPCORE_BEGIN_NAMESPACE

#ifndef IMP_NO_DEPRECATED

// exposed for testing
struct Segment {
  algebra::Vector3D first, second;
  Segment(const algebra::Vector3D &f,
          const algebra::Vector3D &s): first(f), second(s){}
};

// for testing
IMPCOREEXPORT
Segment shortest_segment(const Segment &s,
                         const algebra::Vector3D &p);

// for testing
IMPCOREEXPORT
Segment shortest_segment(const Segment &sa,
                        const Segment &sb);

//! Apply a function to the distance between two bonds.
/** A UnaryFunction is applied to the distance between the edges
    defined by two bonds. If the volume_key is non-default, then
    the bond is given a width to define a cylinder of that volume
    and the distance between the cylinders is used.
    \deprecated Use atom::BondBondPairScore instead
    \ingroup bond
    \see BondDecorator
 */
class IMPCOREEXPORT BondBondPairScore : public PairScore
{
  Pointer<UnaryFunction> f_;
  FloatKey volume_;

  Segment get_endpoints(Particle *p) const;
  Float get_offset(Particle *p, Float d) const;
  algebra::Vector3D get_difference(const Segment &s0,
                                   const Segment &s1) const;
 public:
  //! Score based on f applied to the distance
  BondBondPairScore(UnaryFunction *f,
                            FloatKey volume= FloatKey());
  virtual ~BondBondPairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

#endif // IMP_NO_DEPRECATED

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_BOND_PAIR_SCORE_H */
