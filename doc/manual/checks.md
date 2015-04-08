Runtime checks {#checks}
==============

%IMP implements lots of runtime checks to ensure that
 - it is being used correctly (`USAGE` checks); for example, input parameters
   are checked to make sure they are within the expected range.
 - it is working correctly (`INTERNAL` or debug checks).

The maximum check level is set [when IMP is built](@ref cmake_config). It is
generally recommended to build with only `USAGE` checks, since `INTERNAL`
checks typically result in much slower code.

Runtime checks can also be turned on and off
globally at runtime using IMP::base::set_check_level()
or for individual objects.

Note that while it is possible to turn off `USAGE` checks it is generally
not recommended. If %IMP runs markedly more slowly with `USAGE` checks turned
on, then this is a bug and [it should be reported](@ref bugs).
