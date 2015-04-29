PMI changelog {#pmi_changelog}
=============

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
