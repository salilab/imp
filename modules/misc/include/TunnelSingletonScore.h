/**
 *  \file TunnelSingletonScore.h
 *  \brief Score particles with respect to a tunnel.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef IMPMISC_TUNNEL_SINGLETON_SCORE_H
#define IMPMISC_TUNNEL_SINGLETON_SCORE_H

#include "config.h"

#include <IMP/SingletonScore.h>
#include <IMP/Vector3D.h>
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
  int coordinate_;
  Vector3D center_;
  Float height_;
  Float radius_;
  Pointer<UnaryFunction> f_;
  FloatKey rk_;
public:
  /** */
  TunnelSingletonScore(UnaryFunction* f, FloatKey r);

  //! Set the center of the tunnel
  void set_center(Vector3D c) {
    center_=c;
  }
  /** Set height of slab */
  void set_height(Float h) {
    IMP_check(h >= 0,
              "Height can't be negative",
              ValueException);
    height_=h;
  }
  /** */
  void set_radius(Float h) {
    IMP_check(h >= 0,
              "Radius can't be negative",
              ValueException);
    radius_=h;
  }
  //! set the index of the coordinate of the cylinder axis
  /**
      \throw ValueException if i>=3
   */
  void set_coordinate(unsigned int i) {
    IMP_check(i < 3,
              "Invalid coordinate value",
              ValueException);
    coordinate_=i;
  }
  //! Return the index of the coordinate of the cylinder axis
  unsigned int get_coordinate() const {
    return coordinate_;
  }

  /** */
  virtual Float evaluate(Particle *a, DerivativeAccumulator *da) const;
  /** */
  virtual void show(std::ostream &out=std::cout) const;
};

IMPMISC_END_NAMESPACE

#endif  /* IMPMISC_TUNNEL_SINGLETON_SCORE_H */
