from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re


def _action_config_py(target, source, env):
    config= source[0].get_contents().split("#")
    def _export_to_config(name, ename, env):
        if env.get(ename, None):
            if type(env[ename]) == type([]):
                val=":".join(env[v])
                print val
                config.append(name+"='"+val+"'")
            else:
                config.append(name+"='"+str(env[ename])+"'")
    for v in ['build', 'repository', 'cxxflags', 'linkflags',
              'precommand', 'includepath', 'modeller',
              'prefix', 'path', 'libpath']:
        _export_to_config(v, v, env)

    for vp in [('cxxcompiler', 'CXX'),
               ('python', 'IMP_PROVIDE_PYTHON'),
               ('rpath', 'IMP_USE_RPATH'),
               ('platformflags', 'IMP_USE_PLATFORM_FLAGS'),
               ('static', 'IMP_BUILD_STATIC')]:
        _export_to_config(vp[0], vp[1], env)
    #print "opening h at " +target[0].abspath + " for module %(module)s"%vars
    h = file(target[0].abspath, 'w')
    #print "Generating "+str(h)
    s= config
    for l in s:
        print >> h, l

def _print_config_py(target, source, env):
    print "Generating config.py"

ConfigPY = Builder(action=Action(_action_config_py,
                                 _print_config_py))
