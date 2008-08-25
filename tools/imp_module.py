"""Tools and Builders for IMP modules.

   The IMPModuleExports Builder generates a header file used to mark classes
   and functions for export, e.g.
   env.IMPModuleExports('foo_exports.h', env.Value('foo'))
   generates exports macros for the 'foo' module.
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

def generate(env):
    """Add builders and construction variables for the IMP module tool."""
    from SCons.Builder import Builder
    exports = Builder(action=action_exports)
    env.Append(BUILDERS = {'IMPModuleExports': exports})

def exists(env):
    """Right now no external programs are needed"""
    return True
