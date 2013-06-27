/**
 *  \file IMP/core/ConnectivityRestraint.h    \brief Connectivity restraint.
 *
 *  Restrict max distance between at least one pair of particles of any
 *  two distinct types.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPCORE_CONNECTIVITY_RESTRAINT_H
#define IMPCORE_CONNECTIVITY_RESTRAINT_H

#include <IMP/core/core_config.h>
#include "DistanceRestraint.h"

#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/PairScore.h>

IMPCORE_BEGIN_NAMESPACE

//! Ensure that a set of particles remains connected with one another.
/** The restraint implements ambiguous connectivity. That is, it takes
    several particles and ensures that they remain
    connected, but allows how they are connected to change. If you wish
    to restraint the connectivity of sets of
    particles (i.e. each protein is represented using a set of balls)
    use an appropriate PairScore which calls a Refiner (such
    as ClosePairsPairScore).

    \include connectivity_restraint.py

    More precisely, the restraint scores by computing the MST on the complete
    graph connecting all the particles. The edge weights are given by
    the value of the PairScore for the two endpoints of the edge.
 */
class IMPCOREEXPORT ConnectivityRestraint : public Restraint {
  IMP::base::OwnerPointer<PairScore> ps_;
  IMP::base::OwnerPointer<SingletonContainer> sc_;

 public:
  //! Use the given PairScore
  ConnectivityRestraint(PairScore *ps, SingletonContainer *sc);
  /** \deprecated_at{2.1} Use constructor that takes a container instead.
   */
  IMPCORE_DEPRECATED_METHOD_DECL(2.1)
  ConnectivityRestraint(PairScore *ps);
#ifndef IMP_DOXYGEN
  /** @name Particles to be connected

       The following methods are used to manipulate the list of particles
       that are to be connected. Each particle should have all the
       attributes expected by the PairScore used.

       Ideally, one should pass a singleton container instead. These
       can only be used if none is passed.
  */
  /*@{*/
  void add_particle(Particle *p);
  void add_particles(const ParticlesTemp &ps);
  void set_particles(const ParticlesTemp &ps);
/*@}*/
#endif

  //! Return the set of pairs which are connected by the restraint
  /** This set of pairs reflects the current configuration at the time of
      the get_connected_pairs() call, not the set at the time of the last
      evaluate() call.
  */
  ParticlePairsTemp get_connected_pairs() const;

  //! Return the pair score used for scoring
  PairScore *get_pair_score() const { return ps_; }

  Restraints do_create_current_decomposition() const;

 public:
  double unprotected_evaluate(IMP::DerivativeAccumulator *accum) const
      IMP_OVERRIDE;
  IMP::ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(ConnectivityRestraint);
  ;
};

IMPCORE_END_NAMESPACE

#endif /* IMPCORE_CONNECTIVITY_RESTRAINT_H */
