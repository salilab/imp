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
covers several useful commands for working with `git subtree`.

As an %example, a typical workflow for working with the IMP::pmi module is:

 - `git clone` %IMP as per usual
 - Work on the PMI module in `modules/pmi`
 - Use regular `git` commands `git add`, `git commit` etc. to record changes
 - Periodically push them to the PMI repository (_not_ the main %IMP repository) by running (in the top level %IMP directory)

    `git subtree push --prefix=modules/pmi git@github.com:salilab/pmi.git develop`

   - *Important*: be careful not to also push these commits to %IMP. You
     may need to use `git reset HEAD~N` (where `N` is the number of commits
     you made) to remove them from %IMP (or, if you made both PMI commits
     and regular %IMP commits, use `git rebase -i` to delete just the PMI
     commits).  Use `git subtree pull` (below) to incorporate PMI changes
     into %IMP.
   - `git subtree push` is very slow, since it has to examine every commit
     in the %IMP repository.
   - If this fails with `fatal: bad object`, then it needs you to do a
     `git subtree pull` first.

 - To incorporate changes from the PMI repository into %IMP, use

    `git subtree pull --squash -m "Get latest PMI." --prefix=modules/pmi git@github.com:salilab/pmi.git develop`

   - This will condense all of the PMI changes into a single %IMP
     commit (`--squash`) then merge that into %IMP itself (so you'll always end
     up with two commits in %IMP, regardless of how many were made to PMI).
     *IMPORTANT*: check the squashed commit with `git show` before you push;
     sometimes `subtree` gets confused and makes an enormous commit that touches
     every file in %IMP. If this happens, squash the PMI commits manually
     (e.g. with `git cherry-pick --strategy=recursive -Xsubtree=modules/pmi/`
     and `git rebase -i`) and make sure your final
     commit message looks like the standard `subtree` one (notably, that it
     includes a `git-subtree-split:` line).

Note that since `git subtree` commands can be very slow, if you will be working
a lot with a subtree it is probably faster to checkout its own repository
directly and then use regular `git push` and `git pull` to work inside it.
(For example, you could checkout the PMI repository into `~/pmi`, then
temporarily replace the subtree in %IMP with a symlink to the real repository
with something like `cd ~/imp/imp/modules; mv pmi pmi.old; ln -sf ~/pmi pmi`.
Move back `pmi.old` when you're done, then use `git subtree pull` to incorporate
the PMI changes into %IMP.)
