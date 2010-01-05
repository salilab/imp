from SCons.Script import *


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
