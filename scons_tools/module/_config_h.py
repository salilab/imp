import scons_tools.paths
from SCons.Script import File, Dir, Action, Value

def quote(s):
    return "\""+s+"\""

def build(env, macros, data):
    sources=[]
    cleaned_macros=[]
    for m in macros:
        if type(m)==list:
            cleaned_macros.append(m[0]+"="+str(m[1]))
        else:
            cleaned_macros.append(m)
    sources.append(File("#/scons_tools/build_tools/setup_config_h.py"))
    sources.append(env.Value("--source="+quote(scons_tools.paths.get_source_root(env))))
    sources.append(env.Value("--name="+data.name))
    sources.append(env.Value("--defines="+quote(":".join(cleaned_macros))))
    sources.append(env.Value("--found_optional_modules="+quote(":".join(data.modules))))
    sources.append(env.Value("--unfound_optional_modules="+quote(":".join(data.unfound_modules))))
    sources.append(env.Value("--found_optional_dependencies="+quote(":".join(data.direct_dependencies))))
    sources.append(env.Value("--unfound_optional_dependencies="+quote(":".join(data.unfound_dependencies))))
    cmd=" ".join(["cd", Dir("#/build").abspath, ";"]+[str(x) for x in sources])
    # scons doesn't get the dependencies right no matter what I do, so just write it
    # every time
    env.Execute(cmd)
