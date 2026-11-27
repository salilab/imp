/**
 *  \file IMP/core/COMRestraint.h
 *  \brief Restrain weighted centroids of particle groups.
 *
 *  Copyright 2007-2026 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_COM_RESTRAINT_H
#define IMPCORE_COM_RESTRAINT_H

#include <IMP/core/core_config.h>
#include <IMP/Restraint.h>
#include <IMP/UnaryFunction.h>
#include <IMP/Pointer.h>
#include <IMP/algebra/Vector3D.h>
#include <boost/unordered_map.hpp>

IMPCORE_BEGIN_NAMESPACE

//! Restrain the weighted centroids (COM) of groups of particles.
/** A group is a set of particles with a weight each; its COM is the
    weighted mean of their coordinates (masses as weights make it the
    center of mass; equal weights, the centroid). Each term scores one
    geometric quantity of one or two COMs with a UnaryFunction:
    - add_point_term(): distance from the COM to a point;
    - add_plane_term(): signed distance from the COM to a plane, positive on
      the side the normal points to;
    - add_axis_term(): distance from the COM to a line (a cylinder axis);
    - add_pair_term(): distance between the COMs of two groups.

    The score is the sum of the terms. When particles move (score_moved in
    Monte Carlo) only the terms of the groups holding them are recomputed;
    a moved rigid body counts for the groups holding its members.

    Python wrappers, weighting by IMP::atom::Mass, are in
    IMP.pmi.restraints.basic (COMSlabRestraint, COMCylinderBarrier,
    COMDistanceRestraint).
 */
class IMPCOREEXPORT COMRestraint : public Restraint {
  enum TermType { POINT, PLANE, AXIS, PAIR };
  struct Term {
    TermType type;
    unsigned group1, group2;
    algebra::Vector3D point, direction;
    PointerMember<UnaryFunction> f;
  };

  std::vector<ParticleIndexes> groups_;
  std::vector<Floats> weights_;
  std::vector<Term> terms_;
  // particle (or the rigid body of a rigid member) -> terms it affects
  boost::unordered_map<ParticleIndex, Ints> affected_terms_;
  mutable Floats term_scores_;
  mutable bool cache_valid_;

  algebra::Vector3D get_centroid_of(unsigned group) const;
  double get_feature(const Term &t, algebra::Vector3D &grad1,
                     algebra::Vector3D &grad2) const;
  double evaluate_term(unsigned i, DerivativeAccumulator *accum) const;
  unsigned add_term(Term t);
  void check_group(unsigned group) const;

 public:
  COMRestraint(Model *m, std::string name = "COMRestraint%1%");

  //! Add a group of particles with a weight each; returns its index.
  /** Every particle must have XYZ coordinates. The weights (e.g. masses)
      must not be negative nor all zero; they are normalized to sum to 1. */
  unsigned add_group(const ParticleIndexes &pis, const Floats &weights);

  //! Add a group of particles, all with the same weight (the centroid).
  unsigned add_group(const ParticleIndexes &pis);

  //! Score the distance from the COM of a group to a point.
  unsigned add_point_term(unsigned group, const algebra::Vector3D &point,
                          UnaryFunction *f);

  //! Score the signed distance from the COM to the plane through point.
  /** The distance is positive on the side `normal` points to. */
  unsigned add_plane_term(unsigned group, const algebra::Vector3D &point,
                          const algebra::Vector3D &normal, UnaryFunction *f);

  //! Score the distance from the COM to the line through point along direction.
  unsigned add_axis_term(unsigned group, const algebra::Vector3D &point,
                         const algebra::Vector3D &direction, UnaryFunction *f);

  //! Score the distance between the COMs of two groups.
  unsigned add_pair_term(unsigned group1, unsigned group2, UnaryFunction *f);

  //! The current COM of a group.
  algebra::Vector3D get_center_of_mass(unsigned group) const;

  unsigned get_number_of_groups() const { return groups_.size(); }
  unsigned get_number_of_terms() const { return terms_.size(); }

  virtual double unprotected_evaluate(DerivativeAccumulator *accum) const
      override;
  virtual double unprotected_evaluate_moved(
      DerivativeAccumulator *accum, const ParticleIndexes &moved_pis,
      const ParticleIndexes &reset_pis) const override;
  virtual ModelObjectsTemp do_get_inputs() const override;
  IMP_OBJECT_METHODS(COMRestraint);
};

//! Soft lower bound at 0: log(1 + exp(-x / softness)).
/** About 0 for x well above 0, log(2) at 0, and growing linearly as
    -x / softness below. With a plane term it is a soft wall: the COM is
    pushed to the side the normal points to.
 */
class IMPCOREEXPORT SoftLowerBound : public UnaryFunction {
  double softness_;

 public:
  SoftLowerBound(double softness);

  virtual double evaluate(double feature) const override;
  virtual DerivativePair evaluate_with_derivative(double feature) const
      override;
  IMP_OBJECT_METHODS(SoftLowerBound);
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_COM_RESTRAINT_H */
