"""Utility functions used by all IMP modules"""

import os.path
import re
import sys
from SCons.Script import *
import hierarchy
import symlinks
import bug_fixes
import standards
import dependency.compilation
import module
import dependency
import platform

import SCons

def _propagate_variables(env):
    """enforce dependencies between variables"""
    env['IMP_BUILD_STATIC']= env['static']
    env['IMP_BUILD_DYNAMIC']= env['dynamic']
    env['IMP_PROVIDE_PYTHON']= env['python']
    env['IMP_USE_PLATFORM_FLAGS']= env['platformflags']
    env['IMP_USE_RPATH']= env['rpath']
    if env['pythonsosuffix'] != 'default':
        env['IMP_PYTHON_SO']=env['pythonsosuffix']
    elif env['IMP_PROVIDE_PYTHON'] and not env['IMP_USE_PLATFORM_FLAGS']:
        print >> sys.stderr, "Do not know suffix for python lib, please provide pythonsosuffix"
        env.Exit(1)
    if env['wine']:
        env['IMP_BUILD_STATIC']=False
        env['IMP_PYTHON_SO']='.pyd'

    if env['PLATFORM']!= 'posix' and env['PLATFORM'] != 'darwin' and env['IMP_USE_RPATH']:
        env['IMP_USE_RPATH']=False
        print >> sys.stderr, "WARNING rpath not supported on platform "+ env['PLATFORM']

    if not env['IMP_BUILD_DYNAMIC']:
        env['IMP_PROVIDE_PYTHON']=False
    if not env['IMP_BUILD_DYNAMIC'] and not env['IMP_BUILD_STATIC']:
        print >> sys.stderr, "One of dynamic or static libraries must be supported."
        env.Exit(1)
    if env.get('pythonpath', None):
        env['PYTHONPATH'] = env['pythonpath']
    else:
        env['PYTHONPATH']=''
    if env.get('cxxcompiler', None):
        env['CXX']=env['cxxcompiler']
    if env.get('ar', None):
        env['AR']= env['ar']
    if env.get('ranlib', None):
        env['RANLIB']= env['ranlib']
    if env.get("swigprogram", None):
        env['SWIG']= env["swigprogram"]
    if env.get('cxxflags', None):
        env.Append(CXXFLAGS = env['cxxflags'].split())
    else:
        env.Append(CXXFLAGS=[])

    if env.get('pythoncxxflags', None):
        env.Append(IMP_PYTHON_CXXFLAGS = env['pythoncxxflags'].split())
    elif env.get('cxxflags', None):
        env.Append(IMP_PYTHON_CXXFLAGS = env['cxxflags'].split())
    else:
        env.Append(IMP_PYTHON_CXXFLAGS=[])

    if env.get('linkflags', None):
        env.Append(IMP_LINKFLAGS=env['linkflags'].split())
    else:
        env.Append(IMP_LINKFLAGS=[])
    if env.get('pythonlinkflags', None):
        env.Append(IMP_PYTHON_LINKFLAGS=env['pythonlinkflags'].split())
    else:
        env.Append(IMP_PYTHON_LINKFLAGS=[])

    if env.get('shliblinkflags', None):
        env.Append(IMP_SHLIB_LINKFLAGS=env['shliblinkflags'].split())
    else:
        env.Append(IMP_SHLIB_LINKFLAGS=[])

    if env.get('arliblinkflags', None):
        env.Append(IMP_ARLIB_LINKFLAGS=env['arliblinkflags'].split())
    else:
        env.Append(IMP_ARLIB_LINKFLAGS=[])


    if env.get('binlinkflags', None):
        env.Append(IMP_BIN_LINKFLAGS=env['binlinkflags'].split())
    else:
        env.Append(IMP_BIN_LINKFLAGS=[])

    if env.get('includepath') is not None:
        env['includepath'] = [os.path.abspath(x) for x in \
                          env['includepath'].split(os.path.pathsep)]
        env.Prepend(CPPPATH=env['includepath'])
    else:
        env.Append(CPPPATH=[])

    if env.get('libpath') is not None:
        env.Prepend(LIBPATH=[os.path.abspath(x) for x in \
                             env['libpath'].split(os.path.pathsep)])
    else:
        env.Append(LIBPATH=[])
    if env.get('libs') is not None:
        libs= env['libs'].split(":")
        env.Append(LIBS=libs)
    else:
        env.Append(LIBS=[])

    if env.get('ldlibpath') is not None:
        env['ENV']['LD_LIBRARY_PATH'] = env['ldlibpath']


