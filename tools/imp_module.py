"""Tools and Builders for IMP modules. See `IMPModule` for more information."""

import os.path
from SCons.Script import Builder, File

def action_exports(target, source, env):
    """The IMPModuleExports Builder generates a header file used to mark
       classes and functions for export, e.g.
       env.IMPModuleExports('foo_exports.h', env.Value('foo'))
       generates exports macros for the 'foo' module."""
    name = source[0].get_contents()
    out = file(target[0].abspath, 'w')
    print >> out, """/*
 * \\file %(filename)s
 * \\brief Provide macros to mark functions and classes as exported
 *        from a DLL/.so
 *
 * When building the module, IMP%(NAME)s_EXPORTS should be defined, and when
 * using the module externally, it should not be. Classes and functions
 * defined in the module's headers should then be marked with
 * IMP%(NAME)sEXPORT if they are intended to be part of the API, or with
 * IMP%(NAME)sLOCAL if they are not (the latter is the default).
 *
 * The Windows build environment requires applications to mark exports in
 * this way; we use the same markings to set the visibility of ELF symbols
 * if we have compiler support.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */

#ifndef __IMP_%(NAME)s_EXPORTS_H
#define __IMP_%(NAME)s_EXPORTS_H

#ifdef _MSC_VER
#ifdef IMP%(NAME)s_EXPORTS
#define IMP%(NAME)sEXPORT __declspec(dllexport)
#else
#define IMP%(NAME)sEXPORT __declspec(dllimport)
#endif
#define IMP%(NAME)sLOCAL
#else
#ifdef GCC_VISIBILITY
#define IMP%(NAME)sEXPORT __attribute__ ((visibility("default")))
#define IMP%(NAME)sLOCAL __attribute__ ((visibility("hidden")))
#else
#define IMP%(NAME)sEXPORT
#define IMP%(NAME)sLOCAL
#endif
#endif

#endif  /* __IMP_%(NAME)s_EXPORTS_H */""" \
    % {'name':name, 'NAME':name.upper(),
       'filename':os.path.basename(target[0].abspath)}

def action_version_info(target, source, env):
    """The IMPModuleVersionInfo Builder generates a source file and header to
       return version information, e.g.
       env.IMPModuleVersionInfo(('src/foo_version_info.cpp',
                                 'include/foo_version_info.h'),
                                (env.Value('foo'), env.Value('Me'),
                                 env.Value('1.0')))
       generates version information for the 'foo' module."""
    module = source[0].get_contents()
    author = source[1].get_contents()
    version = source[2].get_contents()
    cpp = file(target[0].abspath, 'w')
    h = file(target[1].abspath, 'w')

    for (f, ext) in ((cpp, 'cpp'), (h, 'h')):
        print >> f, """/**
 *  \\file %(module)s_version_info.%(ext)s    \\brief %(module)s module version information.
 *
 *  Copyright 2007-8 Sali Lab. All rights reserved.
 *
 */
""" % {'module':module, 'ext':ext}

    print >> h, """#ifndef __IMP_%(MODULE)s_VERSION_INFO_H
#define __IMP_%(MODULE)s_VERSION_INFO_H

#include <IMP/VersionInfo.h>
#include \"%(module)s_exports.h\"
""" % {'module':module, 'MODULE':module.upper()}

    print >> cpp, '#include "IMP/%s/%s_version_info.h"\n' % (module, module)

    for f in (h, cpp):
        for namespace in ('IMP', module, 'internal'):
            print >> f, "namespace %s\n{\n" % namespace

    print >> h, """//! Version and authorship of the %(module)s module.
extern IMP%(MODULE)sEXPORT VersionInfo %(module)s_version_info;""" \
        % {'module':module, 'MODULE':module.upper()}

    print >> cpp, 'VersionInfo %s_version_info("%s", "%s");' \
              % (module, author, version)

    for f in (h, cpp):
        for namespace in ('internal', module, 'IMP'):
            print >> f, "\n} // namespace %s" % namespace

    print >> h, "\n#endif  /* __IMP_%s_VERSION_INFO_H */" % module.upper()

def IMPSharedLibrary(env, files, install=True):
    """Build, and optionally also install, an IMP module's C++
       shared library. This is only available from within an environment
       created by `IMPSharedLibraryEnvironment`."""
    module = env['IMP_MODULE']
    lib = env.SharedLibrary('#/build/libs/imp_%s' % module,
                            list(files) + [env['VER_CPP']])
    if install:
        libinst = env.Install(env['libdir'], lib)
        env.Alias('%s-install' % module, [libinst])
        return lib, libinst
    else:
        return lib

def IMPSharedLibraryEnvironment(env):
    """Create a customized environment suitable for building IMP module C++
       shared libraries. Use the resulting object's `IMPSharedLibrary` pseudo
       builder to actually build the shared library."""
    from tools import get_sharedlib_environment
    module = env['IMP_MODULE']
    env = get_sharedlib_environment(env, 'IMP%s_EXPORTS' % module.upper(),
                                    cplusplus=True)
    env.AddMethod(IMPSharedLibrary)
    return env

