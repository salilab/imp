from SCons.Script import *
import copy
import data

# scons messes up the paths when the build dir is separate from the source dir
# and tends to pass paths saying everything is in the build dir
# these methods search for things in the repository if they are not found
# in the build dir.
def fix_node(env, node):
    if not os.path.exists(node.path):
        if env.get('repository', None) != None:
            nnode= File(env['repository']+"/"+node.path)
            if os.path.exists(nnode.path):
                return nnode
    return node

# scons clone doesn't actually clone, try to do it properly
def clone_env(env):
    ret= env.Clone(CXXFLAGS= copy.deepcopy(env.get("CXXFLAGS", [])),
                     CPPFLAGS= copy.deepcopy(env.get("CPPFLAGS", [])),
                     CPPDEFINES= copy.deepcopy(env.get("CPPDEFINES", [])),
                     LDMODULEFLAGS= copy.deepcopy(env.get("LDMODULEFLAGS", [])),
                     LDMODULEPREFIX= copy.deepcopy(env.get("LDMODULEPREFIX", "")),
                     SHLIBPREFIX= copy.deepcopy(env.get("SHLIBPREFIX", "")),
                     SCANNERS= [x for x in env.get("SCANNERS", [])],
                     LDMODULESUFFIX= copy.deepcopy(env.get("LDMODULESUFFIX", "")),
                     SHLIBSUFFIX= copy.deepcopy(env.get("SHLIBSUFFIX", "")))
    # just in case scons fixes the bug
    data.add(ret, data.get(env))
    return ret
