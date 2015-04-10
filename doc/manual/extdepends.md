Adding external dependencies {#extdepends}
============================

You can have an %IMP module depend on some external library.
See the files `modules/base/dependencies.py` and
`modules/base/dependency/Log4CXX.description` for examples.

Think very carefully before introducing a new external dependency. Each
additional dependency makes it harder for people to use your code.
If you do need to add a dependency, it needs to be open source and available
under a suitably permissive license (for %example if it is available under
the GPL then you cannot license your module as LGPL, but will need to also
make it GPL).

Generally if you need to add a new dependency you should probably also put
your code in a new module, rather than adding it to an existing module. That
way, people that elect not to install that dependency will only be deprived
of your code, not of the existing module.
