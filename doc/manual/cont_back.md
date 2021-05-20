Contributing code back to IMP {#cont_back}
=============================

In order to be shared with others as part of the %IMP distribution, code
needs to be of higher quality and more thoroughly vetted than typical
research code. All code contributed to %IMP
- must follow the [code conventions](@ref code_conventions)
- should follow general good [C++ programming practices](@ref good_practice)
- must have [unit tests](@ref testing) that all pass when run with `ctest`
- must have [documentation](@ref documenting) that builds without warnings
- must build on all supported compilers (roughly, recent versions of `gcc`,
  `clang++` and `Visual C++`) without warnings
- should have [examples](@ref write_examples) that all work when run
  with `ctest`

Once your code is ready to go, [open a pull request](@ref devsetup) to get
the changes from [your fork](@ref devsetup) back into the main repository.

\note that all development occurs in the `develop` branch. No development
is done directly in the `main` branch - it is updated for each release by
merging from the `develop` branch, as per the
[gitflow branching model](http://nvie.com/posts/a-successful-git-branching-model/).
If your change is a bug fix that should also patch the last release, once your
code is in `develop` open an [issue on GitHub](https://github.com/salilab/imp/issues)
assigned to `benmwebb` and connected to the milestone for the next point
release (e.g. a milestone with a name like `%IMP 2.0.1 point release`).
