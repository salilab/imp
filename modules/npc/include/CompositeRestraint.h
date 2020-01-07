/**
 *  \file IMP/npc/CompositeRestraint.h    \brief Composite restraint.
 *
 *  Copyright 2007-2020 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPNPC_COMPOSITE_RESTRAINT_H
#define IMPNPC_COMPOSITE_RESTRAINT_H

#include <IMP/npc/npc_config.h>

#include <IMP/SingletonContainer.h>
#include <IMP/Restraint.h>
#include <IMP/PairScore.h>

IMPNPC_BEGIN_NAMESPACE

//! Score a set of particles that form a composite
/** Similarly to IMP::core::ConnectivityRestraint, this ensures that a set
    of Particles remains connected, but allows how they are connected to change
    (for example, A-B-C would score as well as B-A-C). However, unlike that
    restraint, it acts on Particle type (in the case where there are several
    distinct Particles of the same type) and allows the stoichiometry to change
    (For example, A1-B1-C1 would score similarly to A1-B2-C2 or A1-B1-B2-C1).

    To use, first create the restraint and then call add_type() for each
    distinct protein type, e.g. to enforce the composite ABC where there are
    two copies of protein A and B and a single copy of protein C use:
    \begin{code}
      add_type([A1, A2])
      add_type([B1, B2])
      add_type([C1])
    \end{code}

    The restraint scores by computing the complete graph connecting all the
    particles (where the particles are the vertices and the edges are the
    value of the PairScore), then calculating the minimum spanning tree, and
    then using the lowest scoring subgraph that includes at least one of each
    protein type. This is made much more efficient if the restraint's maximum
    score is set (set_maximum_score()).

    \see IMP::core::ConnectivityRestraint
    \see IMP::core::MSConnectivityRestraint
 */
class IMPNPCEXPORT CompositeRestraint : public Restraint {
  typedef std::pair<int, ParticleIndex> TypedParticle;
  PointerMember<PairScore> ps_;
  std::vector<TypedParticle> tps_;
  int num_particle_types_;

 public:
  //! Create with the given PairScore
  CompositeRestraint(Model *m, PairScore *ps)
     : Restraint(m, "CompositeRestraint %1%"), ps_(ps),
       num_particle_types_(0) {}

  //! Add all Particles of a single type
  /** The restraint will ensure that at least one Particle of each type
      is present in the composite. */
  void add_type(ParticleIndexes ps) {
    for (ParticleIndexes::const_iterator it = ps.begin(); it != ps.end();
         ++it) {
      tps_.push_back(TypedParticle(num_particle_types_, *it));
    }
    ++num_particle_types_;
  }

  //! Return the set of pairs which are connected by the restraint
  /** This set of pairs reflects the current configuration at the time of
      the get_connected_pairs() call, not the set at the time of the last
      evaluate() call.
  */
  ParticleIndexPairs get_connected_pairs() const;

  double unprotected_evaluate(DerivativeAccumulator *accum) const IMP_OVERRIDE;
  ModelObjectsTemp do_get_inputs() const IMP_OVERRIDE;
  IMP_OBJECT_METHODS(CompositeRestraint);
};

IMPNPC_END_NAMESPACE

#endif /* IMPNPC_COMPOSITE_RESTRAINT_H */
