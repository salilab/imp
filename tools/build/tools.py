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

def get_applications(source):
    path=os.path.join(source, "applications", "*")
    globs=glob.glob(path)
    mods= [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")
            and os.path.exists(os.path.join(g, "description"))]
    return mods


def get_dependency_description(path):
    name= os.path.splitext(os.path.split(path)[1])[0]
    headers=""
    libraries=""
    extra_libraries=""
    version_cpp=""
    version_headers=""
    body=""
    exec(open(path, "r").read())
    passlibs=split(libraries)
    if len(passlibs)==0:
        passlibs=None
    passheaders=split(headers)
    if len(passheaders)==0:
        passheaders=None
    else:
        pass
    extra_libs=split(extra_libraries)
    build=os.path.splitext(path)[0]+".install"
    if os.path.exists(build):
        build_script=str(build)
    else:
        build_script=None
    return {"name":name,
            "headers":passheaders,
            "libraries":passlibs,
            "extra_libraries":extra_libs,
            "build_script": build_script,
            "body":body,
            "version_cpp":split(version_cpp),
            "version_headers":split(version_headers)}

def get_module_description(source, module, extra_data_path, root="."):
    df= os.path.join(root, source, "modules", module, "description")
    if os.path.exists(df):
        required_modules=""
        optional_modules=""
        required_dependencies=""
        optional_dependencies=""
        exec open(df, "r").read()
        return {"required_modules":split(required_modules),
                "optional_modules":split(optional_modules),
                "required_dependencies":split(required_dependencies),
                "optional_dependencies":split(optional_dependencies)}
    else:
        info= get_module_info(module, extra_data_path)
        return {"required_modules":info["modules"],
                "optional_modules":[],
                "required_dependencies":info["dependencies"],
                "optional_dependencies":[]}

def get_application_description(source, module, extra_data_path, root="."):
    df= os.path.join(root, source, "applications", module, "description")
    required_modules=""
    optional_modules=""
    required_dependencies=""
    optional_dependencies=""
    exec open(df, "r").read()
    return {"required_modules":split(required_modules),
                "optional_modules":split(optional_modules),
                "required_dependencies":split(required_dependencies),
                "optional_dependencies":split(optional_dependencies)}

dependency_info_cache={}

def get_dependency_info(dependency, extra_data_path, root="."):
    global dependency_info_cache
    if dependency_info_cache.has_key(dependency):
        return dependency_info_cache[dependency]
    df= os.path.join(root, "data", "build_info", dependency)
    if not os.path.exists(df) and extra_data_path != "":
        df= os.path.join(extra_data_path, "build_info",dependency)
    ok=False
    libraries=""
    version=""
    includepath=""
    libpath=""
    try:
        exec open(df, "r").read()
    except:
        print >> sys.stderr, "Error reading dependency", dependency, "at", df
        return {"ok":False}
    ret= {"ok":ok,
            "libraries":split(libraries),
            "version":split(version),
            "includepath":includepath,
            "libpath":libpath}
    dependency_info_cache[dependency]=ret;
    return ret

module_info_cache={}

def get_module_info(module, extra_data_path, root="."):
    global module_info_cache
    if module_info_cache.has_key(module):
        return module_info_cache[module]
    if module.find("/") != -1:
        raise ValueError("module name invalid: "+module)
    df= os.path.join(root, "data", "build_info", "IMP."+module)
    external=False
    if not os.path.exists(df) and extra_data_path != "":
        external=True
        df= os.path.join(extra_data_path, "build_info", "IMP."+module)
    ok=False
    modules=""
    unfound_modules=""
    dependencies=""
    unfound_dependencies=""
    swig_includes=""
    exec open(df, "r").read()
    ret= {"ok":ok,
            "modules":split(modules),
            "unfound_modules":split(unfound_modules),
            "dependencies":split(dependencies),
            "unfound_dependencies":split(unfound_dependencies),
            "swig_includes":split(swig_includes)}
    if external:
        ret["external"]=True
    module_info_cache[module]=ret;
    return ret

def get_application_info(module, extra_data_path, root="."):
    df= os.path.join(root, "data", "build_info", "IMP."+module)
    if not os.path.exists(df) and extra_data_path != "":
        df= os.path.join(extra_data_path, "build_info", "IMP."+model)
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
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1].find("SConscript")==-1)]

def get_applications(source):
    path=os.path.join(source, "applications", "*")
    globs=glob.glob(path)
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1].find("SConscript") ==-1)]

# a version of split that doesn't return empty strings when there are no items
def split(string, sep=":"):
    return [x for x in string.split(sep) if x != ""]


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
                d[item] = set([x for x in dep if x not in ordered])
        data = d
    return ret

order_cache=None

def get_sorted_order(root="."):
    global order_cache
    if order_cache:
        return order_cache
    order= split(open(os.path.join(root, "data", "build_info", "sorted_modules"), "r").read(), "\n")
    order_cache= order
    return order

def setup_sorted_order(source, extra_data_path):
    data={}
    for m, path in get_modules(source):
        df= os.path.join(path, "description")
        if not os.path.exists(df):
            continue
        info= get_module_description(source, m, extra_data_path)
        data[m]= set(info["required_modules"] + info["optional_modules"])
        # toposort is destructive
        # get external modules, a bit sloppy for now
    while True:
        to_add={}
        for mk in data.keys():
            for m in data[mk]:
                if not data.has_key(m):
                    print 'adding', m
                    info= get_module_info(m, extra_data_path)
                    to_add[m]=info["modules"]
        for m in to_add.keys():
            data[m]= to_add[m]
        if len(to_add.keys()) == 0:
            break
    sorted= toposort2(data)
    rewrite(os.path.join("data", "build_info", "sorted_modules"),
                  "\n".join(sorted))
    #return sorted

def get_dependent_modules(modules, extra_data_path, root="."):
    for x in modules:
        if x.find("/") != -1:
            raise value_error("bad module name: "+x)
    sorted_order=get_sorted_order(root)
    new_modules=modules
    all_modules=new_modules
    while len(new_modules)>0:
        m= new_modules[-1]
        new_modules=new_modules[:-1]
        cur_modules= [x for x in get_module_info(m, extra_data_path, root)["modules"]\
                      if x not in all_modules]
        all_modules+=cur_modules
        new_modules+=cur_modules
    all_modules.sort(lambda x,y: -cmp(sorted_order.index(x), sorted_order.index(y)))
    return all_modules


def get_dependent_dependencies(modules, dependencies, extra_data_path,
                               root="."):
    for x in modules:
        if x.find("/") != -1:
            raise value_error("bad module name: "+x)
    sorted_order=get_sorted_order(root)
    ret_names=[]
    for m in modules:
        info= get_module_info(m, extra_data_path, root)
        ret_names.extend(info["dependencies"])
    ret= list(set(ret_names+dependencies))
    return ret

def get_disabled_modules(extra_data_path, root="."):
    all= glob.glob(os.path.join(root, "data", "build_info", "IMP.*"))
    modules=[os.path.splitext(a)[1][1:] for a in all]
    return [x for x in modules if not get_module_info(x,extra_data_path, root)["ok"]]
