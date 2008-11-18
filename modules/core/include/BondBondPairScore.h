/**
 *  \file BondBondPairScore.h
 *  \brief Score on the distance between a pair of bonds.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 */

#ifndef IMPCORE_BOND_BOND_PAIR_SCORE_H
#define IMPCORE_BOND_BOND_PAIR_SCORE_H

#include "core_exports.h"

#include <IMP/PairScore.h>
#include <IMP/Vector3D.h>
#include <IMP/UnaryFunction.h>
#include <utility>

IMPCORE_BEGIN_NAMESPACE

// exposed for testing
struct Segment {
  Vector3D first, second;
  Segment(const Vector3D &f, const Vector3D &s): first(f), second(s){}
};

// for testing
IMPDLLEXPORT
Segment shortest_segment(const Segment &s,
                         const Vector3D &p);

// for testing
IMPDLLEXPORT
Segment shortest_segment(const Segment &sa,
                        const Segment &sb);

//! Apply a function to the distance between two bonds.
/** A UnaryFunction is applied to the distance between the edges
    defined by two bonds. If the volume_key is non-default, then
    the bond is given a width to define a cylinder of that volume
    and the distance between the cylinders is used.

    \ingroup bond
 */
class IMPCOREEXPORT BondBondPairScore : public PairScore
{
  Pointer<UnaryFunction> f_;
  FloatKey volume_;

  Segment get_endpoints(Particle *p) const;
  Float get_offset(Particle *p, Float d) const;
  Vector3D get_difference(const Segment &s0,
                          const Segment &s1) const;
 public:
  BondBondPairScore(UnaryFunction *f,
                            FloatKey volume= FloatKey());
  virtual ~BondBondPairScore(){}
  virtual Float evaluate(Particle *a, Particle *b,
                         DerivativeAccumulator *da) const;
  virtual void show(std::ostream &out=std::cout) const;
};

IMPCORE_END_NAMESPACE

#endif  /* IMPCORE_BOND_BOND_PAIR_SCORE_H */
