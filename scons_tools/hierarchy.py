"""Utility function to install a directory hierarchy of include files, with
   a top-level 'include all' header."""

import os
import UserList
from SCons.Script import Action, Entry, File
import module

# should merge with one in scons_tools.module.py


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
    #print [x.path for x in sources]
    for f in sources:
        full = f.path
        if full.rfind("include") != -1:
            src = full[full.rfind("include")+8:]
        elif full.rfind("src") != -1:
            src= full[full.rfind("src")+4:]
        # restrainer needs this before data
        elif full.find("examples") != -1:
            src= full[full.rfind("examples")+9:]
        elif full.rfind("data") != -1:
            src= full[full.rfind("data")+5:]
        else:
            raise ValueError(full)
        #print src
        dest = os.path.join(dir, os.path.dirname(src))
        if can_link:
            insttargets.append(env.LinkInstall(dest, f))
        else:
            insttargets.append(env.Install(dest, f))
    return insttargets

def InstallHierarchy(env, dir, sources, can_link=False):
    targets = \
       _install_hierarchy_internal(env, dir, sources, can_link)
    return targets
