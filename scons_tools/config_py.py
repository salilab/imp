from SCons.Script import Glob, Dir, File, Builder, Action, Exit
import os
import sys
import re

def _flatten(strorlist, delim):
    if type(strorlist)== type(""):
        return strorlist
    elif type(strorlist) == type(True):
        return str(strorlist)
    else:
        return delim.join([_flatten(x, delim) for x in list(strorlist)])

def _action_config_py(target, source, env):
    config= source[0].get_contents().split("#")
    def _export_to_config(name, ename, env, opt=False, delim=":"):
        if opt and not env.get(ename, None):
            return
        #config.append("# "+str(type(env[ename])))
        #config.append( "# "+repr(env[ename]))
        config.append(name+"='"+_flatten(env.subst(env[ename]), delim)+"'")
    for v in ['build', 'repository',
              'precommand', 'includepath', 'modeller',
              'prefix', 'libpath', 'local',
              'pythonpath', 'python_include', 'ldlibpath']:
        _export_to_config(v, v, env, True)
    config.append('platformflags=False')
    for vp in [('cxxcompiler', 'CXX', " "),
               ('cxxflags', 'CXXFLAGS', " "),
               ('pythoncxxflags', 'IMP_PYTHON_CXXFLAGS', " "),
               ('shliblinkflags', 'IMP_SHLIB_LINKFLAGS', " "),
               ('arliblinkflags', 'IMP_ARLIB_LINKFLAGS', " "),
               ('binlinkflags', 'IMP_BIN_LINKFLAGS', " "),
               ('pythonlinkflags', 'IMP_PYTHON_LINKFLAGS', " "),
               ('python', 'IMP_PROVIDE_PYTHON', " "),
               ('rpath', 'IMP_USE_RPATH', " "),
               ('static', 'IMP_BUILD_STATIC', " "),
               ('pythonsosuffix', 'IMP_PYTHON_SO', " ")]:
        _export_to_config(vp[0], vp[1], env, delim=vp[2])
    config.append('path="'+_flatten(os.environ['PATH'], ":")+'"')
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
