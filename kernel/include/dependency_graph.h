/**
 *  \file dependency_graphs.h   \brief Build dependency graphs on models.
 *
 *  Copyright 2007-2011 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMP_DEPENDENCY_GRAPH_H
#define IMP_DEPENDENCY_GRAPH_H

#include "kernel_config.h"
#include "Model.h"
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
IMP_GRAPH(DependencyGraph, bidirectional, Object*, int);

/** The dependency graph captures the interactions between Restraint,
    ScoreState and Particle objects. The graph has an edge if the source
    of the edge is an input for the target of the edge. eg, there
    is an edge connecting a Container to the Restraint which gets
    its particles from the Container. In order for a given
    Restraint to be evaluated properly, all of the Particles connected
    by a path to the Restraint must be up to date.
    \see get_pruned_dependency_graph()
*/
IMPEXPORT DependencyGraph
get_dependency_graph(const RestraintsTemp &rs);

/** The pruned dependency graph merges all particles which have the
    same dependencies to produce a simpler graph.
*/
IMPEXPORT DependencyGraph
get_pruned_dependency_graph(const RestraintsTemp &rs);


/** Get the score states required by the passed restraints.*/
IMPEXPORT ScoreStatesTemp
get_required_score_states(const RestraintsTemp &rs);

#ifndef IMP_DOXYGEN
class Container;
/** Return true if the contents of the container doesn't depend on any
    of the passed particles.
*/
IMPEXPORT bool
get_is_static_container(Container *c,
                        const DependencyGraph &dg,
                        const ParticlesTemp &pst);
#endif

IMP_END_NAMESPACE

#endif  /* IMP_DEPENDENCY_GRAPH_H */
