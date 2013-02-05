/**
 *  \file IMP/kernel/dependency_graph.h
 *  \brief Build dependency graphs on models.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DEPENDENCY_GRAPH_H
#define IMPKERNEL_DEPENDENCY_GRAPH_H

#include <IMP/kernel/kernel_config.h>
#include "base_types.h"
#include <IMP/base/base_macros.h>
#include <boost/graph/adjacency_list.hpp>

IMPKERNEL_BEGIN_NAMESPACE

/*
  Implementations in Model_dependency.cpp
 */

/** \brief A directed graph on the interactions between the various objects in
    the model.

    The vertices are named by the associated Object*. There
    is an edge from a to b, if a is an input to b. For example, there
    is an edge from a particle to a restraint if the restraint directly
    reads the particle.

    See
    \ref dependencies "Dependencies" for more information about dependencies.
*/
IMP_GRAPH(DependencyGraph, bidirectional, ModelObject*, int);

class Model;

/** The dependency graph captures the interactions between Restraint,
    ScoreState and Particle objects. The graph has a directed edge if the source
    of the edge is an input for the target of the edge or the target
    of the edge is an output for the source. eg, there
    is an edge connecting a Container to the Restraint which gets
    its particles from the Container.
    \see get_pruned_dependency_graph()
*/
IMPKERNELEXPORT DependencyGraph
get_dependency_graph(Model *m);


/** The pruned dependency graph merges all particles which have the
    same dependencies to produce a simpler graph.
*/
IMPKERNELEXPORT DependencyGraph
get_pruned_dependency_graph(Model *m);


/** \name Getting required values

    These functions use the dependency graph to determine all the objects
    of a given type that are needed by a particular object. An object is said
    to be needed by another if there is a path from the object to the dependent
    object through the dependency graph (see get_dependency_graph()) not passing
    through a node in all.

    @{
 */
IMPKERNELEXPORT ParticlesTemp
get_required_particles(ModelObject *p,
                       const ModelObjectsTemp &all,
                       const DependencyGraph &dg,
                       const DependencyGraphVertexIndex &index);

/** Return all the score states that depend on p as an input, even indirectly.
 */
IMPKERNELEXPORT ScoreStatesTemp
get_required_score_states(ModelObject *p,
                          const ModelObjectsTemp &all,
                          const DependencyGraph &dg,
                          const DependencyGraphVertexIndex &index);
/** @} */

/** \name Getting dependent values

    These functions use the dependency graph to determine all the objects
    of a given type that depend on a particular object. An object is said
    to depend on another if there is a path from the object to the dependent
    object through the reversed dependency graph (see get_dependency_graph()).

    @{
 */
IMPKERNELEXPORT ParticlesTemp
get_dependent_particles(ModelObject *p,
                        const ModelObjectsTemp &all,
                        const DependencyGraph &dg,
                        const DependencyGraphVertexIndex &index);


/** Return all the restraints that depend on p as an input, even indirectly.
 */
IMPKERNELEXPORT RestraintsTemp
get_dependent_restraints(ModelObject *p,
                         const ModelObjectsTemp &all,
                         const DependencyGraph &dg,
                        const DependencyGraphVertexIndex &index);

/** Return all the score states that depend on p as an input, even indirectly.
 */
IMPKERNELEXPORT ScoreStatesTemp
get_dependent_score_states(ModelObject *p,
                           const ModelObjectsTemp &all,
                           const DependencyGraph &dg,
                           const DependencyGraphVertexIndex &index);
/** @} */




/** Return the required score states for the restraints ordered in
    a valid evaluation order. Make sure you include any score states
    that are simply needed to update optimized particles.
*/
IMPKERNELEXPORT
ScoreStatesTemp get_required_score_states(const ModelObjectsTemp &irs,
                                          const DependencyGraph &dg,
                                  const DependencyGraphVertexIndex &index);


/** Assign an order to the score states in the dependency graph in which
    they can safetly be updated.*/
IMPKERNELEXPORT void set_score_state_update_order(const DependencyGraph& dg,
                                  const DependencyGraphVertexIndex &index);


IMPKERNEL_END_NAMESPACE

#endif  /* IMPKERNEL_DEPENDENCY_GRAPH_H */
