Applying IMP to a new biological system {#biosystem}
=======================================

We have already applied %IMP to solve the structures of many novel biological
systems, listed on the [biological systems page](http://integrativemodeling.org/systems/).
Each system on that page includes all of the files needed to reproduce the
results in the accompanying publication. For example, the list includes the
[modeling example from earlier in this manual](@ref rnapolii_stalk), as well
as [modeling of the Nup84 subcomplex of the Nuclear Pore Complex](http://salilab.org/nup84). Each system is periodically rerun with the latest version of %IMP
to make sure that it still works correctly.

To apply %IMP to a new biological system, you are welcome to use one of the
existing systems, such as the [Nup84 model](http://salilab.org/nup84)
as your template - or you can write from scratch using the basic %IMP classes
and/or the IMP::pmi higher level interface. In either case, we strongly
recommend that you manage your application as a GitHub repository so that
 - others can reproduce your published work
 - changes to the protocol can be documented or rolled back if necessary
 - your system can be added to [our list](http://integrativemodeling.org/systems/)

We recommend the following contents for your repository (see the
[Nup84 repository](https://github.com/integrativemodeling/nup84)
for an example):

 - subdirectories containing your modeling protocol (generally one or more
   Python scripts), input files (e.g. PDB files, EM density maps, lists of
   crosslinks), and outputs (where possible, due to size; it may not be feasible
   to upload all of your output models, for example, but only cluster
   representatives).
 - a top-level `README.md` file describing the system and explaining how to
   run the protocol.
 - a `test` directory containing one or more Python scripts. It should be
   possible to run these scripts without any "special" setup (e.g. they should
   not require any input arguments or environment variables, or use
   hard-coded paths). These scripts should run as much of your modeling
   protocol as possible, and ideally test the results (e.g. comparing models
   against 'known good' clusters). Each script should simply exit with a
   non-zero exit code (e.g. by raising an exception) if something failed. One
   easy way to do this nicely is to use Python's
   [unittest](https://docs.python.org/2/library/unittest.html) module. The
   tests should run in a "reasonable" amount of time (no more than 48 hours)
   on a single processor. If this is not enough time to run your entire
   protocol, run only a subset (e.g. the Nup84 modeling test passes a `--test`
   option to the modeling script, which has it perform fewer iterations of
   sampling).
 - to add your system to [our list](http://integrativemodeling.org/systems/)
   it will also need a `metadata` subdirectory (and also
   [contact us](http://integrativemodeling.org/contact.html)).
   This should contain two files:
   - `thumb.png`: a small image used to represent your system on the page.
   - `metadata.yaml`: a file in [YAML](http://yaml.org/) format specifying
     - `title`: a short descriptive name of your system
     - `tags`: a list of tags to sort your system with others that use
       similar methods or input data
     - `pmid`: the PubMed ID of the accompanying publication
     - `prereqs`: a list of any non-standard packages that are needed
       (in addition to %IMP and Python's standard library) to run the scripts
     - `runtime`: upper limit to the time the tests will take to run
     - `build`: which type of %IMP build to run the tests with
       (`release`, `fast` or `debug`)
