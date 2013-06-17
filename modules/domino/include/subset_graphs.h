/**
 *  \file IMP/domino/subset_graphs.h
 *  \brief A beyesian infererence-based sampler.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

#ifndef IMPDOMINO_SUBSET_GRAPHS_H
#define IMPDOMINO_SUBSET_GRAPHS_H

#include "particle_states.h"
#include "Subset.h"
#include <IMP/ScoringFunction.h>
#include <IMP/display/declare_Geometry.h>
#include <boost/graph/adjacency_list.hpp>

IMPDOMINO_BEGIN_NAMESPACE

/** An undirected graph on subsets of vertices. Each vertex is
    named with an Subset.
 */
IMP_GRAPH(SubsetGraph, undirected, Subset, int, out << vertex);

/** An undirected graph with one vertex per particle of interest.
    Two particles are connected by an edge if a Restraint
    or ScoreState creates and interaction between the two particles.

    See \ref graphs "Graphs in IMP" for more information.
 */
IMP_GRAPH(InteractionGraph, undirected, Particle *, base::Pointer<base::Object>,
          out << vertex->get_name() << "\n[" << vertex->get_index() << "]");

//! Gets all of the Subsets of a SubsetGraph
IMPDOMINOEXPORT Subsets get_subsets(const SubsetGraph &g);

/** Compute the exact
    \external{http://en.wikipedia.org/wiki/Junction_tree,junction tree}
    for an interaction graph. The resulting graph has the junction tree
    properties
    - it is a tree
    - for any two vertices whose subsets both contain a vertex, that vertex
    is contained in all subsets along the path connecting those two vertices.

    \note the junction tree is not nececessarily deterministic (you can
    get different answers if you run it different times with the same inputs).
*/
IMPDOMINOEXPORT SubsetGraph get_junction_tree(const InteractionGraph &ig);

#ifndef IMP_DOXYGEN
/** \name Debugging Junction Trees
    @{ */
IMPDOMINOEXPORT InteractionGraph get_triangulated(
    const InteractionGraph &input);

IMP_WEIGHTED_GRAPH(CliqueGraph, undirected, Subset, out << vertex);

IMPDOMINOEXPORT CliqueGraph get_clique_graph(const InteractionGraph &input);

IMPDOMINOEXPORT SubsetGraph get_minimum_spanning_tree(const CliqueGraph &input);

/** @} */
#endif

/** The restraint graph is formed by having one node per restraint
    and an edge connecting two restraints if they share input
    particles. The associated Subsets are the set of input particles
    for the restraint, projected onto ps.

    \note The graph for the optimized restraints (after temporary
    application of OptimizeContainers and OptimizeRestraints) is
    returned.
*/
IMPDOMINOEXPORT SubsetGraph get_restraint_graph(ScoringFunctionAdaptor rs,
                                                const ParticleStatesTable *pst);

/** Compute the interaction graph of the restraints and the specified
    particles.  The dependency graph in the model is traversed to
    determine how the passed particles relate to the actual particles
    read as input by the model. For example, if particles contains a
    rigid body, then an restraint which uses a member of the rigid
    body will have an edge from the rigid body particle.

    \note You may want to create an OptimizeRestraints object before
    calling this function.

    \note These functions are here to aid in debugging of optimization
    protocols that use Domino2. As a result, its signature and
    functionality may change without notice.
    @{
 */
IMPDOMINOEXPORT InteractionGraph get_interaction_graph(
    ScoringFunctionAdaptor rs, const ParticleStatesTable *pst);

IMPDOMINOEXPORT InteractionGraph get_interaction_graph(
    ScoringFunctionAdaptor rs, const ParticlesTemp &pst);

/** Assuming that all the particles have Cartesian coordinates,
    output edges corresponding to the edges in the interaction graph.
    The edges are named by the restraint which induces them.
*/
IMPDOMINOEXPORT display::Geometries get_interaction_graph_geometry(
    const InteractionGraph &ig);

/** Display the subsets of a subset graph, superimposed on the 3D
    coordinates of the particles.
*/
IMPDOMINOEXPORT display::Geometries get_subset_graph_geometry(
    const SubsetGraph &ig);

/** A directed graph on subsets of vertices. Each vertex is
    named with an Subset.
 */
IMP_GRAPH(MergeTree, bidirectional, Subset, int, out << vertex);

/** \see get_merge_tree(const SubsetGraph&)

    Compute the merge tree for a given restraint set and
    particle states table. An OptimizeRestraints object is
    created to improve the InteractionGraph used internally.
 */
IMPDOMINOEXPORT MergeTree get_merge_tree(ScoringFunctionAdaptor input,
                                         const ParticleStatesTable *pst);

/** \see get_merge_tree(RestraintSet*,const ParticleStatesTable*)

    Compute the merge tree from a junction tree.

    \note the merge tree is not nececessarily deterministic (you can
    get different answers if you run it different times with the same inputs).
 */
IMPDOMINOEXPORT MergeTree get_merge_tree(const SubsetGraph &junction_tree);

/** \see get_merge_tree(RestraintSet*,const ParticleStatesTable*)

    Compute the merge tree from a junction tree, attempting to keep it fairly
    balanced.

    \note the merge tree is not nececessarily deterministic (you can
    get different answers if you run it different times with the same inputs).
 */
IMPDOMINOEXPORT MergeTree get_balanced_merge_tree(
    const SubsetGraph &junction_tree);

IMPDOMINOEXPORT bool get_is_merge_tree(const MergeTree &tree, Subset all,
                                       bool verbose = true);

/** \name Read and write merge trees
    Merge trees can be written to an read from a text stream. As always,
    this requires a canonical particles list whose order is the same
    for the reading and writing.
    @{
*/

IMPDOMINOEXPORT void write_merge_tree(const MergeTree &tree,
                                      const ParticlesTemp &ps,
                                      std::ostream &out);
IMPDOMINOEXPORT MergeTree read_merge_tree(std::istream &input,
                                          const ParticlesTemp &ps);
/** @} */

IMPDOMINO_END_NAMESPACE

#endif /* IMPDOMINO_SUBSET_GRAPHS_H */
