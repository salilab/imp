from SCons.Script import Glob, Dir, File, Builder, Action
import SCons.Node.FS
import os
import os.path




def _action_check_standards(target, source, env):
    env.Execute(" ".join([x.path for x in source]))

def _print_check_standards(target, source, env):
    pass

CheckStandards = Builder(action=Action(_action_check_standards,
                                       _print_check_standards))


def setup_standards(env):
    env.Append(BUILDERS = {'CheckStandards':CheckStandards})
    # Check code for coding standards:
    patterns=["#/modules/*/src/*.cpp",
              "#/modules/*/src/*/*.cpp",
              "#/modules/*/include/*.h",
              "#/modules/*/include/*/*.h",
              "#/modules/*/examples/*.py",
              "#/modules/*/examples/*/*.py",
              "#/modules/*/test/*.py"
              "#/kernel/src/*.cpp",
              "#/kernel/src/*/*.cpp",
              "#/kernel/include/*.h",
              "#/kernel/include/*/*.h",
              "#/kernel/examples/*.py",
              "#/kernel/examples/*/*.py",
              "#/kernel/test/*.py",
              "#/modules/*/bin/*.cpp",
              "#/kernel/bin/*.cpp"]
    outfiles=[]
    for g in patterns:
        files= Glob(g)
        for f in files:
            if str(f).endswith("config.h") or str(f).endswith("version_info.h"):
                pass
            else:
                outfiles.append(f)
    #for o in outfiles:
    #    print o.abspath
    #print "chunking"
    #for i in range(0, len(outfiles), 100):
    #    chunk=outfiles[i:(i+100)]
    #    for o in chunk:
    #        print o.abspath
    #standards = env.CheckStandards(target='standards'+str(i)+'.passed',
    #                               source=["#/tools/check-standards.py"]+chunk)
    standards = env.CheckStandards(target='standards.passed',
                                   source=["#/tools/check-standards.py"]+outfiles)
    env.Alias('standards', standards)
    env.AlwaysBuild(env.Alias('standards'))
