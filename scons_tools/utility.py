from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir, Move, Copy, Scanner
import data
import os
import os.path
import sys
import environment
import dependency
import subprocess

def file_compare(a, b):
    """Check if two files are the same, by comparing the path"""
    pa= a.path
    pb= b.path
    return cmp(pa,pb)

def get_matching(patterns):
    """provide a canonical list of files which match the passed list of patterns.
    Otherwise changes in the ordering will cause scons to rebuild things."""
    ret=[]
    for x in patterns:
        ret+=[x for x in Glob(x) if not os.path.islink(x.abspath)]
    ret.sort(file_compare)
    return ret

def get_matching_recursive(patterns):
    allpatterns=[x for x in patterns]
    for p in ['*/', '*/*/']:
        allpatterns+= [p+x for x in patterns]
    #print allpatterns
    ret= get_matching(allpatterns)
    #print [str(f) for f in ret]
    return ret


def report_error(env, string):
    print >> sys.stderr, string
    env.Exit(1)

def _check_module_names(modules):
    for m in modules:
        if m.startswith("imp") or m.startswith("IMP"):
            error("Module names should not include imp (eg 'algebra', not 'imp.algebra'): "+m)


def postprocess_lib(env, target):
    """Do any handling of already created lib that is needed"""
    def do_mac_name_thing(env, source, target):
        """Set the names and paths for the mac libraries based on the current locations
        of the libs."""
        targetdir= os.path.split(target[0].abspath)[0]
        sourcedir= os.path.split(source[0].abspath)[0]
        #print targetdir
        #print sourcedir
        env.Execute("install_name_tool -id %s %s"% (target[0].abspath, target[0].abspath))
        env.Execute("install_name_tool -change %s %s %s"%(os.path.join(sourcedir, 'libimp.dylib'),
                                                          os.path.join(targetdir, 'libimp.dylib'),
                                                          target[0].abspath))
        for m in environment.get_current_modules(env):
            oname=os.path.join(sourcedir, "libimp_"+m+".dylib")
            nname=os.path.join(targetdir, "libimp_"+m+".dylib")
            env.Execute("install_name_tool -change %s %s %s"%(oname,
                                                              nname,
                                                              target[0].abspath))

    if env['PLATFORM'] == 'darwin':
        dir= os.path.split(target[0].abspath)[0]
        env.AddPostAction(target, do_mac_name_thing)


def make_static_build(env):
    """Make the build static if appropriate"""
    if env['CC'] == 'gcc':
        env.Append(LINKFLAGS=['-static'])
    else:
        print >> sys.stderr, "WARNING: Static builds only supported with GCC, ignored."

def unmake_static_build(env):
    """Make the build static if appropriate"""
    if env['CC'] == 'gcc':
        lf= env['LINKFLAGS']
        lf.remove('-static')
        env.Replace(LINKFLAGS=lf)
    else:
        print >> sys.stderr, "WARNING: Static builds only supported with GCC, ignored."



def get_split_into_directories(paths):
    """Split the input files based on the directory containing them.
    The result is a dictionary containing list like dirname:[file0, file1...]
    """
    retdir={}
    for p in paths:
        components= str(p).split("/")
        if len(components) >1:
            cd= components[0]
            fn= components[-1]
        else:
            cd=""
            fn=components[-1]
        if cd in retdir.keys():
            retdir[cd].append(fn)
        else:
            retdir[cd]=[fn]
    return retdir






def configure(env, name, type, version, required_modules=[],
              optional_dependencies=[], optional_modules=[],
              required_dependencies=[]):
    """Returns ok, version, found_optional_modules, found_optional_dependencies"""
    if not version:
        version= env['IMP_VERSION']
    disabled=[x for x in env.get("disabledmodules", '').split(":")]
    if name in disabled:
        print type.capitalize(), name, "explicitly disabled "
        return (None, None, None, None)
    found_required_modules= data.get(env).get_found_modules(required_modules)
    for m in required_modules:
        if m not in found_required_modules:
            msg="%s"+type.capitalize()+" "+ name +" disabled due to disabled module IMP."+m+"%s"
            print msg%(env['IMP_COLORS']['red'], env['IMP_COLORS']['end'])
            return (None, None, None, None)
    for m in required_dependencies:
        if not data.get_dependency(m)["ok"]:
            msg= "%s"+type.capitalize()+" "+ name+" disabled due to missing dependency "+m+"%s"
            print msg%(env['IMP_COLORS']['red'], env['IMP_COLORS']['end'])
            return (None, None, None, None)
    found_optional_modules=data.get(env).get_found_modules(optional_modules)
    found_optional_dependencies=data.get_found_dependencies(optional_dependencies)
    outversion= version
    if len(found_optional_dependencies +found_optional_modules) > 0:
        outversion=outversion+" with "+", ".join(found_optional_dependencies +found_optional_modules)
    version=outversion
    msg="%sConfiguring " +type+" "+name + "%s version \""+ version + "\""
    print msg%(env['IMP_COLORS']['green'], env['IMP_COLORS']['end'])
    #if len(required_modules+required_dependencies)>0:
    #    print "  (requires " +", ".join(required_modules+required_dependencies) +")"
    found_optional_modules.sort()
    found_optional_dependencies.sort()
    required_modules.sort()
    required_dependencies.sort()
    return (environment.get_named_environment(env, name,
                                  required_modules+found_optional_modules,
                                  required_dependencies+found_optional_dependencies),
            version, found_optional_modules, found_optional_dependencies)

def get_without_extension(name):
    if str(name).rfind('.') == -1:
        return name
    else:
        return str(name)[0:str(name).rfind('.')]

def get_link_name_from_name(name):
    base= get_without_extension(name).split("/")[-1]
    link=base.replace(' ', '_').replace(':', '_')
    return link

