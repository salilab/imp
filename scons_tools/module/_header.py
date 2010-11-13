"""Utility function to install a directory hierarchy of include files, with
   a top-level 'include all' header."""

import os
import UserList
from SCons.Script import Action, Entry, File
import scons_tools.module
import scons_tools.install

# should merge with one in scons_tools.module.py




def _build_header(target, source, env):
    vars= scons_tools.module._get_module_variables(env);
    fh = file(target[0].abspath, 'w')
    print >> fh, "/**\n *  \\file %(module_include_path)s.h   \\brief Include all the headers\n *" \
             % vars
    print >> fh, " *  Copyright 2007-2010 IMP Inventors. All rights reserved."
    print >> fh, " *\n */\n"
    print >> fh, "#ifndef %(PREPROC)s_H\n#define %(PREPROC)s_H\n" % vars
    # prefix does not work when there are a mix of generated and source files
    #= len(os.path.commonprefix([f.path for f in source]))
    for f in source[0].get_contents().split(" "):
        #print src
        if not f.startswith('internal') and len(f) > 0:
            vars['header']= f
            print >> fh, '#include <%(module_include_path)s/%(header)s>' %vars
    print >> fh, "#include <%(module_include_path)s/%(module)s_config.h>"%vars
    print >> fh, "\n#endif  /* %(PREPROC)s_H */" % vars

def build_header(env, dir, sources):
    source_list=[str(x) for x in sources]
    source_list.sort()
    t = env.Command(dir, env.Value(" ".join(source_list)),
                    Action(_build_header,
                               'Auto-generating header ${TARGET}'))
    return t
