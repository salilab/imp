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
from SCons import Script

# We need scons 0.98 or later
EnsureSConsVersion(0, 98)

# Set up build environment:
vars = Variables('config.py')
scons_tools.add_common_variables(vars, "imp")
env = scons_tools.MyEnvironment(variables=vars,
                                tools=["default", "swig"],
                                toolpath=["scons_tools"])
env['IMP_BUILD_SUMMARY']=""
unknown = vars.UnknownVariables()
if unknown:
    print "Unknown variables: ", unknown.keys()
    print "Use 'scons -h' to get a list of the accepted variables."
    Exit(1)


if env.get('repository', None) is not None:
    Repository(env['repository'])

env.AddMethod(scons_tools.imp_module.IMPModuleBuild)
env.AddMethod(scons_tools.application.IMPCPPApplication)
env.AddMethod(scons_tools.application.IMPApplicationTest)
env.Append(BUILDERS={'IMPApplicationRunTest': scons_tools.test.UnitTest})
env['IMP_MODULES_ALL']=[]
scons_tools.boost.configure_check(env, '1.33')
scons_tools.cgal.configure_check(env)
scons_tools.swig.configure_check(env)
scons_tools.modeller_test.configure_check(env)
scons_tools.endian.configure_check(env)
scons_tools.doxygen.configure_check_doxygen(env)
scons_tools.doxygen.configure_check_dot(env)
scons_tools.gcc.configure_check_visibility(env)
scons_tools.gcc.configure_check_hash(env)

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
      '[kernel,module-name]-test' to test all modules, the kernel, or a particular module
      '[kernel, module-name]-test-examples' to test the examples for a particular module or the kernel
      'all' to build and test everything (and clean up everything in conjunction with '-c')

Infrequently changing settings can be stored in a 'config.py' file in the build directory. An
example is provided in tools/example-config.py.
""")

# Make these objects available to SConscript files:
Export('env')
#, 'get_pyext_environment', 'get_sharedlib_environment')

env.Append(BUILDERS = {'CheckStandards':scons_tools.standards.CheckStandards})
# Check code for coding standards:
standards = env.CheckStandards(target='standards.passed',
                               source=scons_tools.standards.get_standards_files(env))


env.Alias('standards', standards)
env.AlwaysBuild(standards)

SConscript('kernel/SConscript')

SConscript('modules/SConscript')
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


if env.GetOption('clean') or env.GetOption('help'):
    pass
else:
    if env.get('COMPILER_OK', None) == None:
        Exit("""
No working compiler found. Please make sure that g++ or another
compiler recognized by scons can be found in your path and that all
the passed compiler options (cxxflags, linkflags) are correct.
""")
    if env.get('BOOST_VERSION', None) == None:
        Exit("""
Boost version is required to build IMP, but it could not be found on your system.

In particular, if you have Boost installed in a non-standard location, please use the 'includepath' option to add this location to the search path.  For example, a Mac using Boost installed with MacPorts will have the Boost headers in /opt/local/include, so edit (or create) config.py and add the line

includepath='/opt/local/include'

You can see the produced config.log for more information as to why boost failed to be found.
""")

def _display_build_summary(target, source, env):
    print "Build of",", ".join(["\""+str(x)+"\"" for x in BUILD_TARGETS if x != "summary"]),"completed."
    for x in env['IMP_BUILD_SUMMARY']:
        print "  "+x
    if env['python']:
        print "  Python support enabled."
    else:
        print "  Python support disabled."
    if env['HAS_MODELLER']:
        print "  Modeller support enabled."
    else:
        print "  Modeller support disabled."
    if env["BOOST_LIBS"]:
        print "  Boost libraries enabled."
    else:
        print "  Boost libraries disabled."
    open(target[0].abspath, "w").write("done")
def _print_config_cpp(target, source, env):
    pass

BuildSummary = Builder(action=Action( _display_build_summary,
                                      _print_config_cpp))
env.Append(BUILDERS={'_BuildSummary': BuildSummary})
#print env.Alias("all")
#for a in env.Alias("all"):
#    print a.name
buildsummary= env._BuildSummary(target="build/tmp/build_summary.passed", source=[])
env.Alias("summary", [buildsummary])
for p in BUILD_TARGETS:
    if p != "summary":
        if len(Glob(p)) >0:
            env.Depends(buildsummary, p)
        else:
            env.Depends(buildsummary, env.Alias(p))
if len(BUILD_TARGETS) ==0:
    env.Depends(buildsummary, "all")
    BUILD_TARGETS.append("all")
#if "summary" not in COMMAND_LINE_TARGETS:
#   COMMAND_LINE_TARGETS.append("summary")
try:
    Script._Add_Targets( [ 'summary' ] )
except:
    print "Unable to add summary target."
    pass
env.AlwaysBuild("build/tmp/build_summary.passed")
