import scons_tools.utility
import scons_tools.data
import scons_tools.paths
import SCons
import os
import stat
from SCons.Script import File, Action, Dir, PathVariable, GetOption

def _search_for_deps(context, libname, extra_libs, headers, body, possible_deps):
    if type(headers) != list:
        headers=[headers]
    context.Message("Checking for library "+str(libname)+"...")
    for i in range(0,len(possible_deps)+1):
        lc= extra_libs+possible_deps[0:i]
        #print "Trying "+ str(i) +" with " +str(lc)
        olibs= context.env.get('LIBS', [])
        if libname is not None:
            context.env.Append(LIBS=[libname]+lc)
        else:
            context.env.Append(LIBS=lc)
            #print context.env["LIBPATH"]
            #print context.env["CPPPATH"]
        #print context.env['LINKFLAGS']
        #print "checking", libname, lc
        prog=["#include <%s>"%x for x in headers]
        prog.append("int main(int, char*[]) {")
        prog.append(body)
        prog.append("}")
        ret=context.sconf.TryLink( "\n".join(prog), ".cpp")
        context.env.Replace(LIBS=olibs)
        if ret:
            context.Result("yes")
            if libname is None:
                libs = lc
            else:
                libs = [libname] + lc
            return (True, libs, None)
    context.Result("no")
    return (False, None, None)

def add_dependency_link_flags(env, dependencies):
    for d in dependencies:
        env.Append(LIBS=scons_tools.data.get(env).dependencies[d].libs)

def _get_version(context, name, includepath, versioncpp, versionheader):
    if versioncpp:
        context.Message('Checking for version of '+name+"...")
        vs="<< ' ' << ".join(versioncpp)
        #if includepath:
        #    oldcpp= context.env.get('CPPPATH', None)
        #    context.env.Replace(CPPPATH=context.env.get('CPPPATH', [])[:]+[includepath])
        #print context.env['LINKFLAGS'], context.env['RPATH']
        r = context.TryRun("#include <"+versionheader+">\n"+\
                             """#include <iostream>

        int main()
        {
            std::cout << """+vs +"""<<std::endl;
            return 0;
        }
""", '.cpp')
        if includepath:
            pass
            #context.env.Replace(CPPPATH=oldcpp)
        if not r[0]:
            context.Result("None")
            return None
        else:
            v= r[1].split('\n')[0]
            if type(versioncpp) == type([]):
                version=v.split()
            else:
                version=v
        if type(version) == type([]):
            context.Result(" ".join(version))
        else:
            context.Result(str(version))
        return version
    else:
        return None

def check_lib(context, name, lib, headers, body="", extra_libs=[], versioncpp=None,
              versionheader=None, search_build=False):
    if type(headers) != type([]):
        scons_tools.utility.report_error(context.env,
                                         "The header argument must be given as a list. It was not for "+name)
    if lib is not None and type(lib) != type([]):
        scons_tools.utility.report_error(context.env,
                                         "The lib argument must be given as a list. It was not for "+name)
    if versioncpp != None and type(versioncpp) != type([]):
        scons_tools.utility.report_error(context.env,
                                         "The versioncpp argument must be given as a list. It was not for "+name)
    #oldflags= context.env.get('LINKFLAGS')
    #context.env.Replace(LINKFLAGS=context.env['IMP_BIN_LINKFLAGS'])
    #print context.env["LIBPATH"]
    #print context.env["CPPPATH"]

    if not search_build:
        swap_flags=True
        oldcpppath= context.env['CPPPATH']
        oldlibpath= context.env['LIBPATH']
        cpppath=[x for x in oldcpppath if x != Dir("#/build/include").abspath]
        libpath=[x for x in oldlibpath if x != Dir("#/build/lib").abspath]
        context.env.Replace(CPPPATH=cpppath)
        context.env.Replace(LIBPATH=libpath)
    else:
        swap_flags=False

    if lib is not None and len(lib) > 0:
        #print lib, name
        ret=_search_for_deps(context, lib[0], lib[1:], headers, body, extra_libs)
    else:
        print "Checking for header-only dependency", name+"...",
        prog=[]
        for h in headers:
          prog.append("#include <%s>"%h)
        prog.append("int main(int, char*[]) {")
        prog.append(body)
        prog.append("}")
        ret=(context.sconf.TryLink( "\n".join(prog), ".cpp"),[])
        if ret[0]:
            print "yes"
        else:
            print "no"
    if not ret[0]:
        #context.env.Replace(LINKFLAGS=oldflags)
        ret= (ret[0], ret[1], None)
    elif context.env['IMP_OUTER_ENVIRONMENT']['IMP_BUILD_STATIC'] and lib != None:
        scons_tools.utility.make_static_build(context.env)
        if type(lib) == list:
            bret=_search_for_deps(context, lib[0], lib[1:], headers, body, extra_libs)
        else:
            bret=_search_for_deps(context, lib, [], headers, body, extra_libs)
        scons_tools.utility.unmake_static_build(context.env)
        # should be the sum of the two
        if bret[0]:
            #context.env.Replace(LINKFLAGS=oldflags)
            ret= (bret[0], ret[1]+bret[1], _get_version(context, name, None,
                                                          versioncpp,
                                                          versionheader))
        else:
            #context.env.Replace(LINKFLAGS=oldflags)
            ret= (False, [], None)
    else:
        vers= _get_version(context, name, None, versioncpp, versionheader)
    #print "version", vers
    #context.env.Replace(LINKFLAGS=oldflags)
        ret=  (True, ret[1], vers)
    if swap_flags:
        context.env.Replace(LIBPATH=oldlibpath)
        context.env.Replace(CPPPATH=oldcpppath)
    return ret

