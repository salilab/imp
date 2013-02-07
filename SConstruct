# include IMP build utility functions:

import scons_tools
import scons_tools.dependency
import scons_tools.dependency.boost
import scons_tools.dependency.swig
import scons_tools.dependency.endian
import scons_tools.dependency.gcc
import scons_tools.dependency.pkgconfig
import scons_tools.application
import scons_tools.test
import scons_tools.config_py
import scons_tools.build_summary
import scons_tools.variables
import scons_tools.install
import scons_tools.imppy
import scons_tools.html_coverage
import scons_tools.python_coverage
import scons_tools.paths
import scons_tools.build_tools.tools
import sys
import os
import atexit
from SCons import Script

# We need scons 0.98 or later
EnsureSConsVersion(0, 98)

# Set up build environment:
vars = Variables(files=[File('#/config.py').abspath])
scons_tools.variables.add_common_variables(vars, "imp")
env = scons_tools.environment.get_base_environment(variables=vars,
                              tools=["default", "swig", "dot", "doxygen", "cpp",
                                     "protoc"],
                              toolpath=["scons_tools/tools"])

env['IMP_PASS']="CONFIGURE"

try:
    os.mkdir("build")
except:
    pass
env.Execute("cd %s; %s" %(Dir("#/build").abspath, File("#/scons_tools/build_tools/setup.py").abspath)
            +" \"--source="+scons_tools.paths.get_input_path(env, ".")+"\""\
            +" \"--disabled="+env.get("disabledmodules", "")+"\""\
            +" \"--datapath="+env.get("datapath", "")+"\"")
env.Execute("cd %s; %s --module=base --alias=compatibility"%(Dir("#/build").abspath,
                         File("scons_tools/build_tools/setup_module_alias.py").abspath
            +" --source="+scons_tools.paths.get_input_path(env, ".")))
env.Execute("cd %s; %s --module=kernel --alias="%(Dir("#/build").abspath,
                         File("scons_tools/build_tools/setup_module_alias.py").abspath
            +" --source="+scons_tools.paths.get_input_path(env, ".")))
try:
    env['IMP_VERSION']=open(scons_tools.utility.get_source_path(env, "VERSION"), "r").read().rstrip('\r\n')
except:
    env['IMP_VERSION']="SVN"
env['IMP_VARIABLES']=vars
env['IMP_SCONS_EXTRA_VARIABLES']=[]
env['IMP_CONFIGURATION']=[]

Export('env')
if env.get('repository', None) is not None:
    Repository(env['repository'])


if not env.GetOption('help'):
    if not env.GetOption('clean'):
        if not env.get('COMPILER_OK', None):
            Exit("""
No working compiler found. Please make sure that g++ or another
compiler recognized by scons can be found in your path and that all
the passed compiler options (cxxflags, linkflags) are correct.
""")

    scons_tools.dependency.pkgconfig.configure_check(env)
    scons_tools.dependency.add_external_library(env, "Boost", None,
                                                "boost/version.hpp", versionheader="boost/version.hpp",
                                                versioncpp=["BOOST_VERSION"])
    scons_tools.dependency.boost.find_lib_version(env)
    if not env.GetOption('clean'):
        if env.get('html_coverage', 'no') != 'no':
            scons_tools.html_coverage.register(env)
        boost_info= scons_tools.build_tools.tools.get_dependency_info("Boost",
                                                                      env.get("datapath", ""),
                                                                      Dir("#/build").abspath)
        try:
            boost_version= int(boost_info["version"][0])
        except (IndexError, AttributeError, ValueError):
            boost_version = None
        if not boost_info["ok"] or (boost_version is None or boost_version < 104000):
            scons_tools.utility.report_error(env, """
Boost version 1.40 or later is required to build IMP, but it could not
be found on your system.

In particular, if you have Boost installed in a non-standard location, please use the 'includepath' option to add this location to the search path.  For example, a Mac using Boost installed with MacPorts will have the Boost headers in /opt/local/include, so edit (or create) config.py and add the line

includepath='/opt/local/include'

You can see the produced config.log for more information as to why boost failed to be found.
""")



if not env.GetOption('help'):
    # various flags depending on compiler versions and things
    scons_tools.dependency.swig.configure_check(env)
    scons_tools.dependency.gcc.configure_check_visibility(env)
    scons_tools.dependency.gcc.configure_check_hash(env)
    # Make these objects available to SConscript files:

scripts=["applications/SConscript", "tools/SConscript", "doc/SConscript"]

module_order=scons_tools.build_tools.tools.get_sorted_order(Dir("#/build").abspath)
print "modules are", module_order
for m in module_order:
    SConscript("modules/%s/SConscript"%m)

env.Execute("cd %s; %s"%(Dir("#/build").abspath,
                         File("scons_tools/build_tools/setup_swig_wrappers.py").abspath
            +" \"--source="+scons_tools.paths.get_input_path(env, ".")+"\""
            +" \"--datapath="+env.get("datapath", "")+"\""))

env.Execute("cd %s; %s"%(Dir("#/build").abspath,
                         File("scons_tools/build_tools/setup_applications.py").abspath
            +" \"--source="+scons_tools.paths.get_input_path(env, ".")+"\""
            +" \"--datapath="+env.get("datapath", "")+"\""))

# placed here so that the result is universally visible since it
# is special cased for benchmarks
env['IMP_PASS']="BUILD"
for m in module_order:
    SConscript("modules/%s/SConscript"%m)
for s in scripts:
    SConscript(s)

env['IMP_PASS']="RUN"
for m in module_order:
    SConscript("modules/%s/SConscript"%m)
for s in scripts:
    SConscript(s)


if not env.GetOption('help'):
    # This must be after the other SConscipt calls so that it knows about all the generated files
    imppy= scons_tools.imppy.add(env, "tools/imppy.sh")
    sitecust = scons_tools.python_coverage.setup(env)
    env.Depends(imppy, sitecust)
    env.Alias(env.Alias('all'), imppy)

    env.Alias(env.Alias('test'), [env.Alias('examples-test')])

    Clean('all', ['build'])
    Clean('all', Glob('scons_tools/*.pyc')\
              + Glob('tools/*.pyc'))

    env.Default(env.Alias('all'))
    install= scons_tools.install.get_install_builder(env)
    env.Depends(install, [env.Alias("all")])
    env.Alias("install", install)
    #env.Depends(install, env.Alias('all'))

    unknown = vars.UnknownVariables()

    # Older versions of scons have a bug with command line arguments
    # that are added late, so remove those we know about from this list
    for var in env['IMP_SCONS_EXTRA_VARIABLES']:
        unknown.pop(var, None)
    if unknown:
        print >> sys.stderr, "\n\nUnknown variables: ", " ".join(unknown.keys())
        print >> sys.stderr, "Use 'scons -h' to get a list of the accepted variables."
        Exit(1)
    scons_tools.build_summary.setup(env)
    config_py=scons_tools.config_py.add(env)
    senv= scons_tools.environment.get_named_environment(env, "scons", [], [])
    scons_tools.install.install(senv, "datadir/scons", "SConstruct")
    scons_tools.install.install_hierarchy(senv, "datadir/scons/scons_tools",
                                          "scons_tools",
                                          Glob("scons_tools/*.py")+
                                          Glob("scons_tools/*/*.py"))

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
