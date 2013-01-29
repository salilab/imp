from SCons.Script import Builder, File, Action, Glob, Return, Dir, Move, Copy, Scanner, Chmod
import os.path
import utility
import data

def add(env, target):
    prec=""
    if env.get('precommand', None):
        prec= env['precommand']
    if env.get('MODELLER_MODPY', None):
        prec=prec+" "+env['MODELLER_MODPY']
    if env.get('datapath', None):
        externmodules=[x for x in data.get(env).modules.keys()\
                       if data.get(env).modules[x].external]
        externdata= env.get('datapath', "")
    else:
        externmodules=[]
        externdata=""
    cmd=["scons_tools/build_tools/setup_imppy.py"]
    cmd.append("--base_dir=\"build\"")
    cmd.append("--python_path=\"%s\""%utility.get_python_path(env))
    cmd.append("--ld_path=\"%s\""%utility.get_ld_path(env))
    cmd.append("--precommand=\"%s\""%prec)
    cmd.append("--path=\"%s\""%env.Value(env['ENV']['PATH']))
    cmd.append("--modules=\"%s\""%":".join([x for x in data.get(env).modules.keys()\
                                              if not data.get(env).modules[x].external]))
    cmd.append("--external_modules=\"%s\""%":".join(externmodules))
    if env.get("wine", False):
        cmd.append("--wine_hack=yes")
    cmd.append("--external_data=\"%s\""%externdata)
    env.Execute(" ".join(cmd))
