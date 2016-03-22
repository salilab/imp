\brief Python classes to represent, score, sample and analyze models.

This module contains a variety of high-level Python
classes to simplify the construction of a modeling protocol in IMP. One recent
use of PMI is the modeling of the
[Nup84 subcomplex of the nuclear pore complex](http://salilab.org/nup84/).
Several other applications can be seen at the
[IMP systems page](http://integrativemodeling.org/systems/?tag=PMI).

_This module is still under heavy development, and should be considered experimental._

See also the [PMI changelog](@ref pmi_changelog).

## Getting started with PMI
The objective of PMI is to make it as easy as possible to use the powerful representation,
scoring, and sampling tools within IMP (and to add some cool analysis functionality) for common modeling problems.
Most PMI classes wrap multiple IMP classes and functions that are commonly combined.
While this greatly improves usability, it can reduce flexibility. Let us know if you want to do something not currently supported.

The typical flow of a PMI modeling script is as follows:
 - [Topology](@ref IMP::pmi::topology): create a [System](@ref IMP::pmi::topology::System), [States](@ref IMP::pmi::topology::State), [Molecules](@ref IMP::pmi::topology::Molecule), add structure and representation
 - [DegreesOfFreedom](@ref IMP::pmi::dof): setup rigid bodies, flexible beads, and other constraints.
 - [Restraints](@ref IMP::pmi::restraints): restrain your system with theoretical or experimental data
 - [Sample](@ref IMP::pmi::macros::ReplicaExchange0) your system with replica exchange (including Monte Carlo or Molecular Dynamics or both)

Here are some examples to get you started
- Basics: [selection in PMI](https://integrativemodeling.org/nightly/doc/ref/pmi_2selection_8py-example.html), [EM modeling with densities](https://integrativemodeling.org/nightly/doc/ref/pmi_2em_8py-example.html), [ideal helices](https://integrativemodeling.org/nightly/doc/ref/pmi_2ideal_helix_8py-example.html), [symmetry](https://integrativemodeling.org/nightly/doc/ref/pmi_2symmetry_8py-example.html)
- [Multiscale modeling](https://integrativemodeling.org/nightly/doc/ref/pmi_2multiscale_8py-example.html): complete example to model a system at multiple resolutions simultaneously and sample with replica exchange
- [Crosslinks with ambiguity](https://integrativemodeling.org/nightly/doc/ref/pmi_2ambiguity_8py-example.html): complete example to create crosslinks and automatically apply them to ambiguous copies of a protein (including in multiple states)
- [MD modeling](https://integrativemodeling.org/nightly/doc/ref/pmi_2atomistic_8py-example.html): complete example to set up restraints for atomic-scale models and then run molecular dynamics

## Automating model construction
We have implemented the [BuildSystem](@ref IMP::pmi::macros::BuildSystem) macro for more easily setting up large systems. This class reads in structure data, creates representations, and sets some basic degrees of freedom.

## Multi-scale representation in PMI
One can create multiple simultaneous representations in PMI. Here is a brief overview:
 - beads: Groups of residues. The "resolution" here is the number of residues per bead (except resolution 0, which corresponds to atomic resolution). For regions with known structure, these are created by averaging along the backbone. Otherwise they are spheres with the approximately correct radius for unstructured protein.
 - densities: These are approximated electron density, in the form of [Gaussians](@ref IMP::core::Gaussian). For structured regions we fit a Gaussian Mixture Model (GMM) to the atomic positions. The key number here is residues_per_component: lower number means more approximate. Currently this representation is only used in the [GaussianEMRestraint](@ref IMP::pmi::restraints::em::GaussianEMRestraint) but we plan to use them for excluded volume, etc.
 - ideal helices: These are actually resolution=1 bead representations at the approximate locations of a helix.

See a longer discussion of resolutions [here](@ref pmi_resolution).

Check out some examples or [systems](http://integrativemodeling.org/systems/?tag=PMI) that use PMI.

_Author(s)_: Riccardo Pellarin, Charles Greenberg, Daniel Saltzberg, Peter Cimermancic, Ben Webb, Daniel Russel,  Elina Tjioe, Seung Joong Kim, Max Bonomi, Yannick Spill

_Maintainers_: Riccardo Pellarin, Charles Greenberg, Daniel Saltzberg

_License_: [LGPL](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html)
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

_Publications_:
 - See [main IMP papers list](@ref publications).
