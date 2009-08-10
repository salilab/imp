
import imp_module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re


def _action_version_info_h(target, source, env):
    """The IMPModuleVersionInfo Builder generates a source file and header to
       return version information, e.g.
       env.IMPModuleVersionInfo(('src/internal/version_info.cpp',
                                 'include/internal/version_info.h'),
                                (env.Value('foo'), env.Value('Me'),
                                 env.Value('1.0')))
       generates version information for the 'foo' module."""
    vars= imp_module.make_vars(env)

    h = file(target[0].abspath, 'w')

    print >> h, """/**
 *  \\file %(module_include_path)s/internal/version_info.h
 *  \\brief %(module)s module version information.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
""" % vars

    print >> h, """#ifndef %(PREPROC)s_INTERNAL_VERSION_INFO_H
#define %(PREPROC)s_INTERNAL_VERSION_INFO_H

#include "../config.h"

""" % vars

    print >> h, """namespace IMP {
class VersionInfo;
}
""" %vars

    print >> h, "%(PREPROC)s_BEGIN_INTERNAL_NAMESPACE\n" % vars

    print >> h, """//! Version and authorship of the %(module)s module.
extern %(PREPROC)sEXPORT VersionInfo version_info;""" \
        % vars

    print >> h, "\n%(PREPROC)s_END_INTERNAL_NAMESPACE" % vars

    print >> h, "\n#endif  /* %(PREPROC)s_INTERNAL_VERSION_INFO_H */" % vars



def _print_version_info_h(target, source, env):
    print "Generating version_info.h"

VersionInfoH = Builder(action=Action(_action_version_info_h,
                                  _print_version_info_h))




def _action_version_info_cpp(target, source, env):
    """The IMPModuleVersionInfo Builder generates a source file and header to
       return version information, e.g.
       env.IMPModuleVersionInfo(('src/internal/version_info.cpp',
                                 'include/internal/version_info.h'),
                                (env.Value('foo'), env.Value('Me'),
                                 env.Value('1.0')))
       generates version information for the 'foo' module."""
    vars= imp_module.make_vars(env)

    cpp = file(target[0].abspath, 'w')

    print >> cpp, """/**
 *  \\file %(module_include_path)s/internal/version_info.cpp
 *  \\brief %(module)s module version information.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
""" % vars

    print >> cpp, """#include <%(module_include_path)s/internal/version_info.h>
#include <IMP/VersionInfo.h>
"""  % vars


    print >> cpp, "%(PREPROC)s_BEGIN_INTERNAL_NAMESPACE\n" % vars


    print >> cpp, 'VersionInfo version_info("%(author)s", "%(version)s");' \
              %vars

    print >> cpp, "\n%(PREPROC)s_END_INTERNAL_NAMESPACE" % vars



def _print_version_info_cpp(target, source, env):
    print "Generating version_info.cpp"

VersionInfoCPP = Builder(action=Action(_action_version_info_cpp,
                                  _print_version_info_cpp))
