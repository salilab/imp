<?php
   include("misc.inc.php");
   print_page_header();
?>
<div id="tutorial">

<h1>The IMP C++/Python library</h1>

<p>
The core of IMP is the C++/Python library, which provides all of the necessary
components, as a set of classes and modules, to allow methods developers
to build an integrative modeling protocol from scratch. Most users of IMP
will use one of the higher-level interfaces described in later sections of
this tutorial; however, we will briefly demonstrate this library here to
illustrate the core IMP concepts that these interfaces rely on.
</p>

<p>
The IMP library is split into a <a href="../doc/html/namespaces.html">kernel
and a set of extension modules</a>. The
<a href="../doc/html/namespaceIMP.html">kernel</a> is a small collection of
classes that define the storage of information about the system and the
main interfaces used to interact with that information. The information
is stored in a set of
<a href="../doc/html/classIMP_1_1Particle.html">Particle</a>
objects; these are flexible and abstract data containers, able to hold
whatever information is necessary to represent the system. For example,
a given Particle may be assigned x, y, and z attributes to store point
coordinates, another may be assigned x, y, z, and a radius to represent
a sphere, and another may contain two pointers to other Particles to
represent a bond or another relationship. The kernel defines only the
abstract interfaces to manipulate the data in the Particles, but does not
provide implementations; these are provided in the extension modules. For
example, it merely defines a
<a href="../doc/html/classIMP_1_1Restraint.html">Restraint</a> as any object
that, given a set of Particles, returns a score, and an
<a href="../doc/html/classIMP_1_1Optimizer.html">Optimizer</a> as an object that
changes the attributes of all Particles to yield an optimized score over
all restraints. It is the
<a href="../doc/html/namespaceIMP_1_1core.html">core module</a> that provides,
for example, a
<a href="../doc/html/classIMP_1_1core_1_1DistanceRestraint.html">concrete
Restraint</a> acting like a harmonic ‘spring’ between two point-like
Particles, an Optimizer that
<a href="../doc/html/classIMP_1_1core_1_1ConjugateGradients.html">utilizes the
conjugate gradients minimization method</a>, and much other functionality.
</p>

<p>
IMP includes a variety of  modules. Some modules provide the basic building
blocks needed to construct a protocol, such as the
<a href="../doc/html/namespaceIMP_1_1core.html">core module</a> that provides
functionality including harmonic restraints, point-like and spherical
particles, and basic optimizers, and the
<a href="../doc/html/namespaceIMP_1_1atom.html">atom module</a> that provides
atom-like particles, a molecular dynamics optimizer, etc.  Other modules
provide support for specific types of experimental data or specialized
optimizers, such as the
<a href="../doc/html/namespaceIMP_1_1em.html">em module</a> that supports
electron microscopy data, and the
<a href="../doc/html/namespaceIMP_1_1domino.html">domino module</a> that
provides an inference-based divide-and-conquer optimizer. IMP is designed
so that it is easy to add a new module; for example, a developer working
on incorporating data from a new experimental technique may add a new IMP
module that translates the data from this technique into spatial restraints.
</p>

<p>
IMP is primarily implemented in C++ for speed; however, each of the classes
is wrapped so that it can also be used from Python.  A protocol can thus
be developed from scratch by simply writing a Python script. As an example,
we will first look at the script <span class="filename">simple.py</span>:
</p>

<?php example_file("library", "simple.py"); ?>

