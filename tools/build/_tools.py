import glob
import os
import os.path
import shutil

def rewrite(filename, contents):
    try:
        old= open(filename, "r").read()
        if old == contents:
            return
        else:
            print "Different", filename
    except:
        print "Missing", filename
    open(filename, "w").write(contents)


def mkdir(path, clean=True):
    if os.path.isdir(path):
        # remove any old links
        if clean:
            for f in glob.glob(os.path.join(path, "*")):
                if os.path.islink(f):
                    os.unlink(f)
        return
    if os.path.isfile(path):
        os.unlink(path)
    os.makedirs(path)

def link(source, target, verbose=False):
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
        os.unlink(tpath)
    if verbose:
        print "linking", spath, tpath
    os.symlink(spath, tpath)


def link_dir(source_dir, target_dir, match=["*"], clean=True, verbose=False):
    if type(match) != list:
        adkfjads;lkfjd;laskjfdl;k
    #print "linking", source_dir, target_dir
    mkdir(target_dir, clean=clean)
    files=[]
    for m in match:
        files.extend(glob.glob(os.path.join(source_dir, m)))
    for g in files:
        name=os.path.split(g)[1]
        if name != "SConscript":
        #print g, name, os.path.join(target_dir, name)
            link(g, os.path.join(target_dir, name), verbose=verbose)

def get_modules(source):
    path=os.path.join(source, "modules", "*")
    globs=glob.glob(path)
    mods= [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")
            and os.path.exists(os.path.join(g, "description"))]
    return mods

def get_module_data(module_path):
    df= os.path.join(module_path, "description")
    required_modules=[]
    optional_modules=[]
    required_dependencies=[]
    optional_dependencies=[]
    exec open(df, "r").read()
    return {"required_modules":required_modules,
            "optional_modules":optional_modules,
            "required_dependencies":required_dependencies,
            "optional_dependencies":optional_dependencies}

def get_biological_systems(source):
    path=os.path.join(source, "biological_systems", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")]

def get_applications(source):
    path=os.path.join(source, "applications", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")]
