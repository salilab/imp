Overview {#library_overview}
========

[TOC]

The %IMP library provides tools to implement the computational parts of
the [integrative modeling iterative process](@ref procedure) (steps 2-4).
%IMP is primarily implemented in C++ for speed; however, each of the classes
is wrapped so that it can also be used from Python. A protocol can thus
be developed from scratch by simply writing a Python script.
The examples below will use Python scripts.

# Modular structure of IMP {#overview_modular}

Functionality in %IMP is grouped into modules, each with its own
namespace (in C++) or package (in Python). For %example, the functionality
for IMP::core can be found like

    IMP::core::XYZ(p)

in C++ and

    IMP.core.XYZ(p)

in Python.

A module contains classes, methods and data which are related.
The IMP::kernel module (equivalently, the top-level IMP namespace) is a small
collection of classes that define the storage of information about the system
and the main interfaces used to interact with that information. However, in
most cases the kernel does not provide actual implementations of these classes;
these are provided in other modules.
For example, it merely defines a [Restraint](@ref IMP::Restraint)
as any object that, given a set of particles, returns a score, and an
[Optimizer](@ref IMP::Optimizer) as an object that changes the
attributes of all particles to yield an optimized score over all restraints.
It is the [core module](@ref IMP::core) that provides, for example, a
[concrete Restraint](@ref IMP::core::DistanceRestraint) that acts like a
harmonic 'spring' between two point-like particles, an Optimizer that
[utilizes the conjugate gradients minimization method](@ref IMP::core::ConjugateGradients),
and much other functionality.

Other %IMP modules provide the basic building blocks needed to construct
a protocol, such as the [atom module](@ref IMP::atom) that
provides atom-like particles, a molecular dynamics optimizer, etc.
Other modules provide support for specific types of experimental data
or specialized optimizers, such as the [em module](@ref IMP::em) that
supports electron microscopy data, and the [domino module](@ref IMP::domino)
that provides an inference-based divide-and-conquer optimizer.
[Many other modules are available](../ref/namespaces.html)
in this version of %IMP.

%IMP is designed so that it is easy to add a new module; for example,
a developer working on incorporating data from a new experimental
technique may add a new %IMP module that translates the data from this
technique into spatial restraints.

# Representation: IMP::Model {#overview_representation}

In IMP, the system is represented by the IMP::Model class, which stores
a collection of "particles", each of which is a
flexible and abstract data container, able to hold whatever information is
necessary to represent the system. For example, a given particle may be
assigned x, y, and z attributes to store point coordinates, another may be
assigned x, y, z, and a radius to represent a sphere, and another may
contain two pointers to other particles to represent a bond or another
relationship.

In IMP, particle attributes can be numbers, strings, or lists of other
particles, among other things. Each particle is identified by an index
(IMP::ParticleIndex) and has an associated name, in order to make
it easier to understand. Finally, attributes are identified by keys
(e.g. IMP::StringKey for string attributes). The key identifies one type of
data that may be contained in many particles.

At the most basic, to create particles and manipulate attributes you can do

    import IMP
    model= IMP.Model()
    particle_0= model.add_particle("my first particle")
    string_key = IMP.StringKey("my first data")
    model.add_attribute(string_key, particle_0, "Hi, particle 0")

    particle_1= model.add_particle("my second particle")
    model.add_attribute(string_key, particle_1, "Hi, particle 1")

    print(model.get_attribute(string_key, particle_0))

Certain of the attributes can be marked as parameters of the model. These
are attributes that you want to sample or optimize. To do so you can do
   model.set_is_optimized(float_key, particle_0)

\note A lot of %IMP uses IMP::Particle objects instead of IMP::ParticleIndex objects to identify particles. They should be treated as roughly the same. To map from an index to a particle you use IMP::Model::get_particle() and to go the other way IMP::Particle::get_index(). Using the indexes is preferred. When doing it on lists, you can use IMP::get_indexes() and IMP::get_particles().

# Decorators {#overview_decorators}

Accessing all your data at such a low level can get tiresome, so we provide
decorators to make it easier. Each type of decorator provides an interface
to manipulate a particular type of data. For example, an IMP.atom.Residue
decorator provides access to residue associated information (e.g. the index
of the residue, or its type) in particles that have it.

    residue= IMP.atom.Residue(model, my_residue)
    print(residue.get_residue_type())

Multiple decorators can be applied to a single particle, if appropriate;
for example, an atom-like particle could be treated like a
[point](@ref IMP::core::XYZ), a [sphere](@ref IMP::core::XYZR),
an [electrically charged particle](@ref IMP::atom::Charged),
or an [atom](@ref IMP::atom::Atom).

Decorators provide a standard interface to add their data to a particle,
decorate a particle that already has the needed data or check if a particle
is appropriate to have the decorator used with it.

    # add coordinates to a particle
    decorated_particle = IMP.core.XYZ.setup_particle(model, my_particle,
                                                IMP.algebra.Vector3D(1,2,3))
    print(decorated_particle.get_coordinates())

    # my_other_particle has already been set up, so we can just decorate
    # it directly
    another_decorated_particle = IMP.core.XYZ(model, my_other_particle)
    print(another_decorated_particle.get_coordinates())

    # we can change the coordinates too
    another_decorated_particle.set_coordinates(IMP.algebra.Vector3D(5,4,3))

(Vector3D is a simple %IMP class that represents a 3D vector, or point.
The IMP::algebra module contains many such general purpose algebraic and
geometric methods and classes.)

Decorators can also be used to create relationships between particles.
For example, rigid bodies are implemented using the IMP::core::RigidBody
decorator. Each rigid body has a collection of other particles that move
along with it, each of which is decorated with the
IMP::core::RigidMember decorator.

# Representing biological molecules {#overview_biomolecules}

Biological modules are represented hierarchically in IMP using the IMP::atom::Hierarchy. These hierarchies follow the natural hierarchical nature of most biomolecules. A protein from a PDB would be a hierarchy with a root for the whole PDB file with a child per chain. Each chain particle has a child for each residue in the chain, and each residue has a child for each atom. Each particle has various types of associated data. For example an atom has data using the IMP::atom::Atom, IMP::core::XYZR, IMP::atom::Mass and IMP::atom::Hierarchy decorators.

The structures represented do not have to be atomic and can be multi-resolution -  that is, they can have coordinates at any level of the hierarchy. The invariants are that the leaves must have coordinates, radii and mass. Pieces of the hierarchy can be picked out using IMP::atom::Selection using the standard sorts of biological criteria:

    # Select residues 10 through 49.
    my_residues= IMP.atom.Selection(my_pdb, residue_indexes=range(10,50)).get_particles()


# Containers {#overview_containers}

You can manipulate and maintain collections of particles using IMP::Container. A collection can be anything from a list of particles gathered manually, to all pairs of particles from some list that are closer than a certain distance to one another. For example, to maintain a list of all close pairs of particles you can do

    # all particle pairs closer than 3A
    # it is always good to give things names; that is what the last argument does
    close_pairs= IMP.container.ClosePairContainer(all_my_particles, 3, "My close pairs")

These containers can then be used to create scoring functions or analyze the data.

# Constraints and Invariants {#overview_constraints}

Many things such as rigid bodies and lists of all close pairs depend on maintaining some property as the model changes. These properties are maintained by IMP::Constraint objects. Since the invariants may depend on things that are reasonably expensive to compute, these invariants are updated only when requested. This means that if you change the coordinates of some particles, the contents of the close pairs list might be incorrect until it is updated. The required update can be triggered implicitly, for example when some scoring function needs it, or explicitly, when IMP::Model::update() is called.

Behind the scenes, IMP maintains an IMP::DependencyGraph that tracks how information flows between the particles and the containers, based on the constraints. It is used to detect, for example, that a particular particle is part of a rigid body, and so if its coordinates are needed for scoring, the rigid body must be brought up to date and the appropriate constraint must be asked to update the member particle's coordinates. In order to be able to track this information, relevant objects (IMP::ModelObject) have methods IMP::ModelObject::get_inputs() and IMP::ModelObject::get_outputs() that return the things that are read and written respectively.

# Scoring {#overview_scoring}

One then needs to be able to evaluate how well the current configuration of the model fits this data that one is using to model. In addition to scores, when requested derivatives of the total score as a function of each parameter can be computed.

## Restraints {#overview_restraints}

An IMP::Restraint computes a score on some set of particles. For example, a restraint be used to penalize configurations of the model that have collisions

    # penalize collisions with a spring constant of 10 kcal/mol/A
    soft_sphere_pair_score= IMP.core.SoftSpherePairScore(10)
    my_excluded_volume_restraint= IMP.container.PairsRestraint(soft_sphere_pair_score,
                                                              close_pairs,
                                                              "excluded volume")

\note Many restraints (including this one) are made more flexible by delegating
part of their work to the IMP::PairScore and/or IMP::UnaryFunction classes.
See the [example script](@ref library_example) for an %example.

To get the score of an individual restraint, you can use its IMP::Restraint::get_score() method.

## Scoring functions {#overview_scoring_functions}

Scoring in %IMP is done by creating an IMP::ScoringFunction. A scoring function
is simply the sum of its terms (restraints). You can create many different
scoring functions for different purposes and each restraint can be part
of multiple scoring functions.

        my_scoring_function= IMP.core.RestraintsScoringFunction([my_excluded_volume_restraint],
                                                                "score excluded volume")

\note You will see old example code that, instead of creating an IMP::ScoringFunction, adds the restraints to the model. This creates an implicit scoring function consisting of all the restraints so added. But it should not be done in new code.

# Sampling {#overview_sampling}

It is now time to find configurations of the model that score well with
regards to the scoring function you developed. %IMP provides a number of
tools for that.

## Optimizers {#overview_optimizers}

An IMP::Optimizer takes the current configuration of the model and perturbs it,
typically trying to make it better (but perhaps just into a different
configuration following some rule, such as
[molecular dynamics](@ref IMP::atom::MolecularDynamics)). They use a scoring
function you provide to guide the process.

    my_optimizer= IMP.core.ConjugateGradients(m)
    my_optimizer.set_scoring_function(my_scoring_function)
    my_optimizer.optimize(1000)

\note In old code, the scoring function may not be explicitly set on the optimizer. The optimizer then uses the implicit scoring function in the IMP::Model. This shouldn't be done in new code as it is a bit error prone and may become an error at some point.

Optionally, you can use an IMP::Sampler, which uses an IMP::Optimizer to
produce a set of configurations of the model using some sampling scheme.

# Storing and analysis {#overview_analsysis}

Configurations of the model can be saved and visualized in a variety of ways. Atomic structures can be written as PDB files using IMP::atom::write_pdb(). More flexibly, coarse grained models, geometry and information about the scoring function can be written to [RMF files](http://integrativemodeling.org/rmf).

     my_rmf= RMF.create_rmf_file("my.rmf")
     IMP.rmf.add_hierarchy(my_rmf, my_hierarchy)
     IMP.rmf.add_restraint(my_rmf, my_excluded_volume_restraint)
     IMP.rmf.save_frame(my_rmf, 0)
