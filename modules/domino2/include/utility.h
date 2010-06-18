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

#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/adjacency_list.hpp>

IMP_BEGIN_NAMESPACE
class Model;
class Particle;
class Object;
IMP_END_NAMESPACE

IMPDOMINO2_BEGIN_NAMESPACE


/** \name Interaction Graph

    Compute the interaction graph of the restraints and the specified
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
#ifndef SWIG
typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                              boost::property<boost::vertex_name_t, Particle*>,
                              boost::property<boost::edge_name_t,
                                              Object*> > InteractionGraph;
#else
class InteractionGraph;
#endif

IMPDOMINO2EXPORT InteractionGraph
get_interaction_graph(Model *m,
                      const ParticlesTemp &particles);

/** @} */



/** \name Dependent particles

    Returns the subset of particles that depend on p as input. This
    will include p.

    \note This function is here to aid in debugging of optimization
    protocols that use Domino2. As a result, its signature and
    functionality may change without notice.
    @{
 */
IMPDOMINO2EXPORT ParticlesTemp get_dependent_particles(Particle *p);

IMPDOMINO2EXPORT ParticlesTemp
get_dependent_particles(Particle *p,
                        const Model::DependencyGraph &dg);
/** @} */


/** \name Junction Tree
    Compute the exact junction tree for an interaction graph.
    @{
*/
#ifndef SWIG
typedef boost::adjacency_list<boost::vecS, boost::vecS,
                              boost::undirectedS,
                              boost::property<boost::vertex_name_t,
                                     Subset > > SubsetGraph;
#else
class SubsetGraph;
#endif
IMPDOMINO2EXPORT SubsetGraph
get_junction_tree(const InteractionGraph &ig);
/** @} */

/** A static container is one such as a container::ListSingletonContainer, whose
    contents will not change as optimized particles are moved around. More
    technically, it is one which either is not written by any score state or,
    any score state which writes it does not depend on optimized particles.

    This function is here for debugging purposes and so may change without
    notice.
 */
IMPDOMINO2EXPORT bool
get_is_static_container(Container *c,
                        const Model::DependencyGraph &dg,
                        const ParticlesTemp &optimized_particles);
#if 0
class ParticleStatesTable;
/** The function transforms the restraints, score states and
    containers in a model to make it better suited for the
    DominoSampler. Transformations include
    - replacing certain container::PairsRestraint and
      container::SingletonRestraint objects by a set a set of
      core::PairRestraint/core::SingletonRestraint objects
    - replacing container::ClosePairContainer and
      core::ExcludedVolumeRestraint by a static interaction list based
      on the ParticleStatesTable.
    \throw ModelException if the model contains any non-static
    containers other than container::ClosePairContainer and its
    ilk. Examples include container::ConnectedPairContainer.
*/
IMPDOMINO2EXPORT void optimize_model(Model *m,
                                     ParticleStatesTable *pst);
#endif
IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_UTILITY_H */
