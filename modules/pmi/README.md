\brief Python classes to represent, score, sample and analyze models.

This module contains a variety of high-level Python
classes to simplify the construction of a modeling protocol in IMP. One recent
use of PMI is the modeling of the
[Nup84 subcomplex of the nuclear pore complex](http://salilab.org/nup84/).
Several other applications can be seen at the
[IMP systems page](http://integrativemodeling.org/systems/?tag=PMI).

_This module is still under heavy development, and should be considered experimental._

See also the [PMI changelog](@ref pmi_changelog).

# Getting started with PMI
The objective of PMI is to make it as easy as possible to use the powerful representation,
scoring, and sampling tools within IMP (and to add some cool analysis functionality) for common modeling problems.
Most PMI classes wrap multiple IMP classes and functions that are commonly combined.
While this greatly improves usability, it can reduce flexibility. Let us know if you want to do something not currently supported.

The typical flow of a PMI modeling script is as follows.
* [Topology](@ref IMP::pmi::topology): create a System, States, Molecules, add structure and representation
* [DegreesOfFreedom](@ref IMP::pmi::dof): setup rigid bodies, flexible beads, and other constraints.
* [Restraints](@ref IMP::pmi::restraints): restrain your system with theoretical or experimental data
* [Sample](@ref IMP::pmi::macros::ReplicaExchange0) your system with replica exchange (including Monte Carlo or Molecular Dynamics or both)

# Automating model construction
We have implemented the TopologyReader(@ref IMP::pmi::topology::TopologyReader) for automatically doing the first two bullet points above:
reading in structure data, creating representations, and setting some basic degrees of freedom.
See the [PMI tutorial](@ref rnapolii_stalk) for a complete explanation of how to use this class.

# Resolution in PMI
Check out some examples or [systems](http://integrativemodeling.org/systems/?tag=PMI) that use PMI.

_Author(s)_: Riccardo Pellarin, Charles Greenberg, Daniel Saltzberg, Peter Cimermancic,  Ben Webb, Daniel Russel,  Elina Tjioe, Seung Joong Kim, Max Bonomi, Yannick Spill

_Maintainer_: Riccardo Pellarin

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See [main IMP papers list](@ref publications).
