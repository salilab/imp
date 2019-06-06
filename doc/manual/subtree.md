git submodules and subtrees {#subtree}
===========================

%IMP includes several pieces of code that are actually maintained in
separate git repositories on [GitHub](https://github.com). These include
the [RMF library](https://github.com/salilab/rmf) and the
[IMP::pmi module](https://github.com/salilab/pmi). There are two main ways
to achieve this with git: `git submodule` and `git subtree`. Modern versions
of %IMP use `git subtree` (we previously used `submodule`; see the
[issue on GitHub](https://github.com/salilab/imp/issues/876) for a discussion).
`subtree` is a little harder for developers to work with than `submodule`,
but is much easier for end users (no need to run any setup scripts after
getting %IMP, forks of the %IMP repository work, and GitHub's "Download ZIP"
feature works).

[This blog post](http://blogs.atlassian.com/2013/05/alternatives-to-git-submodule-git-subtree/)
covers several useful commands for working with `git subtree`. However,
`git subtree` commands are hard to use correctly can be very slow, so it's
generally easier to use this procedure when working on PMI:

 - Build %IMP as per usual, say in `~/imp/build/`.
 - Don't touch the files in `~/imp/modules/pmi`; instead `git clone` the PMI
   repository into a new directory, separate from %IMP, say `~/pmi/`.
 - Make a new directory for the PMI build, e.g. `~/pmi/build`, then
   do an *out-of-tree* build of PMI with:

    `cmake .. -G Ninja -DIMP_DIR=~/imp/build/ && ninja`
   
 - Use regular `git` commands `git add`, `git commit`, `git push` etc. to
   record changes to the PMI repository. Test changes using `ctest` or
   the `setup_environment.sh` script in `~/pmi/build`. 
 - To incorporate changes from the PMI repository into %IMP, first push them
   to the PMI repository, then use the utility script (in the IMP clone)

    `tools/git/update-pmi.sh`

   - This will squash all of the PMI changes into a single %IMP
     commit, which can then be `git push`ed into the %IMP repository in the
     normal way.
