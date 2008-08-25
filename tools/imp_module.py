"""Tools and Builders for IMP modules.

   The IMPModuleExports Builder generates a header file used to mark classes
   and functions for export, e.g.
   env.IMPModuleExports('foo_exports.h', env.Value('foo'))
   generates exports macros for the 'foo' module.

   The IMPModuleVersionInfo Builder generates a header and source file to
   return version information, e.g.
   env.IMPModuleVersionInfo(('foo_version_info.cpp', 'foo_version_info.h'),
                            (env.Value('foo'), env.Value('Me'),
                             env.Value('1.0')))
   generates version information for the 'foo' module.
"""

import os.path

def action_exports(target, source, env):
    """Single source should be a Value, which is the name of the module."""
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
    """Two outputs (.cpp and .h) generated from three input Values"""
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

    print >> cpp, '#include "%s_version_info.h"\n' % module

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


def generate(env):
    """Add builders and construction variables for the IMP module tool."""
    from SCons.Builder import Builder
    exports = Builder(action=action_exports)
    version_info = Builder(action=action_version_info)
    env.Append(BUILDERS = {'IMPModuleExports': exports,
                           'IMPModuleVersionInfo': version_info})

def exists(env):
    """Right now no external programs are needed"""
    return True
