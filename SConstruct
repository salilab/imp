# Include IMP build utility functions:
import scons_tools
import scons_tools.boost
import scons_tools.cgal


# We need scons 0.98 or later
EnsureSConsVersion(0, 98)

# Set up build environment:
vars = Variables('config.py')
scons_tools.add_common_variables(vars, "imp")
vars.Add(BoolVariable('cgal', 'Whether to use the CGAL package', True))
env = scons_tools.MyEnvironment(variables=vars, require_modeller=False,
                    tools=["default", "docbook", "swig",
                           "imp_module"],
                    toolpath=["scons_tools"])
unknown = vars.UnknownVariables()
if unknown:
    print "Unknown variables: ", unknown.keys()
    print "Use 'scons -h' to get a list of the accepted variables."
    Exit(1)

# We need SWIG 1.3.34 or later
if not env.GetOption('clean') and not env.GetOption('help'):
    env.EnsureSWIGVersion(1, 3, 34)

if env.get('repository', None) is not None:
    Repository(env['repository'])


scons_tools.boost.configure_check(env, '1.33')
scons_tools.cgal.configure_check(env)

conf=env.Configure(config_h="kernel/include/internal/config.h")
if env['CGAL_LIBS'] != ['']:
    conf.Define("IMP_USE_CGAL")

if not env.get('deprecated', "True"):
    conf.Define('IMP_NO_DEPRECATED')
conf.Finish()
env.AlwaysBuild("build/include/IMP/internal/config.h")

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

# Check code for coding standards:
standards = env.Command("standards", "SConstruct", "tools/check-standards.py")
env.AlwaysBuild(standards)

# Subdirectories to build:
bin = SConscript('bin/SConscript')
Export('bin')
env.IMPModuleSetup('kernel', module_suffix="", module_include_path="IMP",
                   module_src_path="kernel", module_preproc="IMP", module_namespace="IMP")
SConscript('build/SConscript')
SConscript('modules/SConscript')
SConscript('applications/SConscript')
# This must be after the other SConscipt calls so that it knows about all the generated files
SConscript('doc/SConscript')

env.Alias(env.Alias('test'), [env.Alias('examples-test')])

Clean('build', ['build/tmp/',
                'build/include',
                'build/lib'])
Clean('all', Glob('scons_tools/*.pyc')\
                          + Glob('tools/*.pyc')\
                          + Glob('bin/*.pyc'))

# bin script first requires kernel libraries to be built:
#env.Depends(bin, [src, pyext])

# Build the binaries by default:
env.Default([bin])
