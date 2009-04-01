# Include IMP build utility functions:
from tools import *
from tools import boost, cgal

# We need scons 0.98 or later
EnsureSConsVersion(0, 98)

# Set up build environment:
vars = Variables('config.py')
add_common_variables(vars, "imp")
vars.Add(PackageVariable('embed', 'Location of the EMBED package', 'no'))
vars.Add(BoolVariable('cgal', 'Whether to use the CGAL package', True))
env = MyEnvironment(variables=vars, require_modeller=False,
                    tools=["default", "doxygen", "docbook", "swig",
                           "imp_module"],
                    toolpath=["tools"])
unknown = vars.UnknownVariables()
if unknown:
    print "Unknown variables: ", unknown.keys()
    print "Use 'scons -h' to get a list of the accepted variables."
    Exit(1)

# We need SWIG 1.3.31 or later
if not env.GetOption('clean') and not env.GetOption('help'):
    env.EnsureSWIGVersion(1, 3, 31)

boost.configure_check(env, '1.33')
cgal.configure_check(env)

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
      'scons examples' to run the examples;

Other useful targets:
      'kernel-test', 'kernel-install' to test or install the kernel;
      'modules-test', 'modules-install' to test or install ALL modules (even
          attempt to do so for those not configured);
      'foo-test', 'foo-install' to test or install the module 'foo'.
""")

# Make these objects available to SConscript files:
Export('env', 'get_pyext_environment', 'get_sharedlib_environment')

# Check code for coding standards:
standards = env.Command("standards", "SConstruct", "tools/check-standards.py")
env.AlwaysBuild(standards)

# Subdirectories to build:
bin = SConscript('bin/SConscript')
Export('bin')
SConscript('doc/SConscript')
SConscript('examples/SConscript')
(src, pyext) = SConscript('kernel/SConscript')
SConscript('build/SConscript')
SConscript('modules/SConscript')
SConscript('benchmarks/SConscript')

Clean('build', ['build/tmp/',
                'build/include',
                'build/lib'])

# bin script first requires kernel libraries to be built:
#env.Depends(bin, [src, pyext])

# Build the binaries by default:
env.Default([bin, src])
if env.get('python', True):
    env.Default(pyext)
