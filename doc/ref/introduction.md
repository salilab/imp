Introduction {#introduction}
============

[TOC]

## Understanding what is going on ## {#introduction_understanding}

IMP provides two sorts of tools to help you understand what is going on when you write a script. Both logging and checks are disabled if you use a fast build, so make sure you have access to a non-fast build.

### Logging ### {#introduction_logging}

Many operations in IMP can print out log messages as they work, allowing one to see what is being done. The amount of logging can be controlled globally by using IMP::base::set_log_level() or for individual objects by calling, for example `model.set_log_level(IMP.base.VERBOSE)`.

### Runtime checks ### {#introduction_checks}

IMP implements lots of runtime checks to make sure both that it is used properly and that it is working correctly. These can be turned on and off globally using IMP::base::set_check_level() or for individual objects.

# Where to go next # {#introduction_next}

Probably the best thing to do next is to read the [kernel/nup84.py](kernel_2nup84_8py-example.html) example.