def get_dependency_string(name):
    lname= name.lower()
    nname=lname.replace(".", "_")
    return nname

# return (ok, libs, version, includepath, libpath)
def _get_bad():
    return (False, None, None, None, None)
def _get_info_variables(context, env, name, has_version):
    lcname= get_dependency_string(name)
    context.Message('Checking for '+name+' with variables...')
    if not env['IMP_OUTER_ENVIRONMENT'].get(lcname, None)\
            or env['IMP_OUTER_ENVIRONMENT'].get(lcname) != "yes":
        context.Result("no")
        return _get_bad()
    if env['IMP_OUTER_ENVIRONMENT'].get(lcname+"libs", None) is None:
        scons_tools.utility.report_error(env['IMP_OUTER_ENVIRONMENT'],
                                         "If configure specifies 'yes' for "+
                                         name+" it must also specify "+lcname+"libs"+
                                         env['IMP_OUTER_ENVIRONMENT'].get(lcname+"libs", "no found"))
        context.Result("no")
        return _get_bad()
    if has_version and not env['IMP_OUTER_ENVIRONMENT'].get(lcname+"version", None):
        scons_tools.utility.report_error(env['IMP_OUTER_ENVIRONMENT'], "If configure specifies 'yes' for "+
                                         name+" it must also specify "+lcname+"version")
        context.Result("no")
        return _get_bad()
    vers=None
    if has_version:
        vers= env['IMP_OUTER_ENVIRONMENT'].get(lcname+'version')
        if vers.find(" ") != -1:
            vers=vers.split()
        else:
            vers=[vers]
    context.Result("yes")
    return (True, env['IMP_OUTER_ENVIRONMENT'].get(lcname+"libs").split(":"),
            vers, None, None)
def _get_info_pkgconfig(context, env,  name, versioncpp, versionheader):
    if not context.env['IMP_OUTER_ENVIRONMENT']['IMP_HAS_PKG_CONFIG']:
        return _get_bad()
    lcname= get_dependency_string(name)
    context.Message('Checking for '+name+' with pkg-config...')
    ret = context.TryAction('pkg-config --exists \'%s\'' % lcname)[0]
    if not ret:
        context.Result("no")
        return _get_bad()
    context.Result("yes")
    (includepath, libpath, libs)= scons_tools.dependency.pkgconfig.get_config(context, lcname)
    if not versioncpp:
        version=None
    else:
        version= _get_version(context, name, includepath, versioncpp, versionheader)
    return (True, libs, version, includepath, libpath)

def _get_info_test(context, env, name, lib, headers, body,
                   extra_libs, versioncpp, versionheader,
                   search_build=False):
    lcname= get_dependency_string(name)
    #print context.env["LIBPATH"]
    #print context.env["CPPPATH"]

    (ret, libs, version)= check_lib(context, name, lib=lib, headers=headers,
                                    body=body,
                                    extra_libs=extra_libs,
                                    versioncpp=versioncpp,
                                    versionheader=versionheader,
                                    search_build=search_build)
    if not ret:
        return _get_bad()
    else:
        return (True, libs, version, None, None)

def _fix_boost(env, l):
    if not l.startswith("Boost"):
        return l
    lib= l[6:].lower()
    return scons_tools.dependency.boost.get_boost_lib_name(env, lib)

