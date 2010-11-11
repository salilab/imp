# include IMP build utility functions:

import scons_tools
import scons_tools.dependency
import scons_tools.dependency.boost
import scons_tools.dependency.swig
import scons_tools.standards
import scons_tools.dependency.endian
import scons_tools.dependency.gcc
import scons_tools.dependency.modeller_test
import scons_tools.dependency.doxygen
import scons_tools.application
import scons_tools.test
import scons_tools.config_py
import scons_tools.build_summary
import scons_tools.variables
import sys
import atexit
from SCons import Script

# We need scons 0.98 or later
EnsureSConsVersion(0, 98)

# Set up build environment:
vars = Variables(files=[File('#/config.py').abspath])
scons_tools.variables.add_common_variables(vars, "imp")
env = scons_tools.IMPEnvironment(variables=vars,
                                 tools=["default", "swig"],
                                 toolpath=["scons_tools"])
env['IMP_ENABLED']=[]
env['IMP_DISABLED']=[]
env['IMP_BUILD_SUMMARY']=""
env['IMP_CONFIGURATION']=[]
env['IMP_VARIABLES']=vars

Export('env')

if env.get('repository', None) is not None:
    Repository(env['repository'])

env['IMP_MODULES_ALL']=[]

if not env.GetOption('help'):

    env.AddMethod(scons_tools.application.IMPApplicationTest)
    env.Append(BUILDERS={'IMPApplicationRunTest': scons_tools.test.UnitTest})
    if not env.GetOption('clean'):
        if not env.get('COMPILER_OK', None):
            Exit("""
No working compiler found. Please make sure that g++ or another
compiler recognized by scons can be found in your path and that all
the passed compiler options (cxxflags, linkflags) are correct.
""")

    scons_tools.dependency.boost.configure_check(env, '1.33')
    if not env.GetOption('clean'):
        if env.get('BOOST_VERSION', None) == None:
            Exit("""
Boost version is required to build IMP, but it could not be found on your system.

In particular, if you have Boost installed in a non-standard location, please use the 'includepath' option to add this location to the search path.  For example, a Mac using Boost installed with MacPorts will have the Boost headers in /opt/local/include, so edit (or create) config.py and add the line

includepath='/opt/local/include'

You can see the produced config.log for more information as to why boost failed to be found.
""")

scons_tools.dependency.add_external_library(env, "ANN", "ANN",
                                       "ANN/ANN.h")
scons_tools.dependency.add_external_library(env, "NetCDF", ["netcdf_c++", 'netcdf'],
                                       "netcdfcpp.h")
scons_tools.dependency.add_external_library(env, "FFTW3", "fftw3",
                                       "fftw3.h")
scons_tools.dependency.add_external_library(env, "CGAL", ["CGAL",'gmp', 'mpfr', 'm'],
                                       ['CGAL/Gmpq.h', 'CGAL/Lazy_exact_nt.h'],
                                       body='CGAL_assertion(1); CGAL::Lazy_exact_nt<CGAL::Gmpq> q;',
                                       extra_libs=['boost_thread-mt', 'boost_thread', 'pthread'])
scons_tools.dependency.add_external_library(env, "Boost.FileSystem", "boost_filesystem",
                                       'boost/filesystem/path.hpp',
                                       extra_libs=['libboost_system'],
                                       alternate_name='boost_filesystem-mt')
scons_tools.dependency.add_external_library(env, "Boost.ProgramOptions", "boost_program_options",
                                       'boost/program_options.hpp',
                                       extra_libs=['libboost_system'],
                                       alternate_name='boost_program_options-mt')



if not env.GetOption('help'):
    # various flags depending on compiler versions and things
    scons_tools.dependency.swig.configure_check(env)
    scons_tools.dependency.boost.configure_tr1_check(env)
    scons_tools.dependency.modeller_test.configure_check(env)
    scons_tools.dependency.endian.configure_check(env)
    scons_tools.dependency.doxygen.configure_check_doxygen(env)
    scons_tools.dependency.doxygen.configure_check_dot(env)
    scons_tools.dependency.gcc.configure_check_visibility(env)
    scons_tools.dependency.gcc.configure_check_hash(env)
    # Make these objects available to SConscript files:
    scons_tools.standards.setup_standards(env)

    env.Append(BUILDERS={'IMPConfigPY':scons_tools.config_py.ConfigPY})

SConscript('kernel/SConscript')
SConscript('modules/SConscript')
SConscript('applications/SConscript')


if not env.GetOption('help'):
    # This must be after the other SConscipt calls so that it knows about all the generated files
    if env['doxygen']:
        SConscript('doc/SConscript')
    SConscript('tools/SConscript')

    env.Alias(env.Alias('test'), [env.Alias('examples-test')])

    Clean('all', ['build'])
    Clean('all', Glob('scons_tools/*.pyc')\
              + Glob('tools/*.pyc'))

    env.Alias(env.Alias('all'), 'tools')
    env.Default(env.Alias('all'))


    unknown = vars.UnknownVariables()
    # scons has a bug with command line arguments that are added late
    if unknown:
        really_unknown=[]
        for u in unknown.keys():
            if u not in [scons_tools.dependency.get_dependency_string(x) for x in env.get_all_known_dependencies()]:
                really_unknown.append(u)
        if len(really_unknown) >0:
            print >> sys.stderr, "\n\nUnknown variables: ", " ".join(unknown.keys())
            print >> sys.stderr, "Use 'scons -h' to get a list of the accepted variables."
            Exit(1)
    scons_tools.build_summary.setup(env)
    config_py=env.IMPConfigPY(target=["#/config.py"],
                              source=[env.Value("#".join(env['IMP_CONFIGURATION']))])

else:
    tenv= Environment(variables=vars)
    Help("""
    Available command-line options:
    (These can also be specified in regular Python syntax by creating a file
    called 'config.py' in this directory.)
    """)
    Help(vars.GenerateHelpText(tenv))

    Help("""
    Type: 'scons' to build the IMP kernel and all configured modules (i.e. those
                  with no unmet dependencies);
          'scons test' to run unit tests for the kernel and all configured modules;
          'scons install' to install the kernel and all configured modules;
          'scons doc{-install}' to build (and optionally install) doc

    Other useful targets:
          '[kernel,modulename]-test' to test all modules, the kernel, or a particular module
          '[kernel, modulename]-test-examples' to test the examples for a particular module or the kernel
          'all' to build and test everything (and clean up everything in conjunction with '-c')

    Infrequently changing settings can be stored in a 'config.py' file in the build directory. An example is provided in tools/example-config.py.
    """)
