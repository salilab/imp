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
