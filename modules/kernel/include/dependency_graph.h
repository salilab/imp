/**
 *  \file IMP/dependency_graph.h
 *  \brief Build dependency graphs on models.
 *
 *  Copyright 2007-2021 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPKERNEL_DEPENDENCY_GRAPH_H
#define IMPKERNEL_DEPENDENCY_GRAPH_H

#include <IMP/kernel_config.h>
#include "base_types.h"
#include <IMP/base_macros.h>
#include "internal/utility.h"
#include <boost/graph/adjacency_list.hpp>
#include "ModelObject.h"

IMPKERNEL_BEGIN_NAMESPACE

/*
  Implementations in Model_dependency.cpp
 */

//! Directed graph on the interactions between the various objects in the model.
/** The vertices are named by the associated Object*. There
    is an edge from a to b, if a is an input to b. For example, there
    is an edge from a particle to a restraint if the restraint directly
    reads the particle.

    See
    \ref dependencies "Dependencies" for more information about dependencies.
*/
IMP_GRAPH(DependencyGraph, bidirectional, ModelObject *, int,
          internal::show_dg_node(vertex, out));

class Model;

/** The dependency graph captures the interactions between Restraint,
    ScoreState and Particle objects. The graph has a directed edge if the source
    of the edge is an input for the target of the edge or the target
    of the edge is an output for the source. eg, there
    is an edge connecting a Container to the Restraint which gets
    its particles from the Container.
    \see get_pruned_dependency_graph()
*/
IMPKERNELEXPORT DependencyGraph get_dependency_graph(Model *m);

/** The pruned dependency graph merges all particles which have the
    same dependencies to produce a simpler graph.
*/
IMPKERNELEXPORT DependencyGraph get_pruned_dependency_graph(Model *m);

IMPKERNELEXPORT ScoreStatesTemp
    get_required_score_states(ModelObject *p, const ModelObjectsTemp &all,
                              const DependencyGraph &dg,
                              const DependencyGraphVertexIndex &index);
IMPKERNELEXPORT ParticlesTemp
    get_dependent_particles(ModelObject *p, const ModelObjectsTemp &all,
                            const DependencyGraph &dg,
                            const DependencyGraphVertexIndex &index);

#ifndef IMP_DOXYGEN
//! Return all Restraints that depend on this Particle.
/*  Model::set_has_all_dependencies() must be called first.
    \note The list may contain duplicates. */
IMPKERNELEXPORT RestraintsTemp
    get_dependent_restraints(Model *m, ParticleIndex pi);

//! Return all ScoreStates that depend on this Particle.
/** \note The list may contain duplicates. */
IMPKERNELEXPORT ScoreStatesTemp
    get_dependent_score_states(Model *m, ParticleIndex pi);

//! Return all ScoreStates that are required by this Particle.
/** \note The list may contain duplicates. */
IMPKERNELEXPORT ScoreStatesTemp
    get_required_score_states(Model *m, ParticleIndex pi);
#endif

IMPKERNEL_END_NAMESPACE

#endif /* IMPKERNEL_DEPENDENCY_GRAPH_H */
