# include IMP build utility functions:
import scons_tools
import scons_tools.boost
import scons_tools.cgal
import scons_tools.swig
import scons_tools.standards
import scons_tools.endian
import scons_tools.gcc
import scons_tools.modeller_test
import scons_tools.doxygen
import scons_tools.application
import scons_tools.test
import atexit
from SCons import Script

# We need scons 0.98 or later
EnsureSConsVersion(0, 98)

# Set up build environment:
vars = Variables('config.py')
scons_tools.add_common_variables(vars, "imp")
external_libs=[]
env = scons_tools.MyEnvironment(variables=vars,
                                tools=["default", "swig"],
                                toolpath=["scons_tools"])
env['IMP_ENABLED']=[]
env['IMP_DISABLED']=[]
env['IMP_BUILD_SUMMARY']=""

if not env.GetOption('help'):

    if env.get('repository', None) is not None:
        Repository(env['repository'])

    env.AddMethod(scons_tools.imp_module.IMPModuleBuild)
    env.AddMethod(scons_tools.application.IMPCPPApplication)
    env.AddMethod(scons_tools.application.IMPApplicationTest)
    env.Append(BUILDERS={'IMPApplicationRunTest': scons_tools.test.UnitTest})
    env['IMP_MODULES_ALL']=[]

    if not env.GetOption('clean'):
        if env.get('COMPILER_OK', None) == None:
            Exit("""
No working compiler found. Please make sure that g++ or another
compiler recognized by scons can be found in your path and that all
the passed compiler options (cxxflags, linkflags) are correct.
""")

    scons_tools.boost.configure_check(env, '1.33')
    if not env.GetOption('clean'):
        if env.get('BOOST_VERSION', None) == None:
            Exit("""
Boost version is required to build IMP, but it could not be found on your system.

In particular, if you have Boost installed in a non-standard location, please use the 'includepath' option to add this location to the search path.  For example, a Mac using Boost installed with MacPorts will have the Boost headers in /opt/local/include, so edit (or create) config.py and add the line

includepath='/opt/local/include'

You can see the produced config.log for more information as to why boost failed to be found.
""")

scons_tools.checks.handle_optional_lib(env, "ANN", "ANN",
                                       "ANN/ANN.h", vars,
                                       liblist=external_libs)
scons_tools.checks.handle_optional_lib(env, "NetCDF", "netcdf_c++",
                                       "netcdfcpp.h", vars, extra_libs=['netcdf'],
                                       liblist=external_libs)
scons_tools.checks.handle_optional_lib(env, "GSL", "gsl",
                                       "gsl/gsl_multimin.h", vars, extra_libs=['gslcblas'],
                                       liblist=external_libs)
scons_tools.checks.handle_optional_lib(env, "CGAL", "CGAL",
                                       ['CGAL/Gmpq.h', 'CGAL/Lazy_exact_nt.h'],
                                       vars,
                                       body='CGAL_assertion(1); CGAL::Lazy_exact_nt<CGAL::Gmpq> q;',
                                       extra_libs=['gmp', 'mpfr', 'm','boost_thread-mt', 'boost_thread', 'pthread'],
                                       liblist=external_libs)
scons_tools.checks.handle_optional_lib(env, "Boost.FileSystem", "boost_filesystem",
                                       'boost/filesystem/path.hpp',
                                       vars,
                                       extra_libs=['libboost_system'],
                                       alternate_name=['boost_filesystem-mt'],
                                       liblist=external_libs)
scons_tools.checks.handle_optional_lib(env, "Boost.ProgramOptions", "boost_program_options",
                                       'boost/program_options.hpp',
                                       vars,
                                       extra_libs=['libboost_system'],
                                       alternate_name=['boost_program_options-mt'],
                                       liblist=external_libs)



if not env.GetOption('help'):
    # various flags depending on compiler versions and things
    scons_tools.cgal.configure_check(env)
    scons_tools.swig.configure_check(env)
    scons_tools.modeller_test.configure_check(env)
    scons_tools.endian.configure_check(env)
    scons_tools.doxygen.configure_check_doxygen(env)
    scons_tools.doxygen.configure_check_dot(env)
    scons_tools.gcc.configure_check_visibility(env)
    scons_tools.gcc.configure_check_hash(env)

    # Make these objects available to SConscript files:
    Export('env')
    Export('vars')
#, 'get_pyext_environment', 'get_sharedlib_environment')

    scons_tools.standards.setup_standards(env)

    SConscript('kernel/SConscript')

SConscript('modules/SConscript')
if not env.GetOption('help'):
    SConscript('applications/SConscript')
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

    def _bf_to_str(bf):
        """Convert an element of GetBuildFailures() to a string
        in a useful way."""
        import SCons.Errors
        if bf is None: # unknown targets product None in list
            return '(unknown tgt)'
        elif isinstance(bf, SCons.Errors.StopError):
            return str(bf)
        elif bf.node:
            return str(bf.node)
        elif bf.filename:
            return bf.filename
        return 'unknown failure: ' + bf.errstr

    def _display_build_summary(env):
        print
        print
        found_deps=[]
        unfound_deps=[]
        for x in env['IMP_BUILD_SUMMARY']:
            print x
        if env['python']:
            found_deps.append('python')
        else:
            unfound_deps.append('python')
        for l in external_libs:
            if env[scons_tools.checks.nicename(l).upper()+"_LIBS"]:
                found_deps.append(l)
            else:
                unfound_deps.append(l)
        print "Enabled dependencies: ",", ".join(found_deps)
        print "Disabled/unfound optional dependencies: ",\
            ", ".join(unfound_deps)
        print
        disabledm=[]
        enabledm=[]
        for m in env['IMP_MODULES_ALL']:
            if not env.get(m+"_ok", False):
                disabledm.append(m)
            else:
                enabledm.append(m)
        print "Enabled modules: ", ", ".join(enabledm)
        if len(disabledm) >0:
            print "Disabled modules:", ", ".join(disabledm)
        from SCons.Script import GetBuildFailures
        abf=GetBuildFailures()
        if abf:
            print "Errors building:"
            for bf in abf:
                print "  "+_bf_to_str(bf)


    atexit.register(_display_build_summary, env)


unknown = vars.UnknownVariables()
if unknown:
    print "Unknown variables: ", unknown.keys()
    print "Use 'scons -h' to get a list of the accepted variables."
    Exit(1)
Help("""
Available command-line options:
(These can also be specified in regular Python syntax by creating a file
called 'config.py' in this directory.)
""")
Help(vars.GenerateHelpText(env))

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
