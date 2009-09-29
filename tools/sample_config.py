import os
import os.path

## for convenience
home=os.environ['HOME']

## Where to install IMP
prefix=+"/fs"

## Add include paths as CXXFLAGS so scons doesn't treat such headers
## as dependencies.
cxxflags=" ".join(['-I'+os.path.join(prefix, 'include'), '-I/opt/local/include'])

## Add my local environment to that used by scons and tools/imppy.sh
pythonpath=os.environ['PYTHONPATH']
ldlibpath=os.environ['LD_LIBRARY_PATH']
libpath=':'.join(["/opt/local/lib",os.path.join(prefix, 'lib')])
path=os.environ['PATH']

## Build any local module added to the modules directory
localmodules=True

## The build more can be one of 'debug', 'release' or 'fast'.
## It is recommended that you use a 'debug' build for testing and development
## and a 'fast' build for running the final code.
build='debug'

## It is recommented that you build in a different directory than the sources
## to do this, have your config.py and a link to the SConscruct files and scons_tools
## directories in the build directory and set 'repository' to a path to the source
## directory.
repository="../svn/"

# Run all tests with valgrind
#precommand="valgrind --gen-suppressions=yes --db-attach=yes --undef-value-errors=no --suppressions=../svn/tools/valgrind-python.supp"

## Get modeller's location from an environment variable
modeller=os.environ['MODINSTALLSVN']
