Frequently asked questions (FAQ) {#faq}
================================

[TOC]

# General {#faq_general}

## How do I make my script run faster?
* Use specialized scores that don't take a IMP::UnaryFunction argument, e.g.
  replace the combination of IMP::core::SphereDistancePairScore and
  IMP::core::HarmonicLowerBound with IMP::core::SoftSpherePairScore.
  This reduces the number of virtual function calls during evaluation.
* Group similar restraints: e.g. replace a large number of
  IMP::core::PairRestraint objects with a single
  IMP::container::PairsRestraint. This also reduces the number of
  function calls.
* If you are programming in C++, you can use IMP::container::create_restraint()
  to create a 'generic' restraint from a container and a pair score. This
  removes a few more virtual function calls. If you are creating a restraint
  with only a single particle or pair, use IMP::create_restraint() instead.
* Adjusting slack parameters on non-bonded lists: IMP::container::ClosePairContainer and IMP::core::ExcludedVolumeRestraint have "slack" parameters which adjust how often the non-bonded lists that are used are recomputed. Changing this parameter does not change the computed score, just how often the lists are updated. As a rough guideline, you want the slack to be larger than the total amount any particle moves in, say 20 evaluates (e.g. 20 MD steps). Experiment with changing the parameter to see what yields the  best results or try the IMP::container::get_slack_estimate() function to try to estimate the best value to use automatically.
* Turn off or turn down logging: call IMP.set_log_level(IMP.SILENT)
  or [recompile from the source code](@ref installation_source) passing
  [-DIMP_MAX_LOG=SILENT to cmake](@ref cmake_further) (the latter will be
  more efficient).
* Turn off runtime checks: call IMP.set_check_level(IMP.USAGE) or
  IMP.set_check_level(IMP.NONE). Proceed very carefully, since with
  runtime checks turned off, %IMP will not warn you if you do something
  foolish - be sure to test your protocol first! Again, this will be more
  efficient if you
  [recompile from the source code](@ref installation_source) passing
  [the -DIMP_MAX_CHECKS option to cmake](@ref cmake_further).

## Why can't I clone a Particle?
The short answer is: "because no one knows how to do it safely". The complication is that the attributes of various particles may have relationships to one another that would be violated by the cloning process. For example, a Particle attribute in a Particle could be part of a bond, in which case the bond particle must also be cloned and the attribute in the new particle must point to the new bond, or it could be a pointer to a particle which keeps track of all of the atoms in the system, in which case the attribute value should be copied unchanged. A clone function would have no way of knowing which should be done.

We suggest that you either
* call your initialization code more than once (to create two copies of the set of particles):

        protein_0= create_protein("my_protein_name")
        protein_1= create_protein("my_protein_name")

* use the `IMP.atom.create_clone()` function to clone a molecular hierarchy.

# C++ {#faq_cpp}

## What is the difference between the two functions for adding Particle attributes?

Restraints are not allowed to add/remove/change attributes during restraint evaluation. The reason for this is that the order of evaluation of restraints is not defined and if restraints could change attributes, the score could depend on what order they were evaluated in. However, in certain cases it makes sense to cache computations in the particle so as to avoid having to recompute it next time (or in a different restraint). The (undocumented) method add_cache_attribute() provides a means to do that. The assumptions made are that
- any restraint that writes it should write an equivalent value
- it should get cleared if anything "significant" (a still vague concept) in the particle changes.

One usage of it currently is for the collision detection hierarchies used with rigid bodies. This hierarchy is computed on demand (for example when trying to evaluate a distance pair score on a pair of rigid bodies). Any time the rigid body is changed (eg the internal coordinates of a member are changed), the hierarchy gets cleared out and must be recomputed the next time.

## What is this XXXEXPORT stuff?
In %IMP we only export selected functions and classes from the dynamically linked libraries. This is required for things to work on Windows; while it is not strictly necessary on other platforms it gives slightly faster code. As a result, each class/function that is used outside of the library needs to be marked with `IMPMODULENAMEEXPORT`. The rules for doing this are as follows:

* Any class which has any methods which are implemented in a .cpp file must be declared as
   `class IMPMODULENAMEEXPORT ClassName: public BaseClass{};`
* Any function whose implementation is in a .cpp file must be declared as
   `IMPMODULENAMEEXPORT ReturnType function_name(ArgumentType arg_name);`
* Non-template functions defined entirely in a header must be declared inline and **NOT** have an `IMPMODULENAMEEXPORT`
* Template functions or classes must **NOT** have `IMPMODULENAMEEXPORT` tags

Note that the pickiest compiler about this is the Visual Studio compiler, so the fact that your code works with gcc does not guarantee it is correct.

## How do I use CPPCheck with %IMP

`cppcheck --enable=all . -I../debug/build/include --template="{file}:{line}:{severity},{id},{message}"`

## What does this mean? IMP.InternalException: Internal check failure: Particle BLAH missing required attributes for decorator MyDecorator

You are trying to decorate a particle before setting it up as an instance of the decorator. Use MyDecorator.setup_particle(blah)
and now you can decorate:
MyDecorator(blah)

# SWIG {#faq_swig}

## What files do I have to edit to connect C++ to Python?

The `swig.i-in` file that you will find under `mymodule/pyext`.

## Why is SWIG telling me that a class is not defined?

The order of the includes in the `swig.i-in` matters. If a class uses code that is in another file, then you should put the latter first (note also that SWIG does not recursively follow `#include`s inside the `.h` files you `%include`). For example, if your class EnergyTermsRestraint requires EnergyTerms these lines in the `swig.i-in` must appear in this order:

    %include "IMP/mymodule/EnergyTerms.h"
    %include "IMP/mymodule/EnergyTermsRestraint.h"

# git {#faq_git}

## What are some resources for getting started with GIT?
One of its strengths is that there are excellent references, and Google will find you good answers to almost any question you have (many on [Stack Overflow](https://stackoverflow.com)). Some good overviews that %IMP users have liked can be found at:
* [An introduction to GitHub](http://kbroman.org/github_tutorial/)
* [The official Git tutorial](https://git-scm.com/book/en/v1/Getting-Started)
* [A light-weight explanation of most commands and workflows](https://www.atlassian.com/git/)
* a [git-flow teaser](http://jeffkreeftmeijer.com/2010/why-arent-you-using-git-flow/)
* the full [git-flow description](http://nvie.com/posts/a-successful-git-branching-model/) with its graphical scheme
* [10 Things I Hate About Git](https://stevebennett.me/2012/02/24/10-things-i-hate-about-git/)

In addition, help for any git command can be found by doing `git help <command>`.

## What are the key git commands to use?

Show the branches in the log
`git log --graph --all --decorate`

Show which words change in the diff
`git diff --word-diff`  

Clean out unknown files from repository
`git clean -f -d -x`

Use a nice interface when there are merge conflicts
`git mergetool`
