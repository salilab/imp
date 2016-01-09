Contents
========

This reference guide shows the modules, classes and functions that make
up the %IMP API. It is targeted mainly at more advanced users and developers;
for an introduction to %IMP, see the [manual](../manual/).

To help find things we provide the following indexes:
- [Modules](namespaces.html)
- [Command line tools](@ref cmdline_tools)
- [Methods that create classes](@ref factory_index)
- [Methods that use classes](@ref argument_index)
- [Which examples each class is used in](@ref class_example_index)
- [Which examples each function is used in](@ref function_example_index)

An overview of some of the various modules currently
available. Representative classes and functions are shown. Most
modules are licensed under the
[LGPL](http://www.gnu.org/licenses/lgpl.html), however some are
licensed under the [GPL](http://www.gnu.org/copyleft/gpl.html) due to
dependencies. See the doc page for each module for details.

| Module|Representation|Scoring|Sampling|Analysis|
|-------|--------------|-------|--------|--------|
| IMP (kernel) | IMP::Particle, IMP::Model | IMP::Restraint, IMP::ScoringFunction, IMP::create_restraint() | IMP::Optimizer, IMP::Sampler | IMP::ConfigurationSet |
| IMP::algebra | IMP::algebra::VectorD, IMP::algebra::SphereD, IMP::algebra::GridD, IMP::algebra::Transformation3D... | | IMP::algebra::get_random_vector_on() |  |
|IMP::statistics | IMP::statistics::Embedding, IMP::statistics::Metric | | | IMP::statistics::create_lloyds_kmeans(), IMP::statistics::create_connectivity_clustering(), IMP::statistics::HistogramD,... |
| IMP::display | IMP::display::Geometry | | | IMP::display::PymolWriter, IMP::display::WriteOptimizerState... |
| IMP::core | IMP::core::XYZ, IMP::core::XYZR, IMP::core::RigidBody | IMP::core::DistancePairScore, IMP::core::ExcludedVolumeRestraint, IMP::core::AngleTripletScore and other scoring based on distances, angles, volume | IMP::core::MonteCarlo, IMP::core::ConjugateGradients | |
| IMP::atom | IMP::atom::Hierarchy, IMP::atom::Atom, IMP::atom::Residue, IMP::atom::Chain, IMP::atom::Bond, IMP::atom::read_pdb() | [charmm_forcefield.py](atom_2charmm_forcefield_8py-example.html) | IMP::atom::BrownianDynamics, IMP::atom::MolecularDynamics | IMP::atom::write_pdb(), IMP::atom::get_rmsd() |
|IMP::container | | IMP::container::SingletonsRestraint, IMP::container::PairsRestraint, IMP::container::ListSingletonContainer, IMP::container::ClosePairContainer, IMP::container::AllPairContainer, IMP::container::create_restraint(),...| | |
| IMP::rmf | IMP::rmf::create_restraints(), IMP::rmf::create_hierarchies(), IMP::rmf::create_geometries() | | | IMP::rmf::add_restraints(), IMP::rmf::add_hierarchies(), IMP::rmf::add_geometries() |
| IMP::em | IMP::em::DensityMap | IMP::em::FitRestraint | | |
| IMP::em2d | | IMP::em2d::Em2DRestraint | | |
| IMP::saxs | IMP::saxs::Profile | IMP::saxs::Restraint | | |
| IMP::gsl | | | IMP::gsl::Simplex, IMP::gsl::QuasiNewton | |
| IMP::domino | | | IMP::domino::DominoSampler, IMP::domino::BranchAndBoundSampler | |
| IMP::modeller | | Access to the Modeller scoring functions | | |
| IMP::isd | |IMP::isd::GaussianEMRestraint, IMP::isd::AtomicCrossLinkingMSRestraint| | |
| IMP::pmi |IMP::pmi::topology |IMP::pmi::restraints|IMP::pmi::macros::ReplicaExchange0 |IMP::pmi::analysis |


