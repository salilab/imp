/**
 *  \file dependency_graph.h   \brief Build dependency graphs on models.
 *
 *  Copyright 2007-2012 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DEPENDENCY_GRAPH_H
#define IMPKERNEL_DEPENDENCY_GRAPH_H

#include "kernel_config.h"
#include "base_types.h"
#include <IMP/base/base_macros.h>
#include <boost/graph/adjacency_list.hpp>

IMP_BEGIN_NAMESPACE

/*
  Implementations in Model_dependency.cpp
 */

/** \brief A directed graph on the interactions between the various objects in
    the model.

    The vertices are named by the associated Object*. There
    is an edge from a to b, if a is an input to b. For example, there
    is an edge from a particle to a restraint if the restraint directly
    reads the particle.

    See \ref graphs "Graphs in IMP" for more information.
*/
IMP_GRAPH(DependencyGraph, bidirectional, base::Object*, int);

class Model;

/** The dependency graph captures the interactions between Restraint,
    ScoreState and Particle objects. The graph has a directed edge if the source
    of the edge is an input for the target of the edge or the target
    of the edge is an output for the source. eg, there
    is an edge connecting a Container to the Restraint which gets
    its particles from the Container.
    \see get_pruned_dependency_graph()
*/
IMPEXPORT DependencyGraph
get_dependency_graph(Model *m);


/** The pruned dependency graph merges all particles which have the
    same dependencies to produce a simpler graph.
*/
IMPEXPORT DependencyGraph
get_pruned_dependency_graph(Model *m);

#if 0
/** \name Getting required values

    These functions use the dependency graph to determine all the objects
    of a given type that are needed by a particular object. An object is said
    to be needed by another if there is a path from the object to the dependent
    object through the dependency graph (see get_dependency_graph()).

    @{
 */
IMPEXPORT ParticlesTemp
get_required_particles(base::Object *p,
                        const base::ObjectsTemp &all,
                        const DependencyGraph &dg);

/** Return all the score states that depend on p as an input, even indirectly.
 */
IMPEXPORT ScoreStatesTemp
get_required_score_states(base::Object *p,
                          const base::ObjectsTemp &all,
                          const DependencyGraph &dg);
/** @} */
#endif

/** \name Getting dependent values

    These functions use the dependency graph to determine all the objects
    of a given type that depend on a particular object. An object is said
    to depend on another if there is a path from the object to the dependent
    object through the reversed dependency graph (see get_dependency_graph()).

    @{
 */
IMPEXPORT ParticlesTemp
get_dependent_particles(base::Object *p,
                        const base::ObjectsTemp &all,
                        const DependencyGraph &dg);


/** Return all the restraints that depend on p as an input, even indirectly.
 */
IMPEXPORT RestraintsTemp
get_dependent_restraints(base::Object *p,
                         const base::ObjectsTemp &all,
                        const DependencyGraph &dg);

/** Return all the score states that depend on p as an input, even indirectly.
 */
IMPEXPORT ScoreStatesTemp
get_dependent_score_states(base::Object *p,
                           const base::ObjectsTemp &all,
                           const DependencyGraph &dg);
/** @} */


/** Return an ordering for the score states that is consistent with the
    dependencies show in the dependency graph.
*/
IMPEXPORT
ScoreStatesTemp get_ordered_score_states(const DependencyGraph &dg);

IMPEXPORT
ScoreStatesTemp get_required_score_states(const RestraintsTemp &irs);

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_DEPENDENCY_GRAPH_H */
