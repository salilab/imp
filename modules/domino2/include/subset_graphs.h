/**
 *  \file domino2/subset_graphs.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2010 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO2_SUBSET_GRAPHS_H
#define IMPDOMINO2_SUBSET_GRAPHS_H

#include "particle_states.h"
#include "Subset.h"
#include <IMP/display/geometry.h>


IMPDOMINO2_BEGIN_NAMESPACE

/** A directed graph on subsets of vertices. Each vertex is
    named with an Subset.
 */
IMP_GRAPH(SubsetGraph, undirected, Subset, int);


/** An undirected graph with one vertex per particle of interest.
    Two particles are connected by an edge if a Restraint
    or ScoreState creates and interaction between the two particles.

    See \ref graphs "Graphs in IMP" for more information.
 */
IMP_GRAPH(InteractionGraph, undirected, Particle*, Object*);


//! Gets all of the Subsets of a SubsetGraph
IMPDOMINO2EXPORT Subsets get_subsets(const SubsetGraph &g);


/** Compute the exact junction tree for an interaction graph. The resulting
    graph has the junction tree properties
    - it is a tree
    - for any two vertices whose subsets both contain a vertex, that vertex
    is contained in all subsets along the path connecting those two vertices.
*/
IMPDOMINO2EXPORT SubsetGraph
get_junction_tree(const InteractionGraph &ig);




/** The restraint graph is formed by having one node per restraint
    and an edge connecting two restraints if they share input
    particles. The associated Subsets are the set of input particles
    for the restraint, projected onto ps.

    \note The graph for the optimized restraints (after temporary
    application of OptimizeContainers and OptimizeRestraints) is
    returned.
*/
IMPDOMINO2EXPORT
SubsetGraph get_restraint_graph(RestraintSet *rs,
                                const ParticleStatesTable *pst);





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
get_interaction_graph(RestraintSet *rs,
                      const ParticleStatesTable *pst);

/** Assuming that all the particles have Cartesian coordinates,
    output edges corresponding to the edges in the interaction graph.
    The edges are named by the restraint which induces them.
*/
IMPDOMINO2EXPORT display::Geometries
get_interaction_graph_geometry(const InteractionGraph &ig);


/** Display the subsets of a subset graph, superimposed on the 3D
    coordinates of the particles.
*/
IMPDOMINO2EXPORT display::Geometries
get_subset_graph_geometry(const SubsetGraph &ig);




IMPDOMINO2_END_NAMESPACE

#endif  /* IMPDOMINO2_SUBSET_GRAPHS_H */