def add_common_variables(vars, package):
    """Add common variables to an SCons Variables object."""
    libenum=["yes", "no", "auto"]
    libdir = '${prefix}/lib'
    if hasattr(os, 'uname') and sys.platform == 'linux2' \
       and os.uname()[-1] == 'x86_64':
        # Install in /usr/lib64 rather than /usr/lib on x86_64 Linux boxes
        libdir += '64'
    vars.Add(PathVariable('cxxcompiler', 'The C++ compiler to use (eg g++).', None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('ar', "The command to make a static library.", None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('ranlib', "The command to make an index of a static library.", None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('swigprogram', 'The path to the swig command.', None,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('prefix', 'Top-level installation directory', '/usr',
                          PathVariable.PathAccept))
    vars.Add(PathVariable('datadir', 'Data file installation directory',
                          '${prefix}/share/%s'%package,
                          PathVariable.PathAccept))
    vars.Add(PathVariable('bindir', 'Executable installation directory',
                          '${prefix}/bin', PathVariable.PathAccept))
    vars.Add(PathVariable('libdir', 'Shared library installation directory',
                          libdir, PathVariable.PathAccept))
    vars.Add(PathVariable('includedir', 'Include file installation directory',
                          '${prefix}/include', PathVariable.PathAccept))
    vars.Add(PathVariable('pythondir', 'Python module installation directory',
                          libdir + '/python%d.%d/site-packages' \
                          % sys.version_info[0:2], PathVariable.PathAccept))
    vars.Add(PathVariable('pyextdir',
                          'Python extension module installation directory',
                          '${pythondir}', PathVariable.PathAccept))
    vars.Add(PathVariable('docdir', 'Documentation installation directory',
                          '${prefix}/share/doc/%s' % package,
                          PathVariable.PathAccept))
    # Note that destdir should not affect any compiled-in paths; see
    # http://www.gnu.org/prep/standards/html_node/DESTDIR.html
    vars.Add(PathVariable('destdir',
                          'String to prepend to every installed filename',
                          '', PathVariable.PathAccept))
    vars.Add(PackageVariable('pythoninclude',
                             'Directory holding Python include files ' + \
                             '(if unspecified, distutils location is used)',
                             'no'))
    vars.Add(PackageVariable('modeller',
                             "Set to 'yes' to use the MODELLER package, "
                             "or 'no' to not use it (and to disable modules "
                             "such as IMP.modeller that use it). 'yes' will "
                             "only find MODELLER if it is in the system Python "
                             "path (e.g. Windows, Mac .dmg or Linux .rpm "
                             "binary installs); if you installed the .tar.gz "
                             "version, or have a copy of the source code, set "
                             "this variable to the top-level MODELLER "
                             "directory.", 'no'))
    vars.Add(BoolVariable('wine',
                          'Build using MS Windows tools via Wine emulation',
                          False))
    vars.Add(EnumVariable('build',
                          "Set to 'release' for a normal build," \
                          +" 'debug' to disable optimization," \
                          +" or 'fast' to disable most runtime checks," \
                          +" but keep debugging information",
                          "release", ['release', 'debug', 'fast']))
    vars.Add(EnumVariable('endian',
                          "The endianness of the platform. \"auto\" will determine it automatically.",
                          "auto", ['auto', 'big', 'little']))
    vars.Add(BoolVariable('linksysv',
                          'Link with old-style SysV, not GNU hash, for ' + \
                          'binary compatibility', False))
    vars.Add('includepath', 'Include search path ' + \
             '(e.g. "/usr/local/include:/opt/local/include")', None)
    vars.Add('libpath', 'Library search path ' + \
             '(e.g. "/usr/local/lib:/opt/local/lib")', None)
    vars.Add('libs', 'Extra libs to add to link commands ' + \
             '(e.g. "efence:pthread")', None)
    vars.Add(BoolVariable('rpath',
                          'Add any entries from libpath to library search ' + \
                          'path (rpath) on Linux systems', True))
    vars.Add('ldlibpath', 'Add to the runtime library search path ' +\
             '(LD_LIBRARY_PATH on linux-like systems) for various ' + \
             'build tools and the test cases', None)
    vars.Add('cxxflags', 'C++ flags for all C++ builds (e.g. "-fno-rounding:-DFOOBAR"). See pythoncxxflags.',
             None)
    vars.Add('pythoncxxflags', 'C++ flags for building the python libraries (e.g. "-fno-rounding:-DFOOBAR")',
             None)

    vars.Add('linkflags', 'Link flags for all linking (e.g. "-lefence"). See pythonlinkflags, arliblinkflags, shliblinkflags.', None)
    vars.Add('pythonlinkflags', 'Link flags for linking python libraries (e.g. "-lefence")', "")
    vars.Add('arliblinkflags', 'Link flags for linking static libraries (e.g. "-lefence")', "")
    vars.Add('shliblinkflags', 'Link flags for linking shared libraries (e.g. "-lefence")', "")
    vars.Add('binlinkflags', 'Link flags for linking executables (e.g. "-lefence")', "")
    vars.Add('path', 'Extra executable path ' + \
             '(e.g. "/opt/local/bin/") to search for build tools', None)
    vars.Add('precommand',
             'A command to be run to wrap program invocations.' + \
             'For example, "valgrind --db-attach=yes --suppressions=valgrind-python.supp"', "")
    vars.Add('pythonpath', 'Extra python path ' + \
             '(e.g. "/opt/local/lib/python-2.5/") to use for tests', None)
    vars.Add('boostversion', 'The version of boost. If this is not none, the passed version is used and checks are not done. The version should look like "104200" for Boost "1.42".', None)
    vars.Add(BoolVariable('platformflags',
                          'If true, add any compiler and linker arguments that might be needed/desired. If false, only used passed flags (eg only the values in "cxxflags", "linkflags" etc).',
                          True))
    vars.Add(BoolVariable('deprecated',
                          'Build deprecated classes and functions', True))
    vars.Add('pythonsosuffix', 'The suffix for the python libraries.', 'default')
    vars.Add(BoolVariable('dot',
                          'Use dot from graphviz to lay out graphs in the documentation if available. This produces prettier graphs, but is slow.',
                          True))
    vars.Add(BoolVariable('svn',
                          'True if this build is from an svn version of IMP. If so, SVN version info is added to the provided version number.',
                          True))
    vars.Add(BoolVariable('python', 'Whether to build the python libraries ', True))
    vars.Add(BoolVariable('localmodules', 'Whether to build local modules that are not part of the IMP distribution', False))
    vars.Add(BoolVariable('linktest', 'Test for header defined functions which are not inline', True))
    vars.Add(PathVariable('repository', 'Where to find the source code to build. This is only needed if building in a different directory than the source.', None, PathVariable.PathAccept)) #PathIsDir
    vars.Add(BoolVariable('static', 'Whether to build static libraries.', False))
    vars.Add(BoolVariable('dynamic', 'Whether to build dynamic libraries (needed for python support).', True))
    vars.Add(BoolVariable('precompiledheader', 'Whether to use a precompiled header for swig libraries ', False))
    vars.Add('disabledmodules', 'A colon-separated list of modules to disable.', '')
    #vars.Add(BoolVariable('noexternaldependencies', 'Do not check files in the provided includepath and libpath for changes.', False))


def update(env, variables):
    variables.Update(env)
    _propagate_variables(env)
