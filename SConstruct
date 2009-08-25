# Include IMP build utility functions:
import scons_tools
import scons_tools.boost
import scons_tools.cgal
import scons_tools.swig
import scons_tools.standards
import scons_tools.endian
import scons_tools.modeller_test

# We need scons 0.98 or later
EnsureSConsVersion(0, 98)

# Set up build environment:
vars = Variables('config.py')
scons_tools.add_common_variables(vars, "imp")
env = scons_tools.MyEnvironment(variables=vars,
                                tools=["default", "docbook", "swig"],
                                toolpath=["scons_tools"])
unknown = vars.UnknownVariables()
if unknown:
    print "Unknown variables: ", unknown.keys()
    print "Use 'scons -h' to get a list of the accepted variables."
    Exit(1)


if env.get('repository', None) is not None:
    Repository(env['repository'])

env.AddMethod(scons_tools.imp_module.IMPModuleBuild)
env['IMP_MODULES_ALL']=[]
scons_tools.boost.configure_check(env, '1.33')
scons_tools.cgal.configure_check(env)
scons_tools.swig.configure_check(env)
scons_tools.modeller_test.configure_check(env)
scons_tools.endian.configure_check(env)

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
SConscript('build/SConscript')
SConscript('modules/SConscript')
SConscript('applications/SConscript')
# This must be after the other SConscipt calls so that it knows about all the generated files
SConscript('doc/SConscript')
SConscript('tools/SConscript')
SConscript('bin/SConscript')

env.Alias(env.Alias('test'), [env.Alias('examples-test')])

Clean('all', ['build/tmp/',
                'build/include',
                'build/lib',
                'build/data'])
Clean('all', Glob('scons_tools/*.pyc')\
          + Glob('tools/*.pyc'))

env.Default('tools/imppy.sh')
env.Default('bin')