def add_external_library(env, name, lib, headers, body="", extra_libs=[],
                         versioncpp=None, versionheader=None,
                         enabled=True, build_script=None, alternate_lib=None):
    if env['IMP_PASS']!="CONFIGURE":
        return
    tenv= scons_tools.environment.get_test_environment(env)
    lcname= get_dependency_string(name)
    ucname= lcname.upper()
    dta= scons_tools.data.get(env)
    extra_libs=[_fix_boost(env, l) for l in extra_libs]

    variables=[lcname, lcname+"libs", lcname+"version"]
    def _check(context):
        local=False
        pythonpath=None
        if context.env['IMP_OUTER_ENVIRONMENT'][lcname] == "no":
            context.Message('Checking for '+name+' ...')
            context.Result("disabled")
            ok=False
        else:
            (ok, libs, version, includepath, libpath)\
                  = _get_info_variables(context, context.env, name, versioncpp)
            if not ok:
                (ok, libs, version, includepath, libpath)=\
                      _get_info_pkgconfig(context, env, name, versioncpp, versionheader)
                if not ok:
                    (ok, libs, version, includepath, libpath)=\
                      _get_info_test(context, env, name, lib, headers, body,
                                      extra_libs, versioncpp, versionheader)
                    if not ok and alternate_lib:
                        (ok, libs, version, includepath, libpath)=\
                        _get_info_test(context, env, name, alternate_lib, headers, body,
                                       extra_libs, versioncpp, versionheader)
                    if not ok and build_script:
                        local=True
                        paths={"builddir":Dir("#/build/").abspath,
                               "workdir":Dir("#/build/src/"+name).abspath,
                               "srcdir":scons_tools.paths.get_input_path(context.env, "dependency/"+name),
                               "libpath":env.get("libpath", ""),
                               "cxxflags":env.get("cxxflags", ""),
                               "linkflags":env.get("linkflags", ""),
                               "includepath":env.get("includepath", ""),
                               "cmake": env.get("cmake", ""),
                               "jobs":GetOption('num_jobs'),
                               "buildtype": env.get("build", "")}
                        build= open(scons_tools.paths.get_input_path(context.env, build_script), "r").read()
                        if not os.path.exists(paths["workdir"]):
                            os.makedirs(paths["workdir"])

                        buildscript= build%paths
                        scriptfile=File("#/build/src/install_%s"%name).abspath
                        open(scriptfile, "w").write(buildscript)
                        os.chmod(scriptfile, stat.S_IRWXU)
                        # not sure why this is printed 6 times...
                        #env.Execute(buildscript, lambda x,y,e: "Running "+build_script)
                        print "Executing", build_script
                        os.system(scriptfile)
                        (ok, libs, version, includepath, libpath)=\
                            _get_info_test(context, env, name, lib, headers, body,
                                           extra_libs, versioncpp, versionheader, True)
                                #print "found", ok

        if not ok:
            open(File("#/build/data/build_info/%s"%name).abspath, "w").write("ok=False\n")
            return False
        else:
            if not version:
                pversioncpp=None
                pversionheader=None
            else:
                pversioncpp=versioncpp
                pversionheader=versionheader
            config=["ok=True"]
            if libs:
                config.append("libraries=\"%s\""%":".join(libs))
            if pythonpath:
                config.append("pythonpath=\"%s\""%":".join(pythonpath))
            if includepath:
                config.append("includepath=\"%s\""%":".join(includepath))
            if libpath:
                config.append("libpath=\"%s\""%":".join(libpath))
            if version:
                config.append("version=\"%s\""%":".join(version))
            open(File("#/build/data/build_info/%s"%name).abspath, "w").write("\n".join(config))
            return True
    vars = env['IMP_VARIABLES']
    env['IMP_SCONS_EXTRA_VARIABLES'].append(lcname)
    if enabled:
        vars.Add(SCons.Variables.EnumVariable(lcname, 'Whether to use the '+name+' package', "auto", ["yes", "no", "auto"]))
    else:
        vars.Add(SCons.Variables.EnumVariable(lcname, 'Whether to use the '+name+' package', "no", ["yes", "no", "auto"]))
    if lib:
        vars.Add(lcname+'libs', 'Libs to link against when using '+name+'. Needed if "'+lcname+'" is "yes".', None)
    if versioncpp:
        vars.Add(lcname+'version', 'Version to test against when using '+name, None)
    vars.Update(env)
    if not env.GetOption('help'):
        custom_tests = {'CheckThisLib':_check}
        conf = tenv.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
        if conf.CheckThisLib():
            env.Append(IMP_ENABLED=[name])
        else:
            env.Append(IMP_DISABLED=[name])
            env.Append(IMP_CONFIGURATION=[lcname+"='no'"])
        conf.Finish()
