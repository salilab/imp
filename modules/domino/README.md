                 \section background Background
                 Traditional optimizers require the data be encoded as a single (or a small number of) scoring function and then move the particles with a variety of random and downhill steps to try to find configurations of the particles which minimize the scoring function used. These sampling processes, in practice, provide no guarantees about whether the true minimal solutions. Since they depend on a scoring function, the set of solutions they do find are sensitive to the relative weights of the terms coming from different types of data, which can be difficult to choose correctly. Finally, the choice of which type of moves to take an when can make a large difference.

\section hodwdifferent How Domino is different
The Domino sampler provides a wider variety of ways in which the data about the system can be encoded as restrictions on the allowed configurations and then enumerates all configurations that meet the restrictions, given sufficient memory/time to represent/store them all. Specifically, in a Domino encoding of a problem, each particle has a discrete set of states that it can be assigned. Data that concerns only a single particle, such as that about a particles absolute location/orientation and can be encoded directly this sampling space. Other data is encoded as filters, which determine whether a particular assignment of discrete states to a subset of the particles is acceptable or not. Most simply, a filter can be a scoring function term (a Restraint) along with an associated maximum value that term is allowed to take. Other filters include ones based on a maximum score for a set of scoring function terms, that eliminate all except one of assignments that are invariant under the permutation of particles which are deemed to be equivalent (eg to only find unique volumes occupied by proteins when sequence information is not used), that eliminate assignments where multiple variables are to the same state (eg to provide cheap excluded volume). There is no need to establish weighting between different types of experimental data.

\section whyitworks Why Domino works
Domino can efficiently perform the enumeration by subdividing the particles into possibly overlapping subsets. The set of satisfying assignments for each subset is enumerated independently and then the subsets are repeatedly merged pairwise until only the set of all particles is remaining. A merge step involves first enumerating all consistent assignments of the two subsets being merged (that is, those assignments which agree on the assignment of any shared particles). Then, each such assignment is checked for consistency with all the filters which additionally constrain the newly created subset of particles. Since after each merge only the satisfying assignments for the currently subset are kept, and any satisfying assignment for a merged subset can be generated from satisfying assignments of subsets it was created by merging, at the end, exactly the set of satisfying assignments of the full particles set is generated.

\section choosingmergetree Considerations for choosing a merge tree
Each filters is used to prune the assignments as early in the process as possible, that is, as soon as all the needed particles are first found in the same subset. In general, the earlier in the subset enumeration and merging filters can be applied, the more efficient the enumeration process. We have found that a depth first traversal of the junction tree {ref something} on a sparse subset of the interactions implied by connectivity based restraints (those derived from protein structures, proteomics, cross linking etc) is a good starting point for the merge tree. The junction tree ensures that the set of particles used by each filter occurs together in at least one of the leaf sets. The junction tree also ensures that each particle only is found in a connected subtree, helping limit the creation of assignments which will later be eliminated due to incompatibility during later merges. For further efficiency, each node of the junction tree -derived merge tree is broken down into a linear tree of subsets each containing a single particle. The particles are ordered heuristically, attempting to merge the most constrained particles first.



\section spatialfilt Domino as spatial filtering
An alternate way to think about the way Domino works is to imagine eliminating parts of the space of all possible assignments based on the filters. Initially we have a n dimensional grid (n is the number of particles), which has m (the number of states for each particles, assuming, for simplicity, they all have the same number) voxels along each dimension. Each n^m voxel in the grid is a possible assignment. Applying filters to a subset of size k involves eliminating an m-k-dimensional line of voxels (orthogonal to the k-dimension subspace of the subset, passing through the eliminated assignment of in the subset space). Each time two subsets of size k are merged, a never seen before 2k-dimensional subspace of solutions is made available that can have filters applied to it. Each such state eliminated by a filter again eliminates a m-2k-dimensional line.



\section rulesofthumb Rules of Thumb
There are some good general rules of thumb for converting data into a problem suitable for a Domino sampler. First, data is only useful in the sampling process in so far as it can be used to accept or discard a configuration of a subset of the particles unconditionally. That is, simply scoring a configuration is not useful one must be able to say that a certain configuration is not worth considering. The restrictions can be relaxed in later runs to find configurations which are less good, so there is nothing to be worried about from having the filters be fine up front.

Pieces of data which concern only a single particle should be used to determine the discrete set of conformations allowed for that particle. This way, no computation has to be done during sampling for those terms.

Other data can be incorporated into the sampling process either as restraints with accompanying maximum scores (or sets of restraints with a maximum score), or more directly as filters. The latter can be more efficient, but requires more thought and so the former should be the default.

The set of data which involve only a few particles each (pair scores, connectivity etc), should be encoded as restraints and used to build the interaction graph (which is then used to build the junction tree and the merge tree). Other data that involves many particles interacting at once or all pairs of particles (eg dope scoring terms, when one can not show that most pairs of particles are never close to one another), can still be used, they just should not be used for the interaction graph.

The idea of a Table is used in several places in the module. A Table, here, is an object which, for example, knows how to create a an object which can filter the subset states of a particular subset. That is, a SubsetFilterTable knows how to create a SubsetFilter (via the SubsetFilterTable::get_subset_filter() ) which can answer queries about whether a particular SubsetState is OK (via the SubsetFilter::get_is_ok() method). This makes it easy to perform preprocessing to make the SubsetFilter::get_is_ok() calls faster. The table idea is usedf with ParticleStatesTable, SubsetStatesTable and SubsetFilterTable.


_Author(s)_: Daniel Russel, Keren Lasker

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See main IMP papers list.