def get_display_from_name(name):
    base= get_without_extension(name).split("/")[-1]
    text=base.replace('_', ' ')
    return text

def get_link_from_name(name):
    link=get_link_name_from_name(name)
    text=get_display_from_name(name)
    return "\\ref "+link+' "'+text+'"'


def add_to_include_path(env, path):
    if not path:
        return
    #print env["CXXVERSION"], env["CXXVERSION"].split(".")[0] == "4", env["CXXVERSION"].split(".")[1] == "0"
    #print not (sys.platform=="darwin"
    #                and env["CXXVERSION"].split(".")[0] == "4"
    #                and env["CXXVERSION"].split(".")[1] == "0")
    if dependency.gcc.get_is_gcc(env)\
           and not path.startswith("#"):
        if sys.platform=="darwin"\
            and env["CXXVERSION"].split(".")[0] == "4"\
            and env["CXXVERSION"].split(".")[1] == "0":
            env.Append(CXXFLAGS=["-I"+path])
        else:
            env.Append(CXXFLAGS=["-isystem",path])
    else:
        env.Append(CPPPATH=[path])
def get_abspaths(env, name, pl):
    # ick
    if type(pl)==type(""):
        pl=pl.split(os.path.pathsep)
    if pl==[]:
        return []
    if type(pl) != type([]):
        report_error(env, "not a list: "+pl)
    bad=".."+os.path.sep
    ret=[]
    #print pl
    for p in pl:
        #if p.startswith(bad):
        #    utility.report_error(env, "Path lists such as "+name+" should not contain relative paths, bad things will happen.")
        #print p
        if p.startswith(bad):
            ret.append(Dir("#/"+p).abspath)
        else:
            ret.append(Dir(p).abspath)
        if not os.path.isdir(ret[-1]):
            #print "bad"
            report_error(env, "The path "+p+" in "+name+" does not exist.")

    #print ret
    #print name, "from", pathlist, "to", ":".join(ret)
    return ret
def get_source_path(env, path):
    """Return a path in the source directory, supporting repositories"""
    if env.get("repository"):
        return File(env['repository']+"/"+path).abspath
    else:
        return File("#/"+path).abspath
def get_paths(env, paths):
    if not paths:
        return []
    spl= paths.split(os.path.pathsep)
    return [x for x in spl if x != ""]
def get_env_paths(env, name):
    paths= env.get(name, "")
    #print "#########", name, paths
    ret= get_paths(env, paths)
    #print ret
    return ret

def add_to_lib_path(env, path):
    if not path:
        return
    env.Append(LIBPATH=[path])

def get_dylib_name(env):
    if env['PLATFORM'] == 'posix' or env['PLATFORM']=='sunos':
        return "LD_LIBRARY_PATH"
    elif env['PLATFORM'] == 'darwin':
        return "DYLD_LIBRARY_PATH"
    else:
        return None

def get_ld_path(env):
    ret=[]
    # Setting DYLD_LIBRARY_PATH on Mac OS X (Darwin) is usually a bad idea
    # (e.g. can cause conflicts when different versions of libraries are
    # provided by MacPorts/Fink/HomeBrew and the OS, since it overrides the
    # rpath) and shouldn't be necessary anyway (paths to libraries are
    # stored in the binary)
    add_libpath = not env['IMP_USE_RPATH'] and env.get('libpath', None) \
                  and env['PLATFORM'] != 'darwin'
    if add_libpath:
        ret=get_env_paths(env, 'libpath')
    ret.extend(get_env_paths(env, 'ldlibpath'))
    #print get_env_paths(env, 'ldlibpath')
    return ":".join(get_abspaths(env, "ldpath", ret))

def get_python_path(env):
    ret=[]
    ret.extend(get_env_paths(env, 'pythonpath'))
    #print get_env_paths(env, 'ldlibpath')
    return ":".join(get_abspaths(env, "pythonpath", ret))


def get_separator(env):
    if env['PLATFORM'] == 'win32' or env['wine']:
        return ";"
    else:
        return ":"


def get_python_result(env, setup, cmd):
    #print "hi"
    pp= get_python_path(env)
    if pp:
        #print 1
        ap=get_abspaths(env, 'pythonpath', pp)
        #print ap
        setpp="import sys; sys.path.extend("+str(ap)+");"
    else:
        setpp=""
    varname= get_dylib_name(env)
    ldpath= get_ld_path(env)
    #print "here", ldpath
    if varname:
        if len(ldpath)>0 and os.environ.has_key(varname):
            olddylib= os.environ[varname]
            os.environ[varname]=ldpath+get_separator(env)+olddylib
        else:
            os.environ[varname]=ldpath
            olddylib=None
    #print setup, cmd
    scmd = setpp+setup+";"+"print "+cmd + "\n"
    #print scmd
    #print "\n*****"
    #print scmd
    #print "*****"
    #print varname, os.environ[varname]
    #print "*****"
    #print ldpath
    #print "*****"
    sp=subprocess.Popen(env['PYTHON'], shell=True, stdout=subprocess.PIPE,
                        stdin=subprocess.PIPE,
                        stderr= subprocess.PIPE,
                        env=os.environ)
    sp.stdin.write(scmd)
    sp.stdin.close()
    ret=sp.stdout.read()
    eret=sp.stderr.read()
    if len(eret)>0:
        print eret
    #print "\n******\nreturned", ret
    #print "\n******\nerror", eret
    #print ret[:-1]
    if varname:
        if olddylib:
            os.environ[varname]=olddylib
        else:
            del os.environ[varname]
    #print "done"
    #print "found", ret, "done", ret[:-1], "end"
    return ret[:-1]
