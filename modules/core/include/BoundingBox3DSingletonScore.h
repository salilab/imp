/**
 *  \file IMP/core/BoundingBox3DSingletonScore.h
 *  \brief Score particles based on a bounding box
 *
 *  Copyright 2007-2022 IMP Inventors. All rights reserved.
 */

#ifndef IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H
#define IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H

#include <IMP/core/core_config.h>
#include <IMP/generic.h>
#include <IMP/SingletonScore.h>
#include <IMP/singleton_macros.h>
#include <IMP/UnaryFunction.h>
#include <IMP/algebra/BoundingBoxD.h>
#include "XYZ.h"
#include "internal/evaluate_distance_pair_score.h"
#include <boost/lambda/lambda.hpp>

IMPCORE_BEGIN_NAMESPACE

//! A generic C++ template for scoring particles based on how far outside a
//!  box they are using unary function UF.
/** 
    The bounding box score is evaluated by applying a function f of
    class UF to a particle based on the distance of its center from
    the bounding box walls.  The radius of the particle is ignored. If
    the particle is within the bounding box, the score is always
    0. Logically, it is exepcted that f(0) = 0 and f(x) > 0 if x>0.

    In Python, use the templated \ref BoundingBox3DSingletonScore class
    as in the following example. In the example, 10 particles of
    radius 1.0 are generated in a 10 A x 10 A x 10 A bounding box. The
    particles are restrained in the bounding box by applying an
    upper-bounded harmonic potential. It is upper bounded because it
    is active only outside the bounding box.

    \code{.py}
    k_bb = 10.0 # bounding box force coefficient in kcal/mol/A^2
    bb_side = 10.0 # in A
    radius = 1.0 # in A
    number
    m = IMP.Model()
    bb = IMP.algebra.BoundingBox3D(IMP.algebra.Vector3D(0, 0, 0),
                               IMP.algebra.Vector3D(bb_side, bb_side, bb_side))
    particles = []
    for i in range(0,10):
      p = IMP.Particle(m) 
      s = IMP.algebra.Sphere3D(
                IMP.algebra.get_random_vector_in(bb), radius)
      d = IMP.core.XYZR.setup_particle(p, s)
      d.set_coordinates_are_optimized(True)
      particles.append(p)
    hpb = IMP.core.HarmonicUpperBound(0, k)
    bbss = IMP.core.BoundingBox3DSingletonScore(hpb, bb) 
    bbr = IMP.container.SingletonsRestraint(bbss, particles)
    \endcode

    \see BoundingBox3DSingletonScore
    \see create_bounding_box_3d_singleton_score
 */
template <class UF>
class GenericBoundingBox3DSingletonScore : public SingletonScore {
  IMP::PointerMember<UF> f_;
  algebra::BoundingBoxD<3> bb_;

 public:
  /**
    @param f an unary function applied to particles that are outside the box. 
            f is passed the distance of the particle center from the bounding 
            box walls, ignoring the particle radius. Logically, it is exepcted 
            to satisfy f(0) = 0 and f(x) > 0 if x>0.
    @param bb the bounding box to which f is applied
   */
  GenericBoundingBox3DSingletonScore(UF *f, const algebra::BoundingBoxD<3> &bb);

  /**
    return 0 if the p is within the bounding box or f(d) if it is
    outside the bounding box, where f is the unary function provided
    during construction and d is the distance of the center of p from
    the bounding box walls.
    Update derivatives as needed, weighted using da.
  */
  virtual double evaluate_index(Model *m, ParticleIndex p,
                                DerivativeAccumulator *da) const override;
  virtual ModelObjectsTemp do_get_inputs(
      Model *m, const ParticleIndexes &pis) const override {
    return IMP::get_particles(m, pis);
  }
  IMP_SINGLETON_SCORE_METHODS(GenericBoundingBox3DSingletonScore);
  IMP_OBJECT_METHODS(GenericBoundingBox3DSingletonScore);
  ;
};

#if !defined(SWIG) && !defined(IMP_DOXYGEN)
template <class UF>
GenericBoundingBox3DSingletonScore<UF>::GenericBoundingBox3DSingletonScore(
    UF *f, const algebra::BoundingBox3D &bb)
    : f_(f), bb_(bb) {
  IMP_USAGE_CHECK(std::abs(f_->evaluate(0)) < .1,
                  "The unary function should return "
                  " 0 when passed a value of 0. Not "
                      << f_->evaluate(0));
}
template <class UF>
double GenericBoundingBox3DSingletonScore<UF>::evaluate_index(
    Model *m, ParticleIndex pi,
    DerivativeAccumulator *da) const {
  IMP_OBJECT_LOG;
  core::XYZ d(m, pi);
  algebra::Vector3D cp;
  bool outside = false;
  for (unsigned int i = 0; i < 3; ++i) {
    if (bb_.get_corner(0)[i] > d.get_coordinate(i)) {
      cp[i] = bb_.get_corner(0)[i];
      outside = true;
    } else if (bb_.get_corner(1)[i] < d.get_coordinate(i)) {
      cp[i] = bb_.get_corner(1)[i];
      outside = true;
    } else {
      cp[i] = d.get_coordinate(i);
    }
  }
  if (outside) {
    IMP_LOG_VERBOSE("Particle " << Showable(pi) << " is outside box: " << d
                                << " of " << bb_ << std::endl);
    algebra::Vector3D deriv;
    double v = internal::compute_distance_pair_score(
        d.get_coordinates() - cp, f_.get(), &deriv, boost::lambda::_1);
    if (da) {
      d.add_to_derivatives(deriv, *da);
    }
    return v;
  } else {
    return 0;
  }
}

#endif

//! Score particles based on how far outside a box they are by
//! applying f to the distance. 
//! \see GenericBoundingBox3DSingletonScore
/** 
    Example usage is provided in \ref GenericBoundingBox3DSingletonScore
*/
IMP_GENERIC_OBJECT(BoundingBox3DSingletonScore, bounding_box_3d_singleton_score,
                   UnaryFunction,
                   (UnaryFunction *f, const algebra::BoundingBoxD<3> &bb),
                   (f, bb));

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_BOUNDING_BOX_3DSINGLETON_SCORE_H */

