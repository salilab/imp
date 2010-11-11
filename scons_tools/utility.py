from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir, Move, Copy, Scanner
import os
import sys

def file_compare(a, b):
    """Check if two files are the same, by comparing the path"""
    pa= a.abspath
    pb= b.abspath
    return cmp(pa,pb)

def get_matching(patterns):
    """provide a canonical list of files which match the passed list of patterns.
    Otherwise changes in the ordering will cause scons to rebuild things."""
    ret=[]
    for x in patterns:
        ret+=Glob(x, ondisk=True)
    ret.sort()#cmp= file_compare)
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
        for m in env['IMP_MODULES_ALL']:
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


def add_link_flags(env, modules, dependencies):
    all_dependencies=dependencies
    all_modules=modules
    for m in modules:
        all_modules+= env.get_module_modules(m)
    for m in all_modules:
        all_dependencies+= env.get_module_dependencies(m)
    final_modules=[]
    for i,m in enumerate(all_modules):
        if not m in all_modules[i+1:]:
            final_modules.append(m)
    final_dependencies=[]
    for i,d in enumerate(all_dependencies):
        if not d in all_dependencies[i+1:]:
            final_dependencies.append(d)
    module_libs=[]
    for m in final_modules:
        if m=='kernel':
            module_libs.append('imp')
        else:
            module_libs.append('imp_'+m)
    dependency_libs=[]
    for d in final_dependencies:
        dependency_libs+=env.get_dependency_libs(d)
    env.Append(LIBS=module_libs)
    env.Append(LIBS=dependency_libs)


def get_split_into_directories(paths):
    """Split the input files based on the directory containing them.
    The result is a dictionary containing list like dirname:[file0, file1...]
    """
    retdir={}
    for p in paths:
        components= str(p).split("/")
        if len(components) >1:
            cd= components[0:-1]
            fn= components[-1]
        else:
            cd="."
            fn=components[-1]
        if cd in retdir.keys:
            retdir[cd].append(fn)
        else:
            retdir[cd]=[fn]
    return retdir
