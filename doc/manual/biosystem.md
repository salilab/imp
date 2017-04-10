Applying %IMP to a new biological system {#biosystem}
========================================

We have already applied %IMP to solve the structures of many novel biological
systems, listed on the [biological systems page](https://integrativemodeling.org/systems/).
Each system on that page includes all of the files needed to reproduce the
results in the accompanying publication. For example, the list includes the
[modeling example from earlier in this manual](@ref rnapolii_stalk), as well
as [modeling of the Nup84 subcomplex of the Nuclear Pore Complex](https://salilab.org/nup84). Each system is periodically rerun with the latest version of %IMP
to make sure that it still works correctly.

To apply %IMP to a new biological system, you are welcome to use one of the
existing systems, such as the [Nup84 model](https://salilab.org/nup84),
as your template - or you can write from scratch using the basic %IMP classes
and/or the IMP::pmi higher level interface. In either case, we strongly
recommend that you manage your application as a GitHub repository so that
 - others can reproduce your published work
 - changes to the protocol can be documented or rolled back if necessary
 - your system can be added to [our list](https://integrativemodeling.org/systems/), so that we can test newer versions of %IMP to make sure we don't break something

We recommend the following contents for your repository (see the
[Nup84 repository](https://github.com/integrativemodeling/nup84)
for an example):

 - subdirectories containing
   - your modeling protocol (generally one or more Python scripts)
   - input files (e.g. PDB files, EM density maps, lists of crosslinks),
     especially if these files aren't in a database somewhere already
   - outputs (trajectories, clusters, analysis). Where this isn't possible
     due to size, we can host the larger files, such as trajectories, elsewhere
     (e.g. as a dataset in [Zenodo](https://zenodo.org)) and link to them
     from the repository.
 - a top-level `%README.md` file describing the system and explaining how to
   run the protocol.
 - a top-level `LICENSE` file with the license for the data files and scripts.
   This doesn't need to be the same license (LGPL/GPL) that %IMP uses; in fact,
   for data files one of the [Creative Commons](https://creativecommons.org/)
   licenses probably makes more sense. We recommend the
   [CC BY-SA license](https://creativecommons.org/licenses/by-sa/4.0/)
   which allows anybody to use and modify the data under the same terms, as
   long as they cite the original work.
 - a `test` directory containing one or more Python scripts with names starting
   with `test`. It should be
   possible to run these scripts without any "special" setup (e.g. they should
   not require any input arguments or environment variables, or use
   hard-coded paths). These scripts should run as much of your modeling
   protocol as possible, and ideally test the results (e.g. by comparing models
   against 'known good' clusters). Each script should simply exit with a
   non-zero exit code (e.g. by raising an exception) if something failed; one
   easy way to do this nicely is to use Python's
   [unittest](https://docs.python.org/2/library/unittest.html) module. The
   tests should run in a "reasonable" amount of time (no more than 48 hours)
   on a single processor. If this is not enough time to run your entire
   protocol, run only a representative subset
   (e.g. the Nup84 modeling test passes a `--test` option to the modeling
   script, which has it perform fewer iterations of sampling).
 - to add your system to [our list](https://integrativemodeling.org/systems/)
   it will also need a `metadata` subdirectory (also
   [contact us](https://integrativemodeling.org/contact.html) to let us know
   about it).
   This should contain two files:
   - `thumb.png`: a small image used to represent your system on the page.
   - `metadata.yaml`: a file in [YAML](http://yaml.org/) format specifying
     (see also the [Nup84 example](https://github.com/integrativemodeling/nup84/blob/master/metadata/metadata.yaml)):
     - `title`: a short descriptive name for your system
     - `tags`: a list of tags to group your system with others that use
       similar methods or input data
     - `pmid`: the PubMed ID of the accompanying publication
     - `prereqs`: a list of any non-standard packages that are needed
       (in addition to %IMP and Python's standard library) to run the scripts
     - `runtime`: upper limit to the time the tests will take to run
     - `build`: which type of %IMP build to run the tests with
       (`release`, `fast` or `debug`); `release` is generally recommended