def IMPHeaders(env, files):
    """Install the given header files, plus any auto-generated files for this
       IMP module."""
    from tools.hierarchy import InstallHierarchy
    module = env['IMP_MODULE']
    includedir = os.path.join(env['includedir'], 'IMP')
    inst = InstallHierarchy(env, includedir, env['IMP_MODULE'],
                            env['IMP_MODULE_DESCRIPTION'],
                            list(files) + [env['EXP_H'], env['VER_H']])
    env.Alias('%s-install' % module, inst)
    return inst

def IMPPythonExtension(env, swig_interface):
    """Build and install an IMP module's Python extension and the associated
       wrapper file from a SWIG interface file. This is only available from
       within an environment created by `IMPPythonExtensionEnvironment`."""
    module = env['IMP_MODULE']
    pyext = env.LoadableModule('#/build/libs/_IMP_%s' % module, swig_interface)
    # .py file should also be generated:
    gen_pymod = File('IMP_%s.py' % module)
    env.Depends(gen_pymod, pyext)
    # Place in libs directory:
    pymod = env.InstallAs('#/build/libs/IMP/%s/__init__.py' % module, gen_pymod)

    # Install the Python extension and module:
    libinst = env.Install(env['pyextdir'], pyext)
    pyinst = env.Install(os.path.join(env['pythondir'], 'IMP', module), pymod)
    env.Alias('%s-install' % module, [libinst, pyinst])
    return pyext, pymod

def IMPPythonExtensionEnvironment(env):
    """Create a customized environment suitable for building IMP module Python
       extensions. Use the resulting object's `IMPPythonExtension` pseudo
       builder to actually build the extension."""
    from tools import get_pyext_environment
    module = env['IMP_MODULE']
    env = get_pyext_environment(env, 'IMP' + module.upper(), cplusplus=True)
    env.Append(LIBS=['imp_%s' % module])
    env.Append(SWIGPATH='#/build/include')
    env.Append(SWIGFLAGS='-python -c++ -naturalvar')
    env.AddMethod(IMPPythonExtension)
    return env

def _action_unit_test(target, source, env):
    (dir, script) = os.path.split(source[0].path)
    app = "cd %s && %s %s %s -v > /dev/null" \
          % (dir, " ".join([x.abspath for x in source[1:]]),
             env['PYTHON'], script)
    if env.Execute(app) == 0:
        file(str(target[0]), 'w').write('PASSED\n')
    else:
        print "unit tests FAILED"
        return 1

def _emit_unit_test(target, source, env):
    source.append('#/bin/imppy.sh')
    if env['TEST_ENVSCRIPT']:
        source.append(env['TEST_ENVSCRIPT'])
    return target, source

def IMPModuleTest(env, target, source, **keys):
    """Pseudo-builder to run tests for an IMP module. The single target is
       generally a simple output file, e.g. 'test.passed', while the single
       source is a Python script to run (usually run-all-tests.py).
       If the TEST_ENVSCRIPT construction variable is set, it is a shell
       script to run to set up the environment to run the test script.
       A convenience alias for the tests is added, and they are always run."""
    test = env._IMPModuleTest(target, source, **keys)
    env.Alias("%s-test" % env['IMP_MODULE'], test)
    env.AlwaysBuild(target)
    return test

def IMPModule(env, module, author, version, description):
    """Set up an IMP module. The module's SConscript gets its own
       customized environment ('env') in which the following pseudo-builders
       are available: IMPSharedLibraryEnvironment,
       IMPPythonExtensionEnvironment, IMPHeaders and IMPModuleTest."""
    env = env.Clone()
    exports = Builder(action=action_exports)
    version_info = Builder(action=action_version_info)
    env.Append(BUILDERS = {'IMPModuleExports': exports,
                           'IMPModuleVersionInfo': version_info})

    env['IMP_MODULE'] = module
    env['IMP_MODULE_DESCRIPTION'] = description
    env.Append(CPPPATH=['#/build/include', env['BOOST_CPPPATH']])
    env.Append(LIBPATH=['#/build/libs'], LIBS=['imp'])
    env.Help("""
Type: 'scons %(module)s' to build and test the %(module)s extension module;
      'scons %(module)s-test' to just test it;
      'scons %(module)s-install' to install it.
""" % {'module':module})
    # Generate version information
    env['VER_CPP'], env['VER_H'] = \
        env.IMPModuleVersionInfo(('%s/src/%s_version_info.cpp' % (module,
                                                                  module),
                                  '%s/include/%s_version_info.h' % (module,
                                                                    module)),
                                 (env.Value(module), env.Value(author),
                                  env.Value(version)))
    # Generate exports header
    env['EXP_H'] = env.IMPModuleExports('%s/include/%s_exports.h' % (module,
                                                                     module),
                                        env.Value(module))
    env.AddMethod(IMPSharedLibraryEnvironment)
    env.AddMethod(IMPPythonExtensionEnvironment)
    env.AddMethod(IMPHeaders)
    env.AddMethod(IMPModuleTest)
    env.Append(BUILDERS={'_IMPModuleTest': Builder(action=_action_unit_test,
                                                   emitter=_emit_unit_test)})
    env['TEST_ENVSCRIPT'] = None
    return env.SConscript('%s/SConscript' % module, exports='env')

def generate(env):
    """Add builders and construction variables for the IMP module tool."""
    env.AddMethod(IMPModule)

def exists(env):
    """Right now no external programs are needed"""
    return True
