"""Utility function to install a directory hierarchy of include files, with
   a top-level 'include all' header."""

import os
import UserList
from SCons.Script import Action, Entry

# should merge with one in imp_module.py

def make_vars(env):
    """Make a map which can be used for all string substitutions"""
    module = env['IMP_MODULE']
    module_include_path = env['IMP_MODULE_INCLUDE_PATH']
    module_src_path = env['IMP_MODULE_SRC_PATH']
    module_preproc = env['IMP_MODULE_PREPROC']
    module_namespace = env['IMP_MODULE_NAMESPACE']
    author = env['IMP_MODULE_AUTHOR']#source[0].get_contents()
    version = env['IMP_MODULE_VERSION']#source[1].get_contents()
    vars={'module_include_path':module_include_path,
          'module_src_path':module_src_path, 'module':module,
          'PREPROC':module_preproc, 'author':author, 'version':version,
          'namespace':module_namespace}
    return vars



def _build_header(target, source, env):
    vars= make_vars(env);
    fname = target[0].path
    vars['fname']=fname
    fh = file(fname, 'w')
    print >> fh, "/**\n *  \\file %(fname)s   \\brief Include all the headers\n *" \
             % vars
    print >> fh, " *  Copyright 2007-9 Sali Lab. All rights reserved."
    print >> fh, " *\n */\n"
    print >> fh, "#ifndef %(PREPROC)s_H\n#define %(PREPROC)s_H\n" % vars
    prefix = len(os.path.commonprefix([f.path for f in source]))
    for f in source:
        src = f.path[prefix:]
        if not src.startswith('internal'):
            vars['header']= src
            print >> fh, '#include <%(module_include_path)s/%(header)s>' %vars
    print >> fh, "\n#endif  /* %(PREPROC)s_H */" % vars

def _make_nodes(files):
    nodes = []
    for f in files:
        if isinstance(f, str):
            nodes.append(Entry(f))
        elif isinstance(f, (list, tuple, UserList.UserList)):
            nodes.extend(_make_nodes(f))
        else:
            nodes.append(f)
    return nodes

def _install_hierarchy_internal(env, dir, sources, can_link):
    insttargets = []
    sources = _make_nodes(sources)
    prefix = len(os.path.commonprefix([f.path for f in sources]))
    for f in sources:
        src = f.path[prefix:]
        dest = os.path.join(dir, os.path.dirname(src))
        if can_link:
            insttargets.append(env.LinkInstall(dest, f))
        else:
            insttargets.append(env.Install(dest, f))
    return insttargets

def InstallHierarchy(env, dir, sources, can_link=False):
    """Given a set of header files, install them all under `dir`. A file
       dir.h is created , and with the given comment description. A list of all
       installed headers is returned, suitable for an 'install' alias."""
    targets = \
       _install_hierarchy_internal(env, dir, sources, can_link)

    t = env.Command(dir + '.h', sources,
                    Action(_build_header,
                               'Auto-generating header ${TARGET}'))

    targets.append(t)
    return targets

def InstallPythonHierarchy(env, dir, sources, can_link=False):
    """Given a set of Python files, install them all under `dir`. They are
       placed in the `module` subdirectory (common prefix is stripped from the
       filenames). A list of all installed files is returned, suitable for an
       'install' alias, plus another list of the files in the build
       directory."""
    return _install_hierarchy_internal(env, dir, sources, can_link)

def InstallDataHierarchy(env, dir, sources, can_link):
    """Given a set of data files, install them all under `dir`. They are
       placed in the `module` subdirectory (common prefix is stripped from the
       filenames). A list of all installed files is returned, suitable for an
       'install' alias, plus another list of the files in the build
       directory."""
    return _install_hierarchy_internal(env, dir, sources, can_link)
