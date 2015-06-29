Example of using the library {#library_example}
============================

A complete modeling script, <tt>simple.py</tt>, written in Python,
is shown below:

\include simple.py

In the first part of the script, the %IMP kernel and the
[algebra](@ref IMP::algebra) and [core](@ref IMP::core) modules are loaded,
as regular Python
modules. We then proceed to set up the representation of the system, using
the Model and Particle classes defined in the
kernel. Here we give the two particles (`p1` and `p2`) point-like attributes
using the IMP::core::XYZ decorator.

In the second part, we set up the scoring of the system. We add two restraints
to the Model, one of which harmonically restrains `p1` to the origin and the
other of which restrains `p1` and `p2` to be distance 5.0 apart. (%IMP does not
enforce any units of distance; however, some physical optimizers, such
as molecular dynamics, [expect distances to be in angstroms](@ref units).)

Note that
the core module provides suitable building block restraints for this purpose.
In the first case, we use the IMP::core::SingletonRestraint class that creates
a restraint on a single particle (`p1`). It delegates the task of actually
scoring the particle, however, to another class called
IMP::SingletonScore that is simply given the
Particle and asked for its score. In this example, we use a type of
SingletonScore called
IMP::core::DistanceToSingletonScore
that calculates the Cartesian distance between the point-like Particle and
a fixed point (in this case the origin), and again delegates the task of
scoring the distance to another class, an
IMP::UnaryFunction. In this case, the
UnaryFunction is a simple harmonic function, IMP::core::Harmonic,
with a mean of zero. Thus, the Particle `p1` is harmonically restrained
to be at the origin. The second restraint is set up similarly; however,
in this case the restraints and scores act on a pair of particles.
This building block functionality makes it easy to add a new type of
restraint; for example, to implement a van der Waals potential it is
only necessary to provide a suitable PairScore that scores a single pair
of particles; the functionality for efficiently enumerating all pairs
of such particles is already provided in %IMP.

Finally, in the third part of the script, we tell %IMP that it can move
the two point-like particles, and to build a system configuration that
is consistent with all the restraints. In this example, a simple conjugate
gradients optimization is used.

The script is a regular Python script. Thus, provided that both %IMP and
Python are installed, it can be run on any machine, by typing on a command
line, in the same directory as the script:

<tt>python simple.py</tt>

The script will run the optimization, printing %IMP [log messages](@ref logging)
as it goes, and finally print the coordinates of the optimized particles.

%IMP is designed such that the C++ and Python interfaces are similar to use.
Thus, %IMP applications or protocols can be constructed either in C++ or in
Python, and new %IMP functionality (for example, new types of Restraint)
can be implemented in either language. For a comparison, please inspect the
<tt>simple.cpp</tt> file below. This file implements the same protocol as
the first part of <tt>simple.py</tt> but uses the %IMP C++ classes rather
than their Python equivalents. The two programs are very similar; the only
differences are in the language syntax (eg, the Python
'<tt>import IMP.algebra</tt>' translates to
'<tt>\#include \<IMP/algebra.h\></tt>'
in C++) and in memory handling (Python handles memory automatically;
in C++, memory handling must be done explicitly by using the
IMP::Pointer class or the [IMP_NEW](@ref IMP::IMP_NEW) macro,
which adds reference counting to automatically clean up after %IMP objects
when they are no longer in use).

\include simple.cpp
