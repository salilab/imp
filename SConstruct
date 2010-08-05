# include IMP build utility functions:
import scons_tools
import scons_tools.boost
import scons_tools.cgal
import scons_tools.netcdf
import scons_tools.ann
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
scons_tools.netcdf.configure_check(env)
scons_tools.ann.configure_check(env)
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
      '[kernel,modulename]-test' to test all modules, the kernel, or a particular module
      '[kernel, modulename]-test-examples' to test the examples for a particular module or the kernel
      'all' to build and test everything (and clean up everything in conjunction with '-c')

Infrequently changing settings can be stored in a 'config.py' file in the build directory. An example is provided in tools/example-config.py.
""")

# Make these objects available to SConscript files:
Export('env')
#, 'get_pyext_environment', 'get_sharedlib_environment')

scons_tools.standards.setup_standards(env)

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
    print "Dependencies:"
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
    disabledm=[]
    enabledm=[]
    for m in env['IMP_MODULES_ALL']:
        if not env.get(m+"_ok", False):
            disabledm.append(m)
        else:
            enabledm.append(m)
    print "Enabled modules:"
    print "  ",
    for x in enabledm:
        print x,
    print
    if len(disabledm) >1:
        print "Disabled modules (due to missing dependencies):"
        print "  ",
        for x in disabledm:
            print x,
        print
    elif len(disabledm) ==1:
        print "IMP."+disabledm[0] + " was missing dependencies and disabled."
    from SCons.Script import GetBuildFailures
    abf=GetBuildFailures()
    if abf:
        print "Errors building:"
        for bf in abf:
            print "  "+_bf_to_str(bf)
    else:
        pass
        #print "Built:",", ".join(["\""+str(x)+"\"" for x in BUILD_TARGETS]),"completed."


atexit.register(_display_build_summary, env)

env.AlwaysBuild("build/tmp/build_summary.passed")
