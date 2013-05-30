# OpenMP

# Multithreading with OpenMP # {#openmp}
[TOC]

IMP supports multithreaded evaluation using [OpenMP](http://openmp.org/wp/) (version 3.0 or later). OpenMP 3.0 is supported by recent versions of `gcc`, but not `clang++`. Multithreaded evaluation uses OpenMP tasks, with each IMP::kernel::Restraint or IMP::kernel::ScoreState being made into one or more tasks that are executed independently. IMP uses the information in the IMP::kernel::DepdendencyGraph to automatically determine which IMP::kernel::ScoreState objects can be used in parallel. That is, two score states are independent if neither depend on data written by the other.

# Activating OpenMP ## {#openmp_activating}

To use OpenMP with IMP, turn it on during compilation by adding the following
flag to your `cmake` invocation:

      -DCMAKE_CXX_FLAGS=-fopenmp

# Controlling OpenMP ## {#openmp_controlling}

The functions IMP::base::get_number_of_threads(), IMP::base::set_number_of_threads(), and the RAII class IMP::base::SetNumberOfThreads can be used to control OpenMP evaluation, as can the command line flag "number_of_threads" in executables that use IMP's flags suport (see base/flags.h).

# Writing code with OpenMP ## {#openmp_writing}

If you want to parallelize code, see the helper macros in base/thread_macros.h. In particular, you can define a new task with IMP_TASK() and make your executable multithreaded with IMP_THREADS(). If you define tasks yourself, be sure to add

      #pragma omp taskwait

to make sure all the tasks are finished before returning. In general, you probably need to read various OpenMP documentation to get things to work right. IMP::kernel::Restraints that want to create tasks should implement IMP::kernel::Restraint::do_add_score_and_derivatives() rather than IMP::kernel::Restraint::unprotected_evaluate() as it is hard to properly get the return value back otherwise.

Examples will come.
