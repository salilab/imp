import glob
import os
import os.path
import copy
import shutil
import sys
import difflib

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
        return
    if os.path.isfile(path):
        os.unlink(path)
    os.makedirs(path)

def quote(st):
    out= st.replace("\\f", ""). replace("\\b", "").replace("\\", "\\\\").replace("\"", "\\\"")
    return out

def get_glob(patterns):
    ret=[]
    for p in patterns:
        c= glob.glob(p)
        c.sort()
        ret+=c
    return ret

def rewrite(filename, contents, verbose=True):
    try:
        old= open(filename, "r").read()
        if old == contents:
            return
        elif verbose:
            print "    Different", filename
            for l in difflib.unified_diff(old.split("\n"), contents.split("\n")):
                stl= str(l)
                if (stl[0]=='-' or stl[0]=='+') and stl[1] != '-' and stl[1] != '+':
                    print "    "+stl
    except:
        pass
        #print "Missing", filename
    dirpath=os.path.split(filename)[0]
    if dirpath != "":
        mkdir(dirpath, False)
    open(filename, "w").write(contents)

def rmdir(path):
    try:
        shutil.rmtree(path)
    except:
        pass

def link(source, target, verbose=False):
    tpath= os.path.abspath(target)
    spath= os.path.abspath(source)
    #print tpath, spath
    if not os.path.exists(spath):
        if verbose:
            print "no source", spath
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
        print "linking", spath, tpath
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
    if type(match) != list:
        adkfjads;lkfjd;laskjfdl;k
    exclude = exclude + ["SConscript", "CMakeLists.txt", ".svn"]
    #print "linking", source_dir, target_dir
    if clean:
        existing_links = get_existing_links(target_dir)
    # Don't clean links here, as that forces any valid symlinks to be
    # recreated (potentially forcing a useless rebuild). We'll handle them
    # at the end of this function.
    mkdir(target_dir, clean=False)
    files=[]
    targets={}
    for m in match:
        files.extend(get_glob([os.path.join(source_dir, m)]))
    for g in files:
        name=os.path.split(g)[1]
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
    path=os.path.join(source, "modules", "*")
    globs=get_glob([path])
    mods= [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")
            and os.path.exists(os.path.join(g, "dependencies.py"))]
    return mods

def get_applications(source):
    path=os.path.join(source, "applications", "*")
    globs=get_glob([path])
    mods= [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1] != "SConscript")
            and os.path.exists(os.path.join(g, "dependencies.py"))]
    return mods


def get_dependency_description(path):
    name= os.path.splitext(os.path.split(path)[1])[0]
    pkg_config_name=None
    headers=""
    libraries=""
    extra_libraries=""
    version_cpp=""
    version_headers=""
    body=""
    python_module=""
    is_cmake=False
    exec(open(path, "r").read())
    passlibs=split(libraries)
    passheaders=split(headers)
    extra_libs=split(extra_libraries)
    build=os.path.splitext(path)[0]+".install"
    if os.path.exists(build):
        build_script=str(build)
    else:
        build_script=None
    cmakef=os.path.splitext(path)[0]+".cmake"
    if os.path.exists(cmakef):
        cmake="include(\"${PROJECT_SOURCE_DIR}/%s\")"%(to_cmake_path(os.path.splitext(path)[0]+".cmake"))
    else:
        cmake=""
    if pkg_config_name is None:
        pkg_config_name=name.lower()
    return {"name":name,
            "pkg_config_name":pkg_config_name,
            "headers":passheaders,
            "libraries":passlibs,
            "extra_libraries":extra_libs,
            "build_script": build_script,
            "body":body,
            "version_cpp":split(version_cpp),
            "version_headers":split(version_headers),
            "cmake":cmake,
            "python_module":python_module}

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

def get_all_modules(source, modules, extra_data_path, ordered, root="."):
    ret=[]
    stack=modules
    while len(stack) > 0:
        cur= stack[-1]
        stack=stack[:-1]
        descr= get_module_description(source, cur, extra_data_path)
        for m in descr["required_modules"] + descr["optional_modules"]:
            if m not in ret:
                ret.append(m)
                stack.append(m)
    ret.sort(cmp= lambda x,y: cmp(ordered.index(x), ordered.index(y)))
    return ret

