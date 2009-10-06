
import imp_module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re


def _action_link_test(target, source, env):
    """The IMPModuleLinkTesto Builder generates a source file. By linking in two
    of these, any functions which are defined in headers but not declared inline are detected"""
    vars= imp_module.make_vars(env)
    for fname in [target[0].abspath, target[1].abspath]:
        cpp = file(fname, 'w')
        print >> cpp, """/*
 *
 *  This file is auto-generated, do not edit.
 *
 *  Copyright 2007-9 Sali Lab. All rights reserved.
 *
 */
""" % vars

        print >> cpp, """
#ifndef IMP_NDEBUG
#include "%(module_include_path)s.h"
#endif""" % vars


def _print_link_test(target, source, env):
    print "Generating link test"

LinkTest = Builder(action=Action(_action_link_test,
                                _print_link_test))
