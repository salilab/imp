"""Utility function to install a directory hierarchy of include files, with
   a top-level 'include all' header."""

import os
import UserList
from SCons.Script import Action, Entry, File, Dir
import module
import data
import environment
import SCons
import SCons.Errors



# should merge with one in scons_tools.module.py

def _linkFunc(dest, source, env):
    """Link a file from source to dest"""
    if os.path.isdir(source):
        raise SCons.Errors.UserError("Source must be a file, not a directory")
    else:
        # Cannot use os.path.exists here (does not work for broken symlinks);
        # cannot use os.path.lexists either (not present in older Pythons)
        try:
            os.unlink(dest)
        except OSError:
            pass
        if os.path.isabs(source) or os.path.isabs(dest):
            os.symlink(os.path.abspath(source), dest)
        else:
            # If both paths are relative to top-level directory, figure out how
            # to get from dest to the top level, then make a relative symlink:
            updirs = len(os.path.normpath(dest).split(os.path.sep)) - 1
            uppath = os.path.sep.join([os.path.pardir] * updirs)
            os.symlink(os.path.join(uppath, source), dest)
    return 0

def _link_install(env, target, source, **keys):
    # print target, source
    if hasattr(os, 'symlink'):
        return env.Install(target, [source], INSTALL=_linkFunc, **keys)
    else:
        return env.Install(target, [source], **keys)

def _link_install_as(env, target, source, **keys):
    #print target, source
    if hasattr(os, 'symlink'):
        return env.InstallAs(target, [source], INSTALL=_linkFunc, **keys)
    else:
        return env.InstallAs(target, [source], **keys)


def get_install_directory(env, varname, *subdirs):
    """Get a directory to install files in. The top directory is env[varname],
       prefixed with env['destdir']. The full directory is constructed by
       adding any other function arguments as subdirectories."""
    destdir = env.subst(env['destdir'])
    installdir = env.subst(env[varname])
    if destdir != '' and not os.path.isabs(installdir):
        print >> sys.stderr, "Install directory %s (%s) must be an absolute path,\n" \
              "since you have set destdir." % (varname, installdir)
        env.Exit(1)
    installdir = destdir + installdir
    # Use SCons, not os.path.abspath, since we may not be in the top directory
    if not os.path.isabs(installdir):
        installdir = env.Dir('#/' + installdir).abspath
    return os.path.join(installdir, *subdirs)

def _get_prefix(varname):
    if varname=="datadir":
        prefix="#/build/data"
    elif varname=="bindir":
        prefix="#/build/bin"
    elif varname=="docdir":
        prefix="#/build/doc"
    elif varname=="libdir":
        prefix="#/build/lib"
    elif varname=="includedir":
        prefix="#/build/include"
    elif varname=="pythondir":
        prefix="#/build/lib"
    elif varname=="pyextdir":
        prefix="#/build/lib"
    elif varname=="swigdir":
        prefix="#/build/swig"
    elif varname=="srcdir":
        prefix="#/build/src"
    else:
        print "Unknwn", varname
    return prefix

def _get_path(env, target, prefix):
    ret= "/".join([prefix]+str(target).split("/")[1:])
    cd= environment.get_current_name(env)
    if cd=='kernel':
        cd ="."
    cf=environment.get_current_name(env)
    #if cf=='kernel':
    #    cf='IMP'
    return ret.replace('currentdir', cd).replace('currentfile', cf)

def get_build_path(env, target):
    varname= str(target).split("/")[0]
    prefix= _get_prefix(varname)
    return _get_path(env, target, prefix)


def install(env, target, source, **keys):
    """Like the standard Install builder, but using symlinks if available."""
    #print str(target), str(source)
    varname= str(target).split("/")[0]
    prefix=_get_prefix(varname)
    installpath= _get_path(env, target, prefix)
    #print Dir(installpath).get_abspath(), Dir("/".join(str(source).split("/")[:-1])).get_abspath()
    ret=[]
    if Dir(installpath).get_abspath() !=\
           Dir("/".join(str(source).split("/")[:-1])).get_abspath():
        inst= _link_install(env, installpath, source, **keys)
        data.get(env).add_to_alias(environment.get_current_name(env), inst)
        ret.append(inst[0])
    else:
        data.get(env).add_to_alias(environment.get_current_name(env), source)
    if varname=='swigdir' or varname=='srcdir':
        return
    installpath= _get_path(env, target, env.subst(env[varname]))
    inst= env.Install(installpath, source, **keys)
    ret.append(inst[0])
    data.get(env).add_to_alias(environment.get_current_name(env)+"-install", inst)
    return ret

def install_as(env, target, source, **keys):
    #print "in", target, source
    varname= str(target).split("/")[0]
    prefix=_get_prefix(varname)
    installpath= _get_path(env, target, prefix)
    #print File(installpath).get_abspath(), File(str(source)).get_abspath()
    ret=[]
    if File(installpath).get_abspath() !=\
           File(str(source)).get_abspath():
        inst= _link_install_as(env, installpath, source, **keys)
        ret.append(inst[0])
        data.get(env).add_to_alias(environment.get_current_name(env), inst)
    else:
        data.get(env).add_to_alias(environment.get_current_name(env), source)
    if varname=='swigdir' or varname=='srcdir':
        return
    installpath= _get_path(env, target, env.subst(env[varname]))
    inst= env.InstallAs(File(installpath), source, **keys)
    ret.append(inst[0])
    data.get(env).add_to_alias(environment.get_current_name(env)+"-install", inst)
    data.get(env).add_to_alias("install", inst)
    return ret
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

def _install_hierarchy_internal(env, dir, sources):
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
        insttargets.append(install(env, dest, f))
    return insttargets




def install_hierarchy(env, dir, sources):
    targets = \
       _install_hierarchy_internal(env, dir, sources)
    return targets
