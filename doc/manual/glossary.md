Glossary {#glossary}
========

Several of the more common terms used in %IMP (often parts of class names) are
covered below.

This is *not* intended to be a complete list of classes in %IMP. For that,
please see the [class list](../ref/annotated.html)
in the [Reference Guide](../ref/).

**Model**
Typically, only a single Model is used in an %IMP run. Multiple
proteins or conformations can be (and often are) stored in a single model.
A single conformation is usually stored in a **Hierarchy**.

**Attributes**
The **Model** contains a number of Particles, each of which can have any
number of attributes (and different Particles can have different sets of
attributes). For example, some Particles can have x, y and z attributes and so
act like points, while other Particles can have attributes that point to other
Particles and so act like bonds or other connections. Attributes are rarely
accessed directly, but instead **Decorator** objects are used.

**ParticleIndex**
All of the Particles are stored in the Model as a simple one-dimensional
array. While they can be accessed as Particle objects, it is more
efficient just to use a Model pointer and an index into that array. This
index is termed a ParticleIndex. Any function that expects a ParticleIndex
can also be passed a Particle in Python, which is automatically converted.

**Decorator**
A Decorator is an object that "wraps" an existing Particle and presents
a particular interface to it - it is not a Particle in its own right.
These are used rather than accessing Particle attributes directly. For
example, the IMP::core::XYZ Decorator is used to access point-like
Particles. See also **Hierarchy**.

**Hierarchy**
While Particles are stored as a simple flat list inside the Model,
it is often useful to group them and set up parent-child relationships.
This is implemented in %IMP with a special "Hierarchy" Decorator.
For example, the IMP::atom::Hierarchy Decorator provides for a molecular
hierarchy, whereby a single "residue" Particle may contain a number of "atom"
Particles as children. See also **Leaves**.

**ScoringFunction**
This is used to calculate how well a configuration in the Model satisfies
the Restraints (i.e. the score or energy of the system). Typically this
is just the sum of all Restraints. In most cases, there will only be one
ScoringFunction, but it is possible to create multiple such functions
(for example to score subsets of the system).
Contrast with **Score**.

**Singleton**
Used to refer to a single Particle, as opposed to a pair,
triple or quad of Particles. For example, an
IMP::container::ListSingletonContainer contains a list of
Particles, while an IMP::container::ListPairContainer contains a list
of pairs of Particles.

**Container**
An object that contains a group of related Particles. This allows the same
set of Particles to be used in more than one Restraint or Constraint
without having to specify them multiple times. Some Containers also can
fill themselves automatically, for example with a list of all pairs of
Particles that are close in space (see IMP::container::ClosePairContainer).

**Modifier**
A function that changes the attributes of a single Particle or pair,
triple or quad (IMP::SingletonModifier, IMP::PairModifier,
IMP::TripletModifier, IMP::QuadModifier respectively). For example,
IMP::core::TransformationSymmetry is a SingletonModifier
that rotates and translates a single Particle. Modifiers are most commonly
used by Constraints.

**Predicate**
A function that returns a single integer value, given a single
Particle or pair, triple or quad of Particles (IMP::SingletonPredicate,
IMP::PairPredicate, IMP::TripletPredicate, IMP::QuadPredicate respectively).
Usually this is interpreted as a True/False value (non-zero/zero,
respectively). For example, IMP::core::ClosePairsFinder, which returns all
pairs of Particles that are nearby in space, can use a PairPredicate to
exclude certain pairs from this list.
IMP::atom::StereochemistryPairFilter is a PairPredicate that excludes all pairs
of atoms that are connected by bonds.

**Restraint**
Every Restraint in %IMP is implemented as a function that returns a
score for some subset of the Model. Often this delegates to a **Score** object
to make the restraint more flexible. See also **ScoringFunction** for the total
score for the entire Model (which is typically just the sum of all
restraints). A Restraint is satisfied by modifying the system to minimize
its score.

**Constraint**
Unlike a Restraint, a Constraint defines some invariant of the
system that cannot be violated (IMP::core::RigidBody is a commonly-used
example). Each Constraint is implemented by changing the Model (using
a Modifier) prior to each ScoringFunction evaluation (and in some cases after
evaluation too) to ensure that the invariant is not violated.
**ScoreState** is a synonym.

**ScoreState**
This is simply a synonym for **Constraint**.

**Score**
A Score object calculates a score for a Particle, or pair, triple or quad
of Particles. For example, IMP::core::DistancePairScore scores two Particles
based on the Cartesian distance between them. Typically these are used
internally by Restraints.
Do not confuse with **Restraint** or **ScoringFunction**.

**Mover**
One of the mechanisms for sampling the **ScoringFunction** is Monte Carlo (MC)
simulation. This relies on a number of **Mover** objects that perturb the
system in some way at each MC step. For example, IMP::core::BallMover is often
used with pointlike Particles to move them in Cartesian space.

**Leaves**
The leaves of an IMP **Hierarchy** are simply the children that don't in turn
have their own children. A commonly-used hierarchy is a molecular hierarchy,
in which often the atoms are the leaves (although this is not always the case;
where atomic information is not available the leaves may be residues, fragments
of sequence, or even entire proteins).
