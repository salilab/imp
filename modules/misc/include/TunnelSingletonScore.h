/**
 *  \file TunnelSingletonScore.h
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMISC_TUNNEL_SINGLETON_SCORE_H
#define IMPMISC_TUNNEL_SINGLETON_SCORE_H

#include "config.h"
#include "internal/tunnel_traits.h"

#include <IMP/algebra/Vector3D.h>
#include <IMP/SingletonScore.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>

IMPMISC_BEGIN_NAMESPACE

//! Score particles with respect to a tunnel.
/** Particles with x,y,z coordinates and an optional radius are
    prevented from being in a volume described by a slab from
    (center[i]-height) to (center+height) on the ith coordinate with a
    tunnel of radius r centered at center. To set which coordinate
    is used use the get/set _coordinate functions.

    Note that the UnaryFunction should look like a lower bound with 0
    meaning that the particle is just touching the tunnel.
 */
class IMPMISCEXPORT TunnelSingletonScore : public SingletonScore
{
  internal::TunnelTraits tr_;
  Pointer<UnaryFunction> f_;
  FloatKey rk_;
public:
  /** */
  TunnelSingletonScore(UnaryFunction* f, FloatKey r);

  //! Set the center of the tunnel
  void set_center(algebra::Vector3D c) {
    tr_.set_center(c);
  }
  /** Set height of slab */
  void set_height(Float h) {
    tr_.set_height(h);
  }
  /** */
  void set_radius(Float h) {
    tr_.set_radius(h);
  }
  //! set the index of the coordinate of the cylinder axis
  /**
      \throw ValueException if i>=3
   */
  void set_coordinate(unsigned int i) {
    tr_.set_coordinate(i);
  }


  virtual Float evaluate(Particle *a, DerivativeAccumulator *da) const;

  virtual void show(std::ostream &out=std::cout) const;
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_TUNNEL_SINGLETON_SCORE_H */
