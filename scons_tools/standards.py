from SCons.Script import Glob, Dir, File, Builder, Action
import SCons.Node.FS
import os
import os.path




def _action_check_standards(target, source, env):
    env.Execute(" ".join([x.path for x in source]))

def _print_check_standards(target, source, env):
    print "Checking standards"

CheckStandards = Builder(action=Action(_action_check_standards,
                                       _print_check_standards))


def get_standards_files(env):
    files= [File("#/tools/check-standards.py")]\
        + Glob("#/modules/*/src/*.cpp")\
        + Glob("#/modules/*/src/*/*.cpp")\
        + Glob("#/modules/*/include/*.h")\
        + Glob("#/modules/*/include/*/*.h")\
        + Glob("#/modules/*/examples/*.py")\
        + Glob("#/modules/*/examples/*/*.py")\
        + Glob("#/kernel/src/*.cpp")\
        + Glob("#/kernel/src/*/*.cpp")\
        + Glob("#/kernel/include/*.h")\
        + Glob("#/kernel/include/*/*.h")\
        + Glob("#/kernel/examples/*.py")\
        + Glob("#/kernel/examples/*/*.py")\
        + Glob("#/modules/*/bin/*.cpp")\
        + Glob("#/kernel/bin/*.cpp")
    outfiles=[]
    for f in files:
        if str(f).endswith("config.h") or str(f).endswith("version_info.h"):
            pass
        else:
            outfiles.append(f)
    return outfiles
