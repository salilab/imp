Welcome to the IMP library documentation.
 Throughout the documentation we refer to two, overlapping sets of people.
- _users_ write python scripts to use existing IMP functionality
- _developers_ develop new methods for scoring, sampling or representation,
   and hopefully contribute them back to IMP central
 Various pieces of documentation are marked as being intended for developers.

In addition to the tabs above that take you to individual module, class and function documentation, we have the following pages to help users get started:
- [Installing IMP](@ref installation)
- [Introduction](@ref introduction)
- [Tutorial](../tutorial/index.html)
- [Change history](@ref ChangeLog)
- [Modules and Applications](@ref allmod)
- [Biological systems modeled in IMP](http://integrativemodeling.org/systems/)
- [Wiki](https://github.com/salilab/imp/wiki)
- [Mailing lists](http://integrativemodeling.org/contact.html)

To help find things we provide the following indexes:
- [Methods that create classes](@ref factory_index)
- [Methods that use classes](@ref argument_index)
- [Which examples each class is used in](@ref class_example_index)
- [Which examples each function is used in](@ref function_example_index)

Additional developer documentation:
- [Developers' guide](@ref devguide)
- [Bug Tracker](https://github.com/salilab/imp/issues)
- [Nightly tests](http://integrativemodeling.org/nightly/results/)

An overview of some of the various modules currently
available. Representative classes and functions are shown. Most
modules are licensed under the
[LGPL](http://www.gnu.org/licenses/lgpl.html), however some are
licensed under the [GPL](http://www.gnu.org/copyleft/gpl.html) due to
dependencies. See the doc page for each module for details.

| Module|Representation|Scoring|Sampling|Analysis|
|-------|--------------|-------|--------|--------|
| IMP::kernel | IMP::kernel::Particle, IMP::kernel::Model | IMP::kernel::Restraint, IMP::kernel::ScoringFunction, IMP::kernel::create_restraint() | IMP::kernel::Optimizer, IMP::kernel::Sampler | IMP::kernel::ConfigurationSet |
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
