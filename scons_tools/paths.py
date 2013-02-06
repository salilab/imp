"""Various utilities for handling paths in a uniform manner"""

from SCons.Script import File, Alias, Dir, Glob
import SCons.Node
import data
import os
import glob
import os.path

def get_source_root(env):
    """Root of the source directory"""
    repository=env.get("repository", None)
    if repository:
        cd= os.path.join(Dir("#/").abspath, repository)
    else:
        cd = Dir("#/").abspath
    return os.path.join(cd)



def _get_scons_path(env, path):
    repository=env.get("repository", None)
    if repository and repository != '.':
        rpath = os.path.join(Dir("#/").abspath, repository, Dir(".").path)
        ret= os.path.relpath(path, rpath)
        #print "scons_path", path, rpath, ret
        return ret
    else:
        return path

def _get_source_path(env, name):
    repository=env.get("repository", None)
    if repository:
        cd= os.path.join(Dir("#/").abspath, repository, Dir(".").path)
    else:
        cd= Dir(".").path
        if not os.path.isabs(cd):
            cd = os.path.join(Dir("#/").abspath, cd)
    return os.path.join(cd,str(name))

def get_matching_source(env, patterns):
    """Return File nodes for all source that match pattern"""
    if type(patterns) != type([]):
        raise RuntimeError("second argument to get_matching_source must be a list")
    #print "searching for", patterns, "in", Dir(".").path
    ret=[]
    for p in patterns:
        sp= _get_source_path(env, p)
        #print sp
        for m in glob.glob(sp):
            #print m
            ret.append(_get_source_path(env, m))
    ret.sort(cmp=lambda x,y: cmp(str(x), str(y)))
    #print "found", [x.abspath for x in ret]
    return ret


def get_matching_build(env, patterns, ondisk=False):
    """Return File nodes for all source that match pattern"""
    if type(patterns) != type([]):
        raise RuntimeError("second argument to get_matching_source must be a list")
    #print "searching for", patterns, "in", Dir(".").path
    ret=[]
    for p in patterns:
        sp= "#/build/"+p
        for m in Glob(sp, ondisk=ondisk):
            ret.append(m)
    return ret


def get_matching_build_files(env, patterns, ondisk=False):
    """Return File nodes for all source files that match pattern"""
    ret= [x for x in get_matching_build(env, patterns, ondisk=ondisk)
          if not isinstance(x, SCons.Node.FS.Dir)]
    return ret


def _get_file_name(env, fl):
    fln= str(fl)
    sp= fln.rfind("/")
    if sp==-1:
        return fln
    else:
        return fln[sp+1:]

def get_output_path(env, fl, output_dir=None):
    """Return the path in the output directory for the file"""
    if not output_dir:
        output_dir= Dir(".")
    return os.path.join(output_dir.abspath, _get_file_name(env, fl))


def get_output_path_suffix(env, fl, suffix, output_dir=None):
    """Return the path in the output directory for the file"""
    if not output_dir:
        output_dir= Dir(".")
    oname=_get_file_name(env, fl)
    return os.path.join(output_dir.abspath, oname+suffix)


def get_input_path(env, fl):
    """Return the path in the input directory for the file"""
    return _get_source_path(env, fl)

def get_build_source_file(env, name, modulename=None):
    """Return a file node for a generated source file"""
    if modulename:
        return File("#/build/src/"+modulename+"/"+name)
    else:
        return File("#/build/src/"+name)

def get_build_swig_source_file(env, name, modulename=None):
    """Return a file node for a generated source file"""
    if modulename:
        return File("#/build/src/"+modulename+"_swig/"+name)
    else:
        return File("#/build/src/"+name+"_swig")


def get_build_bin_dir(env, modulename=None):
    """Return a file node for a generated bin file"""
    if modulename:
        return Dir("#/build/module_bin/"+modulename+"/")
    else:
        return Dir("#/build/bin/")

def get_build_benchmark_dir(env, modulename):
    """Return a file node for a generated bin file"""
    return Dir("#/build/benchmark/"+modulename+"/")

def get_build_test_dir(env, modulename=None):
    """Return a file node for a generated bin file"""
    if modulename:
        return Dir("#/build/test/"+modulename+"/")
    else:
        return Dir("#/build/test/")

def get_build_lib_name(env, name):
    """Return a file node for a generate lib file"""
    return File("#/build/lib/"+name)

def get_sconscripts(env, ordered=[], final=[]):
    alls= get_matching_source(env, ["*/SConscript"])
    all_names={}
    for a in alls:
        all_names[str(a).split("/")[0]]=a
    ret=[]
    for o in ordered:
        if o in all_names.keys():
            ret.append(all_names[o])
            del all_names[o]
    fret=[]
    for o in final:
        if o in all_names.keys():
            fret.append(all_names[o])
            del all_names[o]
    for p in all_names.keys():
        ret.append(all_names[p])
    return ret+fret
