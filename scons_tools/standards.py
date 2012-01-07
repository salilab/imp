from SCons.Script import Glob, Dir, File, Builder, Action
import SCons.Node.FS
import os
import os.path




def _action_check_standards(target, source, env):
    env.Execute(source[0].get_contents())

def _print_check_standards(target, source, env):
    print "Checking standards..."

CheckStandards = Builder(action=Action(_action_check_standards,
                                       _print_check_standards))


def setup_standards(env):
    env.Append(BUILDERS = {'CheckStandards':CheckStandards})
    # Check code for coding standards:
    patterns=["#/modules/\*/src/\*.cpp",
              "#/modules/\*/src/\*/\*.cpp",
              "#/modules/\*/include/\*.h",
              "#/modules/\*/include/\*/\*.h",
              "#/modules/\*/examples/\*.py",
              "#/modules/\*/examples/\*/\*.py",
              "#/modules/\*/test/\*.py",
              "#/modules/\*/bin/\*.cpp",
              "#/applications/\*/\*.cpp",
              "#/biological_systems/\*/\*/\*.cpp"]
    cmd="#/tools/check-standards.py"
    if env.get('repository', None):
        patterns= [x.replace("#", env['repository']) for x in patterns]
        cmd=cmd.replace("#", env['repository'])
    else:
        patterns= [x.replace("#", ".") for x in patterns]
        cmd= File(cmd).abspath
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
                                   source=env.Value(" ".join(["cd", Dir("#").abspath,";", cmd]+patterns)))
    env.Alias('standards', standards)
    env.AlwaysBuild(standards)
