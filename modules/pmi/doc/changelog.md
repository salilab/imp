PMI changelog {#pmi_changelog}
=============

*News May 7 2016 - update 2*

Here is a list of recent feature additions and bug fixes in PMI2:
* Add colors with Molecule.add_representation(color=XXX). You can pass an (R,G,B) tuple or a named [color from chimera](https://www.cgl.ucsf.edu/chimera/docs/UsersGuide/colortables.html)
* Build order has been fixed so now fragments should be sorted by residue index in RMF files.
* Clustering will detect ambiguity and find the lowest distance of all permutations of molecule copies.
* Added HelixRestraint for maintaining secondary structure when doing MD
* Ideal helices now work with Molecule.add_representation(ideal_helix=True). Can grab a list of them with Molecule.get_ideal_helices().
* New utility functions: IMP.pmi.tools.shuffle_configuration(), DegreesOfFreedom.optimize_flexible_beads()
* The BuildSystem macro now supports copies (and colors). See the "automatic.py" example.
* Removed dependency on BioPython

--Charles

*News May 7 2016*

We have updated PMI significantly, completely rewriting the topology building tools (you may see the new code referred to as "PMI2". Instead of using the monolithic "representation" class (in representation.py) we are now using a set of modular classes with a bit more flexibility:
* Setup is now done with hierarchical classes in the Topology module: System, State, Molecule. A key improvement is that hierarchy construction (including building representations) is only done at the end, which means much faster setup time. Molecule.add_structure() lets you add PDB files and Molecule.add_representation() lets you choose from various representations including beads, densities (gaussians), and ideal helices. For selecting things to pass to these functions, you can get "TempResidues" by slicing Molecule: mol[a:b], mol.residue_range('a','b') where quotes indicate PDB-style indexing. These slices produce sets so they can be combined etc with set operations.
* Naming is different in PMI2. Before, different segments and copies of molecules had different names. Now we require that the Molecule name be the same for all domains and all copies - the name really should be the name of the sequence. To handle ambiguity, we use the Copy decorator. In PMI2, set up copies with molecule.create_copy() or molecule.create_clone() (copies can have different representation topologies, clones must be identical). You can pass copy_index to IMP.atom.Selection to get the ones you want.
* We now use the IMP.atom.Representation decorator to organize resolutions. This is an improvement over the old code because it allows fast searching of resolutions using IMP.atom.Selection (pass representation_type=IMP.atom.BALLS or IMP.atom.DENSITIES along with resolution=X).
* Movers are now handled separately in the DegreesOfFreedom class, with functions like create_rigid_body(), create_super_rigid_body(), create_flexible_beads(), setup_md(), and constraints like constrain_symmetry()
* The previous "table" format for automatic construction has been revamped. Now we use the TopologyReader to read in a text file (with format checking) and the BuildSystem macro to construct the hierarchy and degrees of freedom.
* Most restraints support the new interface, usually you'll pass the built System hierarchy instead of the old representation object.
* The ReplicaExchange0 macro works the same as before, just pass your System hierarchy, again instaed of the old representation object.
* Most analysis tools are now compatible with PMI2. 
* Lots of examples have been added, see the full list [here](https://integrativemodeling.org/nightly/doc/ref/examples.html).

Changes in IMP that are helpful in PMI:
* Utility functions like IMP.atom.get_copy_index(), IMP.atom.get_state_index(), IMP.atom.get_molecule_name() are aware of Representations and will work on, for example, Gaussians.
* IMP.atom.show_with_representations(hier) will display all the constructed representations
* RMF supports the new PMI hierarchies

--Charles

*News May 14 2014*

From Ben Webb:

PMI was added as an IMP submodule. But what might not be obvious is that a submodule is tied to a specific revision (or git hash) so doesn't automatically update whenever PMI is updated. So far I've been updating PMI in IMP on a rather ad hoc basis (essentially whenever I make changes to PMI itself), but you might want to do this too when you add new features.

It's pretty easy: if you maintain PMI outside of IMP somewhere, just go into your IMP checkout, cd modules/pmi, then `git checkout <githash>` where <githash> is the version of PMI you want to live in IMP. Then you can go back up into IMP proper and git commit/git push that change. Or you can do what I do and work on the copy of PMI that's in IMP directly rather than checking it out separately. To do that, go into modules/pmi in your IMP checkout and run `git checkout develop` to get on the PMI develop branch. Then you can run all the normal git commands (e.g. git pull) in there. (Although note that you can't push by default; that's easy to fix though by running `git remote set-url --push origin git@github.com:salilab/pmi.git`.) When you're done with your git push, you can cd up into IMP proper and commit the change in PMI githash there (without having to remember what it is).

*News April 24 2014*

The main branch is now `develop` (not `master`) and the module is included
in IMP by default.

*News February 28 2014*

The [clustering analysis](http://nbviewer.ipython.org/github/salilab/pmi/blob/master/examples/analysis/clustering_analysis.ipynb?create=1) calculates the localization densities

*News February 25 2014*

First working version of the [clustering analysis](http://nbviewer.ipython.org/github/salilab/pmi/blob/master/examples/analysis/clustering_analysis.ipynb?create=1). For the moment each cluster directory will only contain pdbs and rmfs
of the structures, but soon I'll add statistics, features and localization densities.

To have full speed install the mpi4py python library


*News February 10 2014*

Checkout the macros in macros.py

*News February 6 2014*

Added a FAQ section in the wiki. There will be ipython notebook tutorials soon.
Best IMP version so far: `ee1763c6859a29ab37415c8454d16549268d9668`
Previous versions had a bug in the RigidClosePairsFinder [bug](https://github.com/salilab/pmi/issues/19)


*News February 4 2014:*

Best IMP version so far: `b700fff93ca45ba1551c8aa9d697c805fcb126dc`
Previous versions might be considerably slow due to a [bug](https://github.com/salilab/imp/issues/724)

*News January 30 2014:*

Now the developed git branch is master and not resolution-zero.

If you want to use pmi, after you've freshly cloned it,
you don't have to checkout resolution-zero anymore:
it is the default branch that you get when you clone it.

The resolution-zero branch does not exist anymore,
it was copied into resolution-zero-old.

To see what branch you're in, run (into the pmi source code directory):

    git branch

If you want to update the code and you still are in resolution-zero branch,
just run :

    git checkout master
    git pull

Note that the interface is also changing,
so you'll probably get deprecated warnings in your standard output more and more.
`grep deprecated`  to get the deprecation warnings, which might be lost in the middle of many other messages. Change your python script according to what the warnings say.
The old version of pmi (mainly used by Peter and SJ) is
still available under the tag "v0.1". To get it:

`git checkout tags/v0.1`
