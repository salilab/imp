Import('env')
import scons_tools.dependency

# Uncomment if you need to check for an external library on the system when
# building this module.
#
# If the library is needed to build your module,
# specify required_dependencies=['ExampleLib'] in the IMPModuleBuild call below.
#
# If it is only optional, use 'optional_dependencies' instead, and protect
# C++ code that optionally uses that dependency
# with "#ifdef IMP_USE_EXAMPLELIB".
#
# See IMP.gsl for a working example of an external library dependency
# (it requires GSL, the GNU Scientific Library).
#
#scons_tools.dependency.add_external_library(env, "ExampleLib",
#                                            "example_lib_name",
#                                            "example_lib_header.h")

scons_tools.dependency.boost.add_boost_library(env, "ProgramOptions", "program_options", 'boost/program_options.hpp',
                                              extra_boost_libs=["system"])

env.IMPModuleBuild(version='SVN',
                   # The required modules are the other IMP modules that must be built to use this one
                   required_modules=['container', 'core', 'atom', 'isd2', 'rmf', 'saxs', 'statistics'],
                   required_dependencies=['Boost.ProgramOptions'])
