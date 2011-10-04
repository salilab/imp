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

def _get_prefix(env, varname):
    if varname=="datadir":
        prefix=env["builddir"]+"/data"
    elif varname=="bindir":
        prefix=env["builddir"]+"/bin"
    elif varname=="docdir":
        prefix=env["builddir"]+"/doc"
    elif varname=="libdir":
        prefix=env["builddir"]+"/lib"
    elif varname=="includedir":
        prefix=env["builddir"]+"/include"
    elif varname=="pythondir":
        prefix=env["builddir"]+"/lib"
    elif varname=="pyextdir":
        prefix=env["builddir"]+"/lib"
    elif varname=="swigdir":
        prefix=env["builddir"]+"/swig"
    elif varname=="srcdir":
        prefix=env["builddir"]+"/src"
    elif varname=="moduledir":
        prefix="#/modules/"+environment.get_current_name(env)
    else:
        prefix=varname
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
    prefix= _get_prefix(env, varname)
    return _get_path(env, target, prefix)


def install(env, target, source, **keys):
    """Like the standard Install builder, but using symlinks if available."""
    #print str(target), str(source)
    varname= str(target).split("/")[0]
    prefix=_get_prefix(env, varname)
    installpath= _get_path(env, target, prefix)
    ret=[]
    internal_installpath=installpath
    if Dir(installpath).get_abspath() !=\
           Dir("/".join(str(source).split("/")[:-1])).get_abspath():
        inst= _link_install(env, installpath, source, **keys)
        data.get(env).add_to_alias(environment.get_current_name(env), inst)
        ret.append(inst[0])
    else:
        inst=[]
        data.get(env).add_to_alias(environment.get_current_name(env), source)
    # ick, need a better mechanism for installed vs not
    if varname=='swigdir' or varname=='srcdir'\
            or varname=='moduledir' or varname=="#"\
           or varname=="biological_systems"\
           or varname=="." or varname=="modules":
        return ret
    installpath= _get_path(env, target, env.subst(env[varname]))
    #print Dir(installpath).get_abspath(), internal_installpath
    # handle hacks involving setting the datapath to "./data"
    if Dir(installpath) == Dir(internal_installpath):
        return ret
    destdir = env.subst(env['destdir'])
    if destdir != '' and not os.path.isabs(destdir):
        destdir = Dir('#/' + destdir).abspath
    inst= env.Install(destdir + installpath, source, **keys)
    ret.append(inst[0])
    data.get(env).add_to_alias(environment.get_current_name(env)\
                                   +"-install", inst)
    data.get(env).add_to_alias("install", inst)
    return ret

def install_as(env, target, source, **keys):
    #print "in", target, source
    varname= str(target).split("/")[0]
    prefix=_get_prefix(env, varname)
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
    destdir = env.subst(env['destdir'])
    if destdir != '' and not os.path.isabs(destdir):
        destdir = Dir('#/' + destdir).abspath
    inst= env.InstallAs(File(destdir + installpath), source, **keys)
    ret.append(inst[0])
    data.get(env).add_to_alias(environment.get_current_name(env)+"-install", inst)
    data.get(env).add_to_alias("install", inst)
    return ret


def install_hierarchy(env, dir, root_dir, sources):
    build=[]
    inst=[]
    for s in sources:
        full = File(s).path
        name = full[full.rfind(root_dir+"/")+len(root_dir)+1:]
        f= name.rfind("/")
        if f==-1:
            cdir=""
        else:
            cdir=name[:f]
        l= install(env, Dir(dir+"/"+cdir), File(s))
        build.append(l[0])
        if len(l)>1:
            inst.append(l[1])
    return (build, inst)
