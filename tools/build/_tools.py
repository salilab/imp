import glob
import os
import os.path
import copy
import shutil
import sys

def rewrite(filename, contents):
    try:
        old= open(filename, "r").read()
        if old == contents:
            return
        else:
            print "Different", filename
    except:
        pass
        #print "Missing", filename
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
    required_modules=""
    optional_modules=""
    required_dependencies=""
    optional_dependencies=""
    exec open(df, "r").read()
    return {"required_modules":split(required_modules),
            "optional_modules":split(optional_modules),
            "required_dependencies":split(required_dependencies),
            "optional_dependencies":split(optional_dependencies)}

def get_dependency_data(dependency):
    df= os.path.join("info", dependency)
    ok=False
    libs=""
    try:
        exec open(df, "r").read()
    except:
        print >> sys.stderr, "Error reading dependency", dependency
        return {"ok":False}
    return {"ok":ok,
            "libs":split(libs)}

def get_configured_module_data(model):
    df= os.path.join("info", "IMP."+model)
    ok=False
    modules=""
    unfound_modules=""
    dependencies=""
    unfound_dependencies=""
    exec open(df, "r").read()
    return {"ok":ok,
            "modules":split(modules),
            "unfound_modules":split(unfound_modules),
            "dependencies":split(dependencies),
            "unfound_dependencies":split(unfound_dependencies)}


def get_biological_systems(source):
    path=os.path.join(source, "biological_systems", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")]

def get_applications(source):
    path=os.path.join(source, "applications", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")]

# a version of split that doesn't return empty strings when there are no items
def split(string):
    return [x for x in string.split(":") if x != ""]


def toposort2(data):
    ret=[]
    while True:
        ordered = set([item for item,dep in data.items() if not dep])
        if not ordered:
            break
        ret.extend(sorted(ordered))
        d = {}
        for item,dep in data.items():
            if item not in ordered:
                d[item] = set([x for x in dep if x[0] not in ordered])
        data = d
    return ret

def get_sorted_order_and_dependencies(source):
    data={}
    for m, path in get_modules(source):
        df= os.path.join(path, "description")
        if not os.path.exists(df):
            continue
        required_modules=""
        optional_modules=""
        exec open(df, "r").read()
        data[m]= set([(x, False) for x in split(required_modules)]\
                     + [(x, True) for x in split(optional_modules)])
        # toposort is destructive
    data2=copy.deepcopy(data)
    sorted= toposort2(data)
    for m in sorted:
        direct= data2[m]
        all=[]
        for md, opt in direct:
            all.append((md, opt))
            all.extend([(x[0], x[1] or opt) for x in data2[md]])
        filtered=list(set([x for x in all if not x[1] or (x[0], False) not in all]))
        filtered.sort()
        data2[m]=filtered
    return sorted, data2