<p>
In the first part of the script, the IMP kernel and the
<a href="../doc/html/namespaceIMP_1_1algebra.html">algebra</a>
and core modules are loaded, as regular Python modules. We then proceed
to set up the representation of the system, using the
<a href="../doc/html/classIMP_1_1Model.html">Model</a> and Particle classes
defined in the kernel. The Model class represents the entire system, and
keeps track of all the Particles, Restraints, and links between them.
As mentioned earlier, the Particle class is a flexible container, but here
we give the two Particles (p1 and p2) point-like attributes using the
<a href="../doc/html/classIMP_1_1core_1_1XYZ.html">XYZ</a> class defined
in the core module. This XYZ class is known as a ‘decorator’; it does not
create a new Particle, but merely presents a new interface to an existing
Particle, in this case a point-like one. (Multiple decorators can be applied
to a single Particle; for example, an atom-like Particle could be treated
like a point, a sphere, an electrically charged particle, or an atom.)
We can then treat each Particle like a point using methods in the XYZ
class, here setting the x, y, and z coordinates to a provided vector.
</p>

<p>
In the second part, we set up the scoring of the system. We add two restraints
to the Model, one of which harmonically restrains p1 to the origin and the
other of which restrains p1 and p2 to be distance 5.0 apart.  (IMP does not
enforce any units of distance; however, some physical optimizers, such as
molecular dynamics, expect distances to be in angstroms.) Note that the
core module provides suitable building block restraints for this purpose.
In the first case, we use the
<a href="../doc/html/classIMP_1_1core_1_1SingletonRestraint.html">SingletonRestraint</a>
class that creates a restraint on a single particle (p1). It delegates
the task of actually scoring the particle, however, to another class called
<a href="../doc/html/classIMP_1_1SingletonScore.html">SingletonScore</a>
that is simply given the Particle and asked for its score. In this example,
we use a type of SingletonScore called a
<a href="../doc/html/classIMP_1_1core_1_1DistanceToSingletonScore.html">DistanceToSingletonScore</a>
that calculates the Cartesian distance between the point-like Particle and
a fixed point (in this case the origin), and again delegates the task of
scoring the distance to another class, a
<a href="../doc/html/classIMP_1_1UnaryFunction.html">UnaryFunction</a>.
In this case, the UnaryFunction is a simple harmonic function with a mean
of zero.  Thus, the Particle p1 is harmonically restrained to be at the
origin. The second restraint is set up similarly; however, in this case
the restraints and scores act on a pair of particles. This building block
functionality makes it easy to add a new type of restraint; for example,
to implement a van der Waals potential it is only necessary to provide a
suitable PairScore that scores a single pair of particles; the functionality
for efficiently enumerating all pairs of such particles is already provided
in IMP.</p>

<p>
Finally, in the third part of the script, we tell IMP that it can move the
two point-like particles, and to build a system configuration that is
consistent with all the restraints. In this example, a simple conjugate
gradients optimization is used.
</p>

<p>
The script is a regular Python script. Thus, provided that both IMP and
Python are installed, it can be run on any machine, by typing on a command
line, in the same directory as the script:
</p>

<p><tt>python simple.py</tt></p>

<p>
The script will run the optimization, printing IMP log messages as it goes,
and finally print the coordinates of the optimized particles.
</p>

<p>
IMP is designed such that the C++ and Python interfaces are similar to use.
Thus, IMP applications or protocols can be constructed either in C++ or
in Python, and new IMP functionality (for example, new types of Restraint)
can be implemented in either language. For a comparison, please inspect the
<span class="filename">simple.cpp</span> file below. This file implements
the same protocol as the first part of <span class="filename">simple.py</span>
but uses the IMP C++ classes rather than their Python equivalents. The two
programs are very similar; the only differences are in the language syntax
(eg, the Python ‘<tt>import IMP</tt>’ translates to
‘<tt>#include &lt;IMP.h&gt;</tt>’ in C++)
and in memory handling (Python handles memory automatically; in C++,
memory handling must be done explicitly by using the
<a href="../doc/html/classIMP_1_1Pointer.html">IMP::Pointer</a>
class, which adds reference counting to automatically clean up after
IMP objects when they are not used anymore).
</p>

<?php example_file("library", "simple.cpp"); ?>

<p><a href="restrainer.html">Next chapter</a>; <a href="./">Tutorial index</a>.</p>

</div>
<?php
   print_page_footer();
?>