def get_all_dependencies(source, modules, extra_data_path, ordered, root="."):
    mods= modules + get_all_modules(source, modules, extra_data_path, ordered, root)
    ret=[]
    for m in mods:
        descr= get_module_description(source, m, extra_data_path)
        for d in descr["required_dependencies"] + descr["optional_dependencies"]:
            if d not in ret:
                ret.append(d)
    return ret

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
    swigpath=""
    #try:
    exec open(df, "r").read()
    #except:
    #    print >> sys.stderr, "Error reading dependency", dependency, "at", df
    ret= {"ok":ok,
          "libraries":split(libraries),
          "version":split(version),
          "includepath":includepath,
          "libpath":libpath,
          "swigpath":swigpath}
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
    swig_wrapper_includes=""
    swig_includes=""
    swig_path=""
    include_path=""
    lib_path=""
    exec open(df, "r").read()
    ret= {"ok":ok,
            "modules":split(modules),
            "unfound_modules":split(unfound_modules),
            "dependencies":split(dependencies),
            "unfound_dependencies":split(unfound_dependencies),
            "swig_includes":split(swig_includes),
            "swig_wrapper_includes":split(swig_wrapper_includes)}
    if external:
        ret["external"]=True
    module_info_cache[module]=ret;
    return ret

def get_application_info(module, extra_data_path, root="."):
    df= os.path.join(root, "data", "build_info", "IMP."+module)
    if not os.path.exists(df) and extra_data_path != "":
        df= os.path.join(extra_data_path, "build_info", "IMP."+module)
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
    globs=get_glob([path])
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1].find("SConscript")==-1)]

def get_applications(source):
    path=os.path.join(source, "applications", "*")
    globs=get_glob([path])
    return [(os.path.split(g)[1], g) for g in globs if (os.path.split(g)[1].find("SConscript") ==-1)]

# a version of split that doesn't return empty strings when there are no items
def split(string, sep=":"):
    return [x for x in string.split(sep) if x != ""]


def toposort2(data):
    ret=[]
    while True:
        ordered = [item for item,dep in data.items() if not dep]
        if not ordered:
            break
        ret.extend(sorted(ordered))
        d = {}
        for item,dep in data.items():
            if item not in ordered:
                d[item] = [x for x in dep if x not in ordered]
        data = d
    return ret

order_cache=None

def get_all_configured_applications(root="."):
    apps = split(open(os.path.join(root, "data", "build_info", "applications"), "r").read(), "\n")
    return apps

def get_sorted_order(root="."):
    global order_cache
    if order_cache:
        return order_cache
    order= split(open(os.path.join(root, "data", "build_info", "sorted_modules"), "r").read(), "\n")
    order_cache= order
    return order

def set_sorted_order(sorted, outpath=os.path.join("data", "build_info", "sorted_modules")):
    global order_cache
    order_cache=sorted
    rewrite(outpath,
                  "\n".join(sorted))

def compute_sorted_order(source, extra_data_path):
    data={}
    for m, path in get_modules(source):
        df= os.path.join(path, "description")
        if not os.path.exists(df):
            continue
        info= get_module_description(source, m, extra_data_path)
        data[m]= info["required_modules"] + info["optional_modules"]
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
    return sorted
def setup_sorted_order(source, extra_data_path,
                       outpath=os.path.join("data", "build_info", "sorted_modules")):
    sorted= compute_sorted_order(source, extra_data_path)
    set_sorted_order(sorted, outpath)
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
    all= get_glob([os.path.join(root, "data", "build_info", "IMP.*")])
    modules=[os.path.splitext(a)[1][1:] for a in all]
    return [x for x in modules if not get_module_info(x,extra_data_path, root)["ok"]]

def get_application_executables(path):
    """Return a list of tuples of ([.cpps], [includepath])"""
    def _handle_cpp_dir(path):
        cpps= get_glob([os.path.join(path, "*.cpp")])
        libcpps= get_glob([os.path.join(path, "lib", "*.cpp")])
        if len(libcpps) > 0:
            includes = [os.path.join(path, "lib")]
        else:
            includes = []
        return [([c]+libcpps, includes) for c in cpps]

    ret=_handle_cpp_dir(path)
    for d in get_glob([os.path.join(path, "*")]):
        if not os.path.isdir(d):
            continue
        if os.path.split(d)[1] == "test":
            continue
        if os.path.split(d)[1] == "lib":
            continue
        ret+= _handle_cpp_dir(d)
    return ret
