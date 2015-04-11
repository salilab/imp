Developer setup {#devsetup}
===============

The %IMP source code and documentation is stored
[at GitHub](https://github.com/salilab/imp). For some basic contributions
to %IMP, such as fixing typos in
[this documentation](https://github.com/salilab/imp/tree/develop/doc/manual),
you may be able to use GitHub's web interface. However, for most developer
tasks, you will need to [work with git](@ref faq_git) and make a copy of the
%IMP code on your own machine. This follows a procedure very similar to that
used to [build IMP from source](@ref installation_download).

 1. Make a copy (fork) of the [IMP repository](https://github.com/salilab/imp).
    There is an [excellent help page at GitHub](https://help.github.com/articles/fork-a-repo/)
    that explains this in detail.

 2. Build %IMP [as previously described](@ref installation_download) - the only
    difference is you will want to clone your fork, not the main %IMP
    repository. You will almost always want to use the `develop` branch, as that
    is where all new %IMP development occurs.

 3. Make your changes to %IMP (described in more detail below).

 4. When you are finished with your changes,
    [open a pull request](https://help.github.com/articles/using-pull-requests/)
    to get it incorporated into the main %IMP repository.

\note There are other ways of working with %IMP. For %example, if you are
      working in the Sali lab, in most cases you can work directly on the
      main %IMP repository rather than making a fork (speak to an existing
      developer to make sure.) You may also choose to develop a single %IMP
      module in isolation from %IMP itself, in its own repository (as is
      done for [IMP::pmi](https://github.com/salilab/pmi)).
