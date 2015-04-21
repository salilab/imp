Logging {#logging}
=======

Many operations in IMP can print out log messages as they work, allowing one
to see what is being done. The amount of logging can be controlled globally
by using IMP::set_log_level() or for individual objects by calling,
for example `model.set_log_level(IMP.VERBOSE)`.

The maximum amount of logging can also be set
[when IMP is built](@ref cmake_config). This can result in slightly faster
code than configuring it at runtime.
