from SCons.Script import Builder, File, Action, Glob, Return, Dir, Move, Copy, Scanner, Chmod
import os.path
import utility
import data
import os

def _link(source, target, verbose=False):
    # TODO make it copy the file on windows
    tpath= os.path.abspath(target)
    spath= os.path.abspath(source)
    #print tpath, spath
    if not os.path.exists(spath):
        if verbose:
            print "no source", spath
        return
    if os.path.islink(tpath):
        return
    if os.path.isdir(tpath):
        shutil.rmtree(tpath)
    if os.path.exists(tpath):
        if verbose:
            print "unlinking"
        os.unlink(tpath)
    if verbose:
        print "linking", spath, tpath
    os.symlink(spath, tpath)

def add(env, target):
    prec=""
    if env.get('precommand', None):
        prec= env['precommand']
    if env.get('MODELLER_MODPY', None):
        prec=prec+" "+env['MODELLER_MODPY']
    if env.get('datapath', None):
        externdata= env.get('datapath', "")
    else:
        externdata=""
    cmd=["../scons_tools/build_tools/setup_imppy.py"]
    cmd.append("\"--python_path=%s\""%utility.get_python_path(env))
    cmd.append("\"--ld_path=%s\""%utility.get_ld_path(env))
    cmd.append("\"--precommand=%s\""%prec)
    cmd.append("\"--path=%s\""%env.Value(env['ENV']['PATH']))
    if env.get("wine", False):
        cmd.append("--wine_hack=yes")
    cmd.append("\"--external_data=%s\""%externdata)
    env.Execute("cd build; " + " ".join(cmd))
    _link(File("#/build/imppy.sh").abspath,
          File("#/tools/imppy.sh").abspath, verbose=True)
