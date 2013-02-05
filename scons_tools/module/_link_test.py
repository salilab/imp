
import scons_tools.module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re


def _action_link_test(target, source, env):
    """The IMPModuleLinkTesto Builder generates a source file. By linking in two
    of these, any functions which are defined in headers but not declared inline are detected"""
    module= scons_tools.module._get_module_name(env)
    fname = target[0].abspath
    cpp = file(fname, 'w')
    print >> cpp, """/*
 *
 *  This file is auto-generated, do not edit.
 *
 *  Copyright 2007-2013 IMP Inventors. All rights reserved.
 *
 */

"""

    print >> cpp, """
#ifndef IMP_NO_DEBUG
    #include <IMP/%s.h>
#endif""" % module


def _print_link_test(target, source, env):
    print "Generating link test"

LinkTest = Builder(action=Action(_action_link_test,
                                _print_link_test))
