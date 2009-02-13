"""Utility function to install a directory hierarchy of include files, with
   a top-level 'include all' header."""

import os
import UserList
from SCons.Script import Action, Entry

def _build_header(target, source, env):
    fname = target[0].path
    module = env['IMP_MODULE']
    description = env['IMP_MODULE_DESCRIPTION']
    if module == 'IMP':
        guard = "IMP_H"
    else:
        guard = "IMP_%s_H" % module.upper()
    fh = file(fname, 'w')
    print >> fh, "/**\n *  \\file %s   \\brief %s\n *" \
             % (os.path.basename(fname), description)
    print >> fh, " *  Copyright 2007-8 Sali Lab. All rights reserved."
    print >> fh, " *\n */\n"
    print >> fh, "#ifndef %s\n#define %s\n" % (guard, guard)
    prefix = len(os.path.commonprefix([f.path for f in source]))
    for f in source:
        src = f.path[prefix:]
        if not src.startswith('internal'):
            print >> fh, '#include "%s/%s"' % (module, src)
    print >> fh, "\n#endif  /* %s */" % guard

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

def InstallHierarchy(env, dir, module, description, sources):
    """Given a set of header files, install them all under `dir`. They are
       placed in the `module` subdirectory (common prefix is stripped from the
       filenames) and a file `module`.h is generated in `dir` which includes
       all headers, and with the given comment description. A list of all
       installed headers is returned, suitable for an 'install' alias. The
       headers are also installed in the build directory, but these targets
       are not returned."""
    targets = []
    builddir_targets = []
    sources = _make_nodes(sources)
    prefix = len(os.path.commonprefix([f.path for f in sources]))
    if module != 'IMP':
        incdir = os.path.join('#/build/include', 'IMP')
    else:
        incdir = '#/build/include'
    for f in sources:
        src = f.path[prefix:]
        dest = os.path.join(dir, module, os.path.dirname(src))
        targets.append(env.Install(dest, f))
        # Also place the header in the build directory:
        dest = os.path.join(incdir, module, os.path.dirname(src))
        builddir_targets.append(env.LinkInstall(dest, f))

    gen_heads = []
    for d in (dir, incdir):
        t = env.Command(os.path.join(d, module + '.h'), builddir_targets,
                        Action(_build_header,
                               'Auto-generating header ${TARGET}'),
                        IMP_MODULE=module, IMP_MODULE_DESCRIPTION=description)
        gen_heads.append(t)
    targets.append(gen_heads[0])
    return targets


def InstallPythonHierarchy(env, dir, module, sources):
    """Given a set of Python files, install them all under `dir`. They are
       placed in the `module` subdirectory (common prefix is stripped from the
       filenames). A list of all installed files is returned, suitable for an
       'install' alias, plus another list of the files in the build
       directory."""
    insttargets = []
    libtargets = []
    sources = _make_nodes(sources)
    prefix = len(os.path.commonprefix([f.path for f in sources]))
    if module != 'IMP':
        libdir = os.path.join('#/build/lib', 'IMP')
    else:
        libdir = '#/build/lib'
    for f in sources:
        src = f.path[prefix:]
        dest = os.path.join(dir, module, os.path.dirname(src))
        insttargets.append(env.Install(dest, f))
        # Also place the file in the build directory:
        dest = os.path.join(libdir, module, os.path.dirname(src))
        libtargets.append(env.LinkInstall(dest, f))
    return insttargets, libtargets
