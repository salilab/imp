import scons_tools.utility
import scons_tools.data
import SCons
import os
from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner

def _search_for_deps(context, libname, extra_libs, headers, body, possible_deps):
    for i in range(0,len(possible_deps)+1):
        lc= extra_libs+possible_deps[0:i]
        #print "Trying "+ str(i) +" with " +str(lc)
        olibs= context.env.get('LIBS', [])
        context.env.Append(LIBS=lc)
        ret=context.sconf.CheckLibWithHeader(libname, header=headers, call=body, language='CXX',
                                             autoadd=False)
        context.env.Replace(LIBS=olibs)
        if ret:
            return (True, [libname]+lc, None)
    return (False, None, None)

def add_dependency_link_flags(env, dependencies):
    for d in dependencies:
        env.Append(LIBS=scons_tools.data.get(env).dependencies[d].libs)

def _get_version(context, name, includepath, versioncpp, versionheader):
    if versioncpp:
        context.Message('Checking for version of '+name+"...")
        if type(versioncpp) == type([]):
            vs="<< ' ' << ".join(versioncpp)
        else:
            vs=versioncpp
        if includepath:
            oldcpp= context.env.get('CPPPATH', None)
            context.env.Append(CPPPATH=includepath)
        r = context.TryRun("#include <"+versionheader+">\n"+\
                             """#include <iostream>

        int main()
        {
            std::cout << """+vs +"""<<std::endl;
            return 0;
        }
        """, '.cpp')
        if includepath:
            context.env.Replace(CPPPATH=oldcpp)
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

def check_lib(context, name, lib, header, body="", extra_libs=[], versioncpp=None,
              versionheader=None):
    oldflags= context.env.get('LINKFLAGS')
    context.env.Replace(LINKFLAGS=context.env['IMP_BIN_LINKFLAGS'])
    if type(lib) == list:
        ret=_search_for_deps(context, lib[0], lib[1:], header, body, extra_libs)
    elif lib==None:
        ret=(context.sconf.CheckHeader(header, language="C++"), [])
    else:
        ret=_search_for_deps(context, lib, [], header, body, extra_libs)
    if not ret[0]:
        context.env.Replace(LINKFLAGS=oldflags)
        return ret
    if context.env['IMP_BUILD_STATIC'] and lib != None:
        scons_tools.utility.make_static_build(context.env)
        if type(lib) == list:
            bret=_search_for_deps(context, lib[0], lib[1:], header, body, extra_libs)
        else:
            bret=_search_for_deps(context, lib, [], header, body, extra_libs)
        scons_tools.utility.unmake_static_build(context.env)
        # should be the sum of the two
        if bret[0]:
            context.env.Replace(LINKFLAGS=oldflags)
            return (bret[0], ret[1]+bret[1], _get_version(context, name, None,
                                                          versioncpp,
                                                          versionheader))
        else:
            context.env.Replace(LINKFLAGS=oldflags)
            return (False, [], None)
    vers= _get_version(context, name, None, versioncpp, versionheader)
    #print "version", vers
    context.env.Replace(LINKFLAGS=oldflags)
    return  (True, ret[1], vers)

def get_dependency_string(name):
    lname= name.lower()
    nname=lname.replace(".", "_")
    return nname

# return (ok, libs, version, includepath, libpath)
def _get_bad():
    return (False, None, None, None, None)
def _get_info_variables(context, env, name, has_version):
    lcname= get_dependency_string(name)
    if not env.get(lcname, None) or env.get(lcname) != "yes":
        return _get_bad()
    if not env.get(lcname+"libs", None):
        scons_tools.utility.report_error(env, "If configure specifices 'yes' for "+
                                         name+" it must also specify "+lcname+"libs")
        return _get_bad()
    if has_version and not env.get(lcname+"version", None):
        scons_tools.utility.report_error(env, "If configure specifices 'yes' for "+
                                         name+" it must also specify "+lcname+"version")
        return _get_bad()
    vers=None
    if has_version:
        vers= env.get(lcname+'version')
        if vers.find(" ") != -1:
            vers=vers.split()
    return (True, env.get(lcname+"libs"),
            vers, None, None)
def _get_info_pkgconfig(context, env,  name, versioncpp, versionheader):
    if not context.env['IMP_HAS_PKG_CONFIG']:
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

def _get_info_test(context, env, name, lib, header, body,
                   extra_libs, versioncpp, versionheader):
    lcname= get_dependency_string(name)
    (ret, libs, version)= check_lib(context, name, lib=lib, header=header,
                                    body=body,
                                    extra_libs=extra_libs,
                                    versioncpp=versioncpp,
                                    versionheader=versionheader)
    if not ret:
        return _get_bad()
    else:
        return (True, libs, version, None, None)

def add_external_library(env, name, lib, header, body="", extra_libs=[],
                         versioncpp=None, versionheader=None):
    lcname= get_dependency_string(name)
    ucname= lcname.upper()
    if scons_tools.data.get(env).dependencies.has_key(name):
        # already has been added
        return
    variables=[lcname, lcname+"libs", lcname+"version"]
    def _check(context):
        if context.env[lcname] == "no":
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
                      _get_info_test(context, env, name, lib, header, body,
                                      extra_libs, versioncpp, versionheader)
            if not ok:
                scons_tools.data.get(context.env).add_dependency(name, variables=variables,
                                                                 ok=False)
                return False
            else:
                if not version:
                    pversioncpp=None
                    pversionheader=None
                else:
                    pversioncpp=versioncpp
                    pversionheader=versionheader
                scons_tools.data.get(context.env).add_dependency(name,
                                                                 variables=variables,
                                                                 libs=libs,
                                                                 includepath=includepath,
                                                                 libpath=libpath,
                                                                 version=version,
                                                                 versioncpp=pversioncpp,
                                                             versionheader=pversionheader)
                return True
    vars = env['IMP_VARIABLES']
    vars.Add(SCons.Variables.EnumVariable(lcname, 'Whether to use the '+name+' package', "auto", ["yes", "no", "auto"]))
    vars.Add(lcname+'libs', 'Libs to link against when using '+name+'. Needed if "'+lcname+'" is "yes".', None)
    vars.Add(lcname+'version', 'Version to test against when using '+name, None)
    vars.Update(env)
    if not env.GetOption('help'):
        custom_tests = {'CheckThisLib':_check}
        conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
        if conf.CheckThisLib():
            env.Append(IMP_ENABLED=[name])
            env.Append(IMP_CONFIGURATION=[lcname+"='yes'"])
            env.Append(IMP_CONFIGURATION=[lcname+"libs='"+\
                                          ":".join(scons_tools.data.get(env).dependencies[name].libs)+"'"])
            if scons_tools.data.get(env).dependencies[name].includepath:
                env.Append(IMP_CONFIGURATION=[lcname\
                                      +"includepath='"+\
                                      scons_tools.data.get(env).dependencies[name].includepath+"'"])
            if scons_tools.data.get(env).dependencies[name].libpath:
                env.Append(IMP_CONFIGURATION=[lcname\
                                              +"libpath='"+\
                                            scons_tools.data.get(env).dependencies[name].libpath+"'"])
        else:
            env.Append(IMP_DISABLED=[name])
            env.Append(IMP_CONFIGURATION=[lcname+"='no'"])
        conf.Finish()
