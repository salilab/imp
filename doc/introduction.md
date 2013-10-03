# Introduction

# Integrative modeling # {#introduction}
[TOC]

Detailed structural characterization of macromolecular assemblies is usually more difficult than that of single proteins because assemblies often don’t crystallize or are too large for NMR spectroscopy. This challenge can be addressed by an “integrative” or “hybrid” approach that simultaneously considers all available information about a given assembly. The integrative approach has several advantages. First, synergy among the input data minimizes the drawbacks of sparse, noisy, ambiguous and incoherent datasets. Each individual piece of data contains little structural information, but by simultaneously fitting a model to all data derived from independent experiments, the degeneracy of the structures that fit the data can be markedly reduced. Second, this approach has the potential to produce all structures that are consistent with the data, not only one structure. Third, an analysis of the structures allows us to estimate the precision of both the data and the structures. Last, this approach can make the process of structure determination more efficient, by indicating which measurements would be most informative.

## Example modeling efforts ## {#introduction_efforts}
Hybrid structures based on our integrative approach:
 - The E. coli ribosome, the first eukaryotic ribosome from S. cerevisiae
 - The first mammalian ribosome from C. lupus48 and a fungal ribosome
 - The E. coli Hsp90
 - The eukaryotic chaperonin TRiC/CCT
 - The actin/scruin complex
 - Ryr1 voltage gated channel
 - The baker’s yeast [nuclear pore complex](http://salilab.org/npc) (NPC)
 - The Nup84 complex
 - Transport through the NPC
 - Microtubule nucleation
 - The 26S proteasome
 - [PCS9K-Fab complex](../tutorial/idock_pcsk9.html)
 - The yeast spindle pole body
 - Chromatin globin domain
 - The lymphoblastoid cell genome

## The four stage process ## {#introduction_four_stages}

The integrative structure determination is an iterative process consisting of four stages:
- gathering of data;
- design of the model representation and encoding of the data as a scoring function. The scoring function consists of that consists of terms, called restraints, one for each data-point
- the sampling method that finds good scoring conformations of the model;
- and analysis of data and resulting model conformations, including of uncertainty in the solutions.

# IMP # {#introduction_imp}

IMP provides tools to implement the computational parts of the integrative modeling iterative process, steps 2-4. This computation can be driven from Python scripts or C++ programs. The examples below will use Python scripts.

## Representation: IMP::Model ## {#introduction_representation}

In IMP, the model is represented as a collection of data, called particles, each of which has associated attributes (e.g. an atom with associated coordinates, mass, radius etc). In IMP, the attributes can be numbers, strings, or lists of other particles, among other things. Each particle is identified by an index (IMP::kernel::ParticleIndex) and has an associated name, in order to make it easier to understand. Finally, attributes are identified by keys (e.g. IMP::kernel::StringKey for string attributes). The key identifies one type of data that may be contained in many particles.

At the most basic, to create particles and manipulate attributes you can do

    import IMP.kernel
    model= IMP.kernel.Model()
    particle_0= m.add_particle("my first particle")
    string_key = IMP.kernel.StringKey("my first data")
    model.add_attribute(string_key, particle_0, "Hi, particle 0")

    particle_1= m.add_particle("my second particle")
    model.add_attribute(string_key, particle_1, "Hi, particle 1")

    print model.get_attribute(string_key, particle_0)

Certain of the attributes can be marked as parameters of the model. These are attributes that you want to sample or optimize. To do so you can do
   model.set_is_optimized(float_key, particle_0)

\note A lot of IMP uses IMP::Particle objects instead of IMP::kernel::ParticleIndex objects to identify particles. The should be treated as roughly the same. To map from an index to a particle you use IMP::kernel::Model::get_particle() and to go the other way IMP::kernel::Particle::get_index(). Using the indexes is preferred. When doing it on lists, you can use IMP::kernel::get_indexes() and IMP::kernel::get_particles().

## Decorators ## {#introduction_decorators}

Accessing all your data at such a low level can get tiresome, so we provide decorators to make it easier. Each type of decorator provides an interface to manipulate a particular type of data easier. For example, an IMP.atom.Residue decorator provides access to residue associated information (e.g. the index of the residue, or its type) in particles that have it.

    residue= IMP.atom.Residue(model, my_residue)
    print residue.get_residue_type()

Decorators provide a standard interface to add their data to a particle, decorate a particle that already has the needed data or check if a particle is appropriate to have the decorator used with it.

    # add coordinates to a particle
    decorated_particle = IMP.core.XYZ.setup_particle(model, my_particle, IMP.algebra.Vector3D(1,2,3))
    print decorated_particle.get_coordinates()

    # my_other_particle has already been set up, so we can just decorate it directly
    another_decorated_particle = IMP.core.XYZ(model, my_other_particle)
    print another_decorated_particle.get_coordinates()

    # we can change the coordinates too
    another_decorated_particle.set_coordinates(IMP.algebra.Vector3D(5,4,3))

Decorators can also be used to create relationships between particles. For example, rigid bodies are implemented using the IMP::core::RigidBody decorator. Each rigid body has a collection of other particles that move along with it, the IMP::core::RigidMember particles.

## Representing biological molecules ## {#introduction_biomolecules}

Biological modules are represented hierarchically in IMP using the IMP::atom::Hierarchy. These hierarchies follow the natural hierarchical nature of most biomolecules. A protein from a PDB would be a hierarchy with a root for the whole PDB file with a child per chain. Each chain particle has a child for each residue in the chain, and each residue has a child for each atom. Each particle has various types of associated data. For example and atom has data using the IMP::atom::Atom, IMP::core::XYZR, IMP::atom::Mass and IMP::atom::Hierarchy decorators.

The structures represented do not have to be atomic and can be multi-resolution. That is can have coordinates at any level of the hierarchy. The invariants are that the leaves must have coordinates, radii and mass. Pieces of the hierarchy can be picked out using the IMP::atom::Selection using the standard sort of biological criteria:

    # Select residues 10 through 49.
    my_residues= IMP.atom.Selection(my_pdb, residue_indexes=range(10,50)).get_particles()


## Containers ## {#introduction_containers}

You can manipulate and maintain collections of particles using IMP::Container. A collection can be anything from a list of particles gathered manually, to all pairs of particles from some list that are closer than a certain distance to one another. For example, to maintain a list of all close pairs of particles you can do

    # all particle pairs closer than 3A
    # it is always good to give things name, that is what the last argument does
    close_pairs= IMP.container.ClosePairContainer(all_my_particles, 3, "My close pairs")

These containers can then be used to create scoring functions or analyze the data.

## Constraints and Invariants ## {#introduction_constraints}

Many things such as rigid bodies and lists of all close pairs depend on maintaining some property as the model changes. These properties are maintained by IMP::kernel::Constraint objects. Since the invariants may depend on things that are reasonably expensive to compute, these invariants are updated only when requested. This means that if you change the coordinates of some particles, the contents of the close pairs list might be incorrect until it is updated. The required update can be triggered implicitly, for example when some scoring function needs it, or explicitly, when IMP::kernel::Model::update() is called.

Behind the scenes, IMP maintains an IMP::kernel::DependencyGraph that tracks how information flows between the particles and the containers, based on the constraints. It is used to detect, for example, that a particular particle is part of a rigid body, and so if its coordinates are needed for scoring, the rigid body must be brought up to date and the appropriate constraint must be asked to update the member particle's coordinates. In order to be able to track this information, relevant objects (IMP::kernel::ModelObject) have methods IMP::kernel::ModelObject::get_inputs() and IMP::kernel::ModelObject::get_outputs() that return the things that are read and written respectively.

## Scoring ## {#introduction_scoring}

One then needs to be able to evaluate how well the current configuration of the model fits this data that one is using to model. In addition to scores, when requested derivatives of the total score as a function of each parameter can be computed.

### Restraints ### {#introduction_restraints}

An IMP::kernel::Restraint computes a score on some set of particles. For example, a restraint be used to penalize configurations of the model that have collisions

    # penalize collisions with a spring constant of 10 kcal/mol A
    soft_sphere_pair_score= IMP.core.SoftSpherePairScore(10)
    my_excluded_volume_restraint= IMP.container.PairsRestraint(soft_sphere_pair_score,
                                                              close_pairs,
                                                              "excluded volume")

To get the score of an individual restraint, you can use its IMP::kernel::Restraint::get_score() method.

### Scoring functions ### {#introduction_scoring_functions}

Scoring in IMP is done by creating an IMP::kernel::ScoringFunction. A scoring function consists of the sum of terms, each called a Restraint. You can create many different scoring functions for different purposes and each restraint can be part of multiple scoring functions.

        my_scoring_function= IMP.core.RestraintsScoringFunction([my_excluded_volume_restraint],
                                                                "score excluded volume")

\note You will see old example code that, instead of creating an IMP::kernel::ScoringFunction, adds the restraints to the model. This creates an implicit scoring function consisting of all the restraints so added. But it should not be done in new code.

## Sampling ## {#introduction_sampling}

It is now time to find configurations of the model that score well with regards to the scoring function you developed. IMP provides a number of tools for that.

### Optimizers ### {#introduction_optimizers}

An IMP::kernel::Optimizer takes the current configuration of the model and perturbs it, typically trying to make it better (but perhaps just into a different configuration following some rule, such as molecular dynamics). They using a scoring function you provide to guide the process.

    my_optimizer= IMP.core.ConjugateGradients(m)
    my_optimizer.set_scoring_function(my_scoring_function)
    my_optimizer.optimize(1000)

\note In old code, the scoring function may not be explicitly set on the optimizer. The optimizer then uses the implicit scoring function in the IMP::Model. This shouldn't be done in new code as it is a bit error prone and may become an error at some point.

### Samplers ### {#introduction_samplers}

A IMP::kernel::Sampler produces a set of configurations of the model using some sampling scheme.

## Storing and analysis ## {#introduction_analsysis}

Configurations of the model can be saved and visualized in a variety of ways. Atomic structures can be written as pdb files using IMP::atom::write_pdb(). More flexibly, coarse grained models, geometry and information about the scoring function can be written to RMF files.

     my_rmf= RMF.create_rmf_file("my.rmf")
     IMP.rmf.add_hierarchy(my_rmf, my_hierarchy)
     IMP.rmf.add_restraint(my_rmf, my_excluded_volume_restraint)
     IMP.rmf.save_frame(my_rmf, 0)


## Modular structure of IMP ## {#introduction_modular}

Functionality in \imp is grouped into modules, each with its own
namespace (in C++) or package (in Python). For %example, the functionality
for IMP::core can be found like

    IMP::core::XYZ(p)

in C++ and

    IMP.core.XYZ(p)

in Python.

A module contains classes,
methods and data which are related and controlled by a set of authors. The names
of the authors, the license for the module, its version and an overview of the
module can be found on the module main page (e.g. IMP::example).
See the "Namespaces" tab above for a complete list of modules in this version of \imp.

## Understanding what is going on ## {#introduction_understanding}

IMP provides two sorts of tools to help you understand what is going on when you write a script. Both logging and checks are disabled if you use a `fast` build, so make sure you have access to a non-`fast` build.

### Logging ### {#introduction_logging}

Many operations in IMP can print out log messages as they work, allowing one to see what is being done. The amount of logging can be controlled globally by using IMP::base::set_log_level() or for individual objects by calling, for example `model.set_log_level(IMP.base.VERBOSE)`.

### Runtime checks ### {#introduction_checks}

IMP implements lots of runtime checks to make sure both that it is used properly and that it is working correctly. These can be turned on and off globally using IMP::base::set_checks_level() or for individual objects.

## Conventions ## {#introduction_conventions}

IMP tries to make things simpler to use by adhering to various naming and interface conventions.

### Units ### {#introduction_units}
Unless documented otherwise, the following units are used
- angstrom for all distances
- \f$ \frac{kcal}{mol  \unicode[serif]{xC5}}\f$ for forces/derivatives
- \f$\frac{kcal}{mol}\f$ for energies
- radians for angles. All angles are counterclockwise.
- all charges are in units of the elementary charge
- all times are in femtoseconds

### Names ### {#introduction_names}

- Names in `CamelCase` are class names, for %example IMP::RestraintSet
- Lower case names with underscores (`_`) in them are functions or methods, for example IMP::Model::update().
- Collections of data of a certain class, e.g. `ClassName` are passed using type type `ClassNames`. This type is a `list` in Python and a IMP::base::Vector<ClassName> or, more or less,  IMP::base::Vector<ClassName*> in C++.
- These function names start with a verb, which indicates what the method does. Methods starting with
   - `set_` change some stored value
   - `get_` create or return a \c value object or which
     return an existing \c object \c class object.
   - `create_`  create a new IMP::base::Object class object
   - `add_`, `remove_` or `clear_` manipulate the contents of a collection of data
   - `show_` prints things in a human-readable format
   - `load_` and `save_` or `read_` and `write` move data between files and memory
   - `link_` creates an connection between something and an IMP::base::Object
   - `update_` changes the internal state of an IMP::base::Object
   - `do_` is a virtual method as part of a \external{http://en.wikipedia.org/wiki/Non-virtual_interface_pattern,non-virtual interface pattern}
   - `handle_` takes action when an event occurs
   - `validate_` checks the state of data and prints messages and throws exceptions if something is corrupted
   - `setup_` and `teardown_` create or destroy some type of invariant (e.g. the constraints for a rigid body)
   - `apply_` either applies a passed object to each piece of data in some collection or applies the object itself to a particular piece of passed data (yeah, it is a bit ambiguous).
- names starting with `IMP_` are preprocessor symbols (C++ only)
- names don't use abbreviations

### Graphs ### {#introduction_graphs}

Graphs in IMP are represented in C++ using the \external{http://www.boost.org/doc/libs/release/libs/graph, Boost Graph Library}. All graphs used in IMP are \external{http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/VertexAndEdgeListGraph.html, VertexAndEdgeListGraphs}, have vertex_name properties,
are \external{http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/BidirectionalGraph.html, BidirectionalGraphs} if they are directed.

The Boost.Graph interface cannot be easily exported to Python so we instead provide a simple wrapper IMP::PythonDirectedGraph. There are methods to translate the graphs into various common Python and other formats (e.g. graphviz).


### Values and Objects (C++ only) ### {#introduction_values}

As is conventional in C++, IMP classes are divided into broad, exclusive types
- *Object classes*: They inherit from IMP::base::Object and are always passed by pointer. They are reference counted and so should only be stored using IMP::base::Pointer (in C++, in Python everything is reference counted). Never allocate these on the stack as very bad things can happen. Objects cannot be duplicated. Equality on objects is defined as identity (e.g. two different objects are different even if the data they contain is identical).

- *Value classes* which are normal data types. They are passed by value (or `const&`), never by pointer. Equality is defined based on the data stored in the value. Most value types in IMP are always valid, but a few, mostly geometric types (IMP::algebra::Vector3D) are designed for fast, low-level use and are left in an uninitialized state by their default constructor

- *RAII classes* control some particular resource. They grab control of a resource when created and then free it when they are destroyed. As a result, they cannot be copied. Non-IMP examples include things like files in Python, which are automatically closed when the file object is deleted.

All types in IMP, with a few documented exceptions, can be
- compared to other objects of the same type
- output to a C++ stream or printed in Python
- meaningfully put into Python dictionaries or C++ hash maps

### Backwards compatibility and deprecation ### {#introduction_backwards}

IMP tries to maintain backwards compatibility, however, this is not always feasible. Our general
policy is that functionality that is deprecated in one release (e.g. 2.1) is removed in the next one (2.2).
Deprecated functionality should produce warnings when use (e.g. compile time messages for deprecated
macros and runtime messages for deprecated functions called from Python). In addition, bugs discovered
in deprecated functionality are not fixed.

# Where to go next # {#introduction_next}

Probably the best thing to do next is to read the [kernel/nup84.py](kernel/kernel_2nup84_8py-example.html) example.
