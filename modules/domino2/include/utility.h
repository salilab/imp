/**
 *  \file domino2/utility.h
 *  \brief Functions to get report statistics about the used attributes.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 */

#ifndef IMPDOMINO2_UTILITY_H
#define IMPDOMINO2_UTILITY_H

#include "domino2_config.h"
#include "SubsetState.h"
#include "Subset.h"
#include <IMP/Particle.h>
#include <IMP/SingletonContainer.h>
#include <IMP/RestraintSet.h>
#include <IMP/Model.h>
#include <IMP/core/internal/CoreClosePairContainer.h>

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

IMP_BEGIN_NAMESPACE
class Model;
class Particle;
class Object;
IMP_END_NAMESPACE

IMPDOMINO2_BEGIN_NAMESPACE
/** \name Debug tools

    We provide a number of different functions for helpering
    optimize and understand domino-based sampling. These functions
    are expose part of the implementation and are liable to change
    without notice.
    @{
 */
/** An undirected graph with one vertex per particle of interest.
    Two particles are connected by an edge if a Restraint
    or ScoreState creates and interaction between the two particles.

    See \ref graphs "Graphs in IMP" for more information.
 */
IMP_GRAPH(InteractionGraph, undirected, Particle*, Object*);

/** Compute the interaction graph of the restraints and the specified
    particles.  The dependency graph in the model is traversed to
    determine how the passed particles relate to the actual particles
    read as input by the model. For example, if particles contains a
    rigid body, then an restraint which uses a member of the rigid
    body will have an edge from the rigid body particle.

    \note This function is here to aid in debugging of optimization
    protocols that use Domino2. As a result, its signature and
    functionality may change without notice.
    @{
 */
IMPDOMINO2EXPORT InteractionGraph
get_interaction_graph(Model *m,
                      const ParticlesTemp &particles);

/** Returns the subset of particles that depend on p as input. This
    will include p.

    \note This function is here to aid in debugging of optimization
    protocols that use Domino2. As a result, its signature and
    functionality may change without notice.
 */
IMPDOMINO2EXPORT ParticlesTemp get_dependent_particles(Particle *p);

/** \copydoc{get_dependent_particles(Particle*)}
 */
IMPDOMINO2EXPORT ParticlesTemp
get_dependent_particles(Particle *p,
                        const DependencyGraph &dg);


/** A directed graph on subsets of vertices. Each vertex is
    named with an Subset.
 */
IMP_GRAPH(SubsetGraph, undirected, Subset, int);


/** Compute the exact junction tree for an interaction graph. The resulting
    graph has the junction tree properties
    - it is a tree
    - for any two vertices whose subsets both contain a vertex, that vertex
    is contained in all subsets along the path connecting those two vertices.
*/
IMPDOMINO2EXPORT SubsetGraph
get_junction_tree(const InteractionGraph &ig);


class ParticleStatesTable;
/** A static container is one such as a container::ListSingletonContainer, whose
    contents will not change as optimized particles are moved around. More
    technically, it is one which either is not written by any score state or,
    any score state which writes it does not depend on optimized particles.
 */
IMPDOMINO2EXPORT bool
get_is_static_container(Container *c,
                        const DependencyGraph &dg,
                        const ParticleStatesTable *pst);

/** The class temporarily transforms the restraints,
    in a model to make it better suited for the
    DominoSampler. Transformations include
    - replacing certain container::PairsRestraint and
      container::SingletonRestraint objects by a set a set of
      core::PairRestraint/core::SingletonRestraint objects
    \throw ModelException if the model contains any non-static
    containers other than container::ClosePairContainer and its
    ilk. Examples include container::ConnectedPairContainer.
*/
class IMPDOMINO2EXPORT OptimizeRestraints: public RAII {
  Restraints removed_;
  RestraintSets removed_parents_;
  Restraints added_;
  RestraintSets added_parents_;
  Pointer<Model> m_;

  void optimize_model(Model *m, const ParticlesTemp &particles);
  void unoptimize_model();
public:
  IMP_RAII(OptimizeRestraints, (Model *m, const ParticlesTemp &particles), {},
           {
             optimize_model(m, particles);
           },
           {
             unoptimize_model();
           });
};

/** The class temporarily transforms the
    containers in a model to make it better suited for the
    DominoSampler. Transformations include
    - replacing container::ClosePairContainer and
      core::ExcludedVolumeRestraint by a static interaction list based
      on the ParticleStatesTable.
    \throw ModelException if the model contains any non-static
    containers other than container::ClosePairContainer and its
    ilk. Examples include container::ConnectedPairContainer.
*/
class IMPDOMINO2EXPORT OptimizeContainers: public RAII {
  core::internal::CoreClosePairContainers staticed_;

  void optimize_model(Model *m, const ParticleStatesTable *pst);
  void unoptimize_model();
public:
  IMP_RAII(OptimizeContainers, (Model *m, const ParticleStatesTable *pst), {},
           {
             optimize_model(m, pst);
           },
           {
             unoptimize_model();
           });
};
/**@} */
IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_UTILITY_H */
