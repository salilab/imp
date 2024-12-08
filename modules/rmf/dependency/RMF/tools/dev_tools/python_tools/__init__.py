import glob
import os
import ast
import os.path
import shutil
import sys
import difflib
import subprocess
import signal

# cmake paths are always /-separated; on platforms where the path is not /
# (e.g. Windows) convert a path to or from a form cmake will like
if os.sep == '/':
    def to_cmake_path(p):
        return p

    def from_cmake_path(p):
        return p
else:
    def to_cmake_path(p):
        return p.replace(os.sep, '/')

    def from_cmake_path(p):
        return p.replace('/', os.sep)


def get_existing_links(path):
    """Get any symlinks in the given directory"""
    return [f for f in glob.glob(os.path.join(path, "*")) if os.path.islink(f)]


def mkdir(path, clean=True):
    if os.path.isdir(path):
        if clean:
            # remove any old links
            for f in glob.glob(os.path.join(path, "*")):
                if os.path.islink(f):
                    os.unlink(f)
            # Remove old lists of Python tests
            for f in glob.glob(os.path.join(path, "*.pytests")):
                os.unlink(f)
        else:
            # clean broken links
            for f in glob.glob(os.path.join(path, "*")):
                if os.path.islink(f) and not os.path.exists(os.readlink(f)):
                    os.unlink(f)
        return
    if os.path.isfile(path):
        os.unlink(path)
    os.makedirs(path)


def quote(st):
    out = st.replace(
        "\\f",
        ""). replace(
        "\\b",
        "").replace(
        "\\",
        "\\\\").replace(
        "\"",
        "\\\"")
    return out


def get_glob(patterns):
    ret = []
    for p in patterns:
        c = sorted(glob.glob(p))
        ret += c
    return ret


def rewrite(filename, contents, show_update=True, verbose=False):
    try:
        old = open(filename, "r").read()
        if old == contents:
            return
        elif verbose:
            print("    Different", filename)
            for ln in difflib.unified_diff(old.split("\n"),
                                           contents.split("\n")):
                stl = str(ln)
                if stl[0] in ('-', '+') and stl[1] not in ('-', '+'):
                    print("    " + stl)
    except Exception:
        pass
        # print "Missing", filename
    if show_update:
        print("Updating " + filename)
    dirpath = os.path.split(filename)[0]
    if dirpath != "":
        mkdir(dirpath, False)
    open(filename, "w").write(contents)


def rmdir(path):
    try:
        shutil.rmtree(path)
    except Exception:
        pass


def link(source, target, verbose=False):
    tpath = os.path.abspath(target)
    spath = os.path.abspath(source)
    # print tpath, spath
    if not os.path.exists(spath):
        if verbose:
            print("no source", spath)
        return
    if os.path.islink(tpath):
        if os.readlink(tpath) == spath:
            return
        else:
            os.unlink(tpath)
    elif os.path.isdir(tpath):
        shutil.rmtree(tpath)
    elif os.path.exists(tpath):
        os.unlink(tpath)
    if verbose:
        print("linking", spath, tpath)
    if hasattr(os, 'symlink'):
        os.symlink(spath, tpath)
    # Copy instead of link on platforms that don't support symlinks (Windows)
    elif os.path.isdir(spath):
        def excludes(src, names):
            return ['.svn', 'data']
        shutil.copytree(spath, tpath, ignore=excludes)
    else:
        shutil.copy(spath, tpath)


def link_dir(source_dir, target_dir, match=["*"], exclude=[],
             clean=True, verbose=False):
    if not isinstance(match, list):
        raise TypeError("match must be a list")
    exclude = exclude + ["SConscript", "CMakeLists.txt", ".svn"]
    # print "linking", source_dir, target_dir
    mkdir(target_dir, clean=False)
    if clean:
        existing_links = get_existing_links(target_dir)
    # Don't clean links here, as that forces any valid symlinks to be
    # recreated (potentially forcing a useless rebuild). We'll handle them
    # at the end of this function.
    files = []
    targets = {}
    for m in match:
        files.extend(get_glob([os.path.join(source_dir, m)]))
    for g in files:
        name = os.path.split(g)[1]
        if name not in exclude:
            target = os.path.join(target_dir, name)
            targets[target] = None
            link(g, target, verbose=verbose)
    if clean:
        # Remove any old links that are no longer valid
        for ln in existing_links:
            if ln not in targets:
                os.unlink(ln)


def get_modules(source):
    path = os.path.join(source, "modules", "*")
    globs = get_glob([path])
    mods = [(os.path.split(g)[1], g)
            for g in globs if os.path.split(g)[1] != "SConscript"
            and os.path.exists(os.path.join(g, "dependencies.py"))]
    return mods


# a version of split that doesn't return empty strings when there are no items


def split(string, sep=":"):
    return ([x.replace("@", ":")
             for x in string.replace("\\:", "@").split(sep) if x != ""])


def get_project_info(path):
    cp = os.path.join(path, ".imp_info.py")
    if os.path.exists(cp):
        with open(cp) as fh:
            return ast.literal_eval(fh.read())
    elif path in ("", "/"):
        raise ValueError("no .imp_info.py found")
    else:
        return get_project_info(os.path.split(path)[0])


_subprocesses = []


def run_subprocess(command, **kwargs):
    global _subprocesses
    # if not kwargs.has_key("stdout"):
    #    kwargs["stdout"] = subprocess.PIPE
    # if not kwargs.has_key("stderr"):
    #    kwargs["stderr"] = subprocess.PIPE
    pro = subprocess.Popen(
        command, preexec_fn=os.setsid, stderr=subprocess.PIPE,
        stdout=subprocess.PIPE, universal_newlines=True, **kwargs)
    _subprocesses.append(pro)
    output, error = pro.communicate()
    ret = pro.returncode
    if ret != 0:
        sys.stderr.write(error + '\n')
        raise OSError("subprocess failed with return code %d: %s\n%s"
                      % (ret, " ".join(command), error))
    return output


def _sigHandler(signum, frame):
    print("starting handler")
    signal.signal(signal.SIGTERM, signal.SIG_DFL)
    global _subprocesses
    for p in _subprocesses:
        print("killing", p)
        try:
            os.kill(p.pid, signal.SIGTERM)
        except Exception:
            pass
    sys.exit(1)


signal.signal(signal.SIGTERM, _sigHandler)
