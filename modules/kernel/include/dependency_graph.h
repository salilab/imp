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

#ifndef IMP_DOXYGEN
IMPEXPORT DependencyGraph
get_dependency_graph(const RestraintsTemp &rs);

/** \copydoc get_dependency_graph(const RestraintsTemp&)*/
IMPEXPORT DependencyGraph
get_dependency_graph(const ScoreStatesTemp &ss,
                     const RestraintsTemp &rs);
#endif
class Model;

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
get_dependency_graph(Model *m);


/** The pruned dependency graph merges all particles which have the
    same dependencies to produce a simpler graph.
*/
IMPEXPORT DependencyGraph
get_pruned_dependency_graph(Model *m);

/** Get the score states required by the passed restraints.*/
IMPEXPORT ScoreStatesTemp
get_required_score_states(const RestraintsTemp &rs);


/** Returns the subset of particles that depend on p as input. This
    will not include p. The variable all is the set of all other particles
    of interest, they will block dependency propagation.

    \note This function is here to aid in debugging of optimization
    protocols that use Domino. As a result, its signature and
    functionality may change without notice.
 */
IMPEXPORT ParticlesTemp get_dependent_particles(Particle *p,
                                                const ParticlesTemp &all);

/** \copydoc get_dependent_particles(Particle*,const ParticlesTemp&)
 */
IMPEXPORT ParticlesTemp
get_dependent_particles(Particle *p,
                        const ParticlesTemp &all,
                        const DependencyGraph &dg);


/** Return all the restraints that depend on p as an input, even indirectly.
 */
IMPEXPORT RestraintsTemp
get_dependent_restraints(Particle *p,
                        const ParticlesTemp &all,
                        const DependencyGraph &dg);

/** Return all the score states that depend on p as an input, even indirectly.
 */
IMPEXPORT ScoreStatesTemp
get_dependent_score_states(Particle *p,
                        const ParticlesTemp &all,
                        const DependencyGraph &dg);


/** Return an ordering for the score states that is consistent with the
    dependencies show in the dependency graph.
*/
IMPEXPORT
ScoreStatesTemp get_ordered_score_states(const DependencyGraph &dg);

#if !defined(IMP_DOXYGEN) && !defined(SWIG)
namespace {
 typedef boost::graph_traits<DependencyGraph> MDGTraits;
  typedef MDGTraits::vertex_descriptor MDGVertex;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::type MDGVertexMap;
  typedef boost::property_map<DependencyGraph,
                              boost::vertex_name_t>::const_type
  MDGConstVertexMap;
}
#endif

IMP_END_NAMESPACE

#endif  /* IMPKERNEL_DEPENDENCY_GRAPH_H */
