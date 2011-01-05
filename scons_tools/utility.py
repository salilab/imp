from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir, Move, Copy, Scanner
import data
import os
import sys
import environment
import dependency

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
        ret+=Glob(x)
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
        for m in data.get(env).modules.keys():
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


def _get_cwd_version(env, version, optional_dependencies=[], optional_modules=[]):
    if env['SVNVERSION'] and env['svn']:
        if env.get('repository'):
            rep=env['repository']
            dp= os.path.commonprefix([Dir("#/").abspath, Dir(".").abspath])
            pf=Dir(".").abspath[len(dp)+1:]
            #print pf
            reppath=Dir("#/"+rep).abspath
            path=os.path.join(reppath, pf)
        else:
            path=Dir(".").abspath
        try:
            vr= os.popen(env['SVNVERSION'] + ' ' + path).read()
            version= "SVN "+vr.split("\n")[0]
        except OSError, detail:
            print >> sys.stderr, "WARNING: Could not run svnversion: %s" % str(detail)

    if len(optional_dependencies+ optional_modules)>0:
        version=version+" with "+", ".join(optional_dependencies+ optional_modules)
    return version




def configure(env, name, type, version, required_modules=[],
              optional_dependencies=[], optional_modules=[],
              required_dependencies=[]):
    """Returns ok, version, found_optional_modules, found_optional_dependencies"""
    disabled=["IMP."+x for x in env.get("disabledmodules", '').split(":")]
    if name in disabled:
        print type.capitalize(), name, "explicitly disabled "
        return (None, None, None, None)
    found_required_modules= data.get(env).get_found_modules(required_modules)
    for m in required_modules:
        if m not in found_required_modules:
            print type.capitalize(), name, "disabled due to disabled module "\
                  "IMP."+m
            return (None, None, None, None)
    for m in required_dependencies:
        if not data.get(env).dependencies[m].ok:
            print type.capitalize(), name, "disabled due to missing dependency "\
                  +m
            return (None, None, None, None)
    found_optional_modules=data.get(env).get_found_modules(optional_modules)
    found_optional_dependencies=data.get(env).get_found_dependencies(optional_dependencies)
    version= _get_cwd_version(env, version,
                              optional_dependencies=found_optional_dependencies,
                              optional_modules=found_optional_modules)
    print "Configuring", type, name,"version", version
    if len(required_modules+required_dependencies)>0:
        print "  (requires " +", ".join(required_modules+required_dependencies) +")"
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
    if dependency.gcc.get_is_gcc(env):
        env.Append(CXXFLAGS=["-I"+path])
    else:
        env.Append(CPPPATH=[path])

def add_to_lib_path(env, path):
    if not path:
        return
    env.Append(LIBPATH=[path])
