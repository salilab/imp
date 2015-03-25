Design example {#designexample}
==============

# Overview #

[TOC]
This page walks through an iterative design process to give an
example of what sort of issues are important and what to think about
when choosing how to implement some functionality.

# Original Description # {#design_original}

 Hao wants to implement ligand/protein scoring to IMP so that he can
 take advantage of the existing infrastructure. The details of the scoring
 function are currently experimental. The code does the following:

1. Read in the protein pdb and the small ligand mol2. The protein is in
   a pdb file and so can use IMP::atom::read_pdb. The ligand is in a mol2
   file which defines its own set of pdb-compatible atom types.
2. He proposed storing the coordinates and atom types in vectors outside
   of the decorators to speed up scoring.
3. Read in the potential of mean force (PMF) table from a file with
   a custom format. The number of dimensions can be constant including
   the two atom types for a pair of atoms, and the distance between that
   pair. The values are stored in the table, will not change during the
   program, and need to be looked up quickly given the dimension data.
   The PMF table uses different atom names than the mol2 file.
4. Score a conformation by looping over all ligand-protein atom
     pairs. For each pair look up the PMF value in the table by the
     two atom types and the distance, sum up all PMF values.

## Comments on the original description ## {#design_original_comments}

1. mol2 is a standard file format so it makes sense to have a reader
   for it in IMP. We can adopt the mol2 atom names as the standard names
   for ligand atoms in IMP.
2. The details of how the coordinates are stored and accessed are
   implementation details and worrying about them too much should probably
   be delayed until later once other considerations are figured out.
3. Loading the PMF table is a natural operation for an initialization
   function. However, since the PMF table is not a standard file format,
   it doesn't make sense for it to go into IMP, at least not until a file
   format for the protein-ligand scoring has been worked out. Also there is
   little reason to keep the PMF table atom types around, and they probably
   should be converted to more standard atom types on load. Finally, since
   the data in the PMF file is directly the scoring data, there isn't a
   real need to have a special representation for it in memory.
4. There are two different considerations here; which pairs of atoms to
   use and how to score each pair.


# Design Proposal for Reading # {#design_reading}
Since the mol2 reader is quite separate from the scoring, we will consider
it on its own first. In analogy to the pdb reader, it makes sense to
provide a function `read_mol2(std::istream &in, Model *m)` which returns
an IMP::atom::Hierarchy.

The mol2 atom types can either be added at runtime using
IMP::atom::add_atom_type() or a list of predefined constants can be added
similar to the IMP::atom::AT_N. The latter requires editing both
IMP/atom/Atom.h and modules/atom/src/Atom.cpp and so it is a bit harder
to get right.

# Implementing Scoring as a IMP::Restraint # {#design_restraint}

First, this functionality should probably go in a new module since it
is experimental. One can use the scratch module in a separate `git` branch,
for example.

One could then have a `PMFRestraint` which loads a PMF file from the
module data directory (or from a user-specified path). It would
also take two IMP::atom::Hierarchy decorators, one for the ligand and
one for the protein and score all pairs over the two. For each pair of atoms,
it would look at the IMP::atom::Atom::get_type() value and use that
to find the function to use in a stored table.

Such a design requires a reasonable amount of implementation, especially
once one is interested in accelerating the scoring by only scoring nearby
pairs. The `PMFRestraint` could use a IMP::core::ClosePairsScoreState
internally if needed.

# Implementing Scoring as a IMP::PairScore # {#design_score}

One could instead separate the scoring from the pair generation by implementing
the scoring as an IMP::PairScore. Then the user could specify an
IMP::core::ClosePairsScoreState when experimenting to see what is the fastest
way to implement things.

As with the restraint solution, the IMP::PairScore would use the
IMP::atom::Atom::get_type() value to look up the correct function to use.

If you look around in \imp for similar pair scores (see IMP::PairScore and the
inheritance diagram) you see there is a IMP::core::TypedPairScore which
already does what you need. That is, it takes a pair of particles, looks up
their types, and then applies a particular IMP::PairScore based on their types.
IMP::core::TypedPairScore expects an IMP::IntKey to describe the type. The
appropriate key can be obtained from IMP::atom::Atom::get_type_key().

Then all that needs to be implemented in a function, say
IMP::hao::create_pair_score_from_pmf() which creates an IMP::core::TypedPairScore,
loads a PMF file and then calls IMP::core::TypedPairScore::set_pair_score() for
each pair stored in the PMF file after translating PMF types to the
appropriate IMP::atom::AtomType.

This design has the advantage of very little code to write. As a result it
is easy to experiment (move to 3D tables or change the set of close pairs). Also
different, non-overlapping PDFs can be combined by just adding more terms to
the IMP::core::TypedPairScore.

The disadvantages are that the scoring passes through more layers of function
calls, making it hard to use optimizations such as storing all the coordinates
in a central place.


# Some final thoughts # {#design_final}

1. Figure out orthogonal degrees of freedom and try to split
  functionality into pieces that control each. Here it is the set
  of pairs and how to score each of them. Doing this makes it
  easier to reuse code.
2. Don't create two classes when you only have one set of work. Here,
 all you have is a mapping between a pair of types and a
 distance and a score. Having both a PMFTable and PMFPairScore
 locks you into that aspect of the interface without giving you
 any real flexibility.
3. Implementing things in terms of many small classes makes the
 design much more flexible. You can easily replace a piece
 without touching anything else and since each part is simple,
 replacing a particular piece doesn't take much work. The added
 complexity can easily be hidden away using helper functions in
 your code (or, if the action is very common, in IMP).
