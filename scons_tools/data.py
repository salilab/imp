import utility
from SCons.Script import File, Dir
import os
import os.path


def _check_names(names):
    for n in names:
        if n=="":
            raise ValueError("Invalid empty name")


class IMPData:
    class ModuleData:
        def __init__(self, name, dependencies=[], direct_dependencies=[],
                     unfound_dependencies=[], modules=[],
                     unfound_modules=[],
                     version=None, external=False,
                     ok=True):
            if ok:
                #print "configuring", name, dependencies, direct_dependencies, unfound_dependencies, unfound_modules
                self.dependencies=dependencies
                self.direct_dependencies=direct_dependencies
                self.unfound_dependencies=unfound_dependencies
                self.modules=modules
                self.unfound_modules=unfound_modules
                self.data=False
                self.name=name
                if name=="kernel":
                    self.path=""
                    self.nicename="IMP"
                else:
                    self.path=name
                    self.nicename="IMP."+name
                self.version=version
            self.ok=ok
            self.external=external

    class ApplicationData:
        def __init__(self, name, link="",
                     dependencies=[], unfound_dependencies=[], modules=[],
                     version="", ok=True):
            self.dependencies=dependencies
            self.unfound_dependencies=unfound_dependencies
            self.modules=modules
            self.link=link
            self.version=version
            self.ok=ok
    class ExampleData:
        def __init__(self, name, overview):
            self.file=name
            self.name=utility.get_display_from_name(name)
            self.link= utility.get_link_from_name(name)
            self.overview= overview
    class SystemData:
        def __init__(self, name, link="",
                     dependencies=[], unfound_dependencies=[], modules=[],
                     version="", ok=True):
            self.dependencies=dependencies
            self.link=link
            self.unfound_dependencies=unfound_dependencies
            self.modules=modules
            self.version=version
            self.ok=ok
    def __init__(self, env):
        self.modules={}
        self.applications={}
        self.systems={}
        self.examples={}
        self.env=env
    def _expand_modules(self, imodules, external=False):
        modules=imodules[:]
        ml=[]
        for m in modules:
            if external and m not in self.modules.keys():
                self._check_module(m)
            if self.modules[m].ok:
                ml+=[m]+ self.modules[m].modules
        mret=[]
        for i,m in enumerate(ml):
            if not m in ml[i+1:]:
                mret.append(m)
        return mret
    def _expand_dependencies(self, modules, idependencies):
        dependencies=idependencies[:]
        dret=[]
        for m in modules:
            dependencies+= self.modules[m].dependencies
        for i,m in enumerate(dependencies):
            if not m in dependencies[i+1:]:
                dret.append(m)
        return dret
    def add_application(self, name, link="",
                        dependencies=[], unfound_dependencies=[], modules=[],
                        version=None, ok=True):
        if ok and not version:
            utility.report_error("Version must be specified for modules")
        if not ok:
            self.applications[name]=self.ApplicationData(name, ok=ok)
        else:
            passmodules= self._expand_modules(modules)
            passdependencies= self._expand_dependencies(passmodules,
                                                        dependencies)
            self.applications[name]\
                =self.ApplicationData(name, link=link,
                                      dependencies=passdependencies,
                                      unfound_dependencies=unfound_dependencies,
                                      modules=passmodules,
                                      version=version)
    def add_module(self, name, directory="",
                   dependencies=[], unfound_dependencies=[], modules=[],
                   unfound_modules=[],
                   version="", ok=True,
                   external=False):
        #print name, dependencies, unfound_dependencies
        _check_names(dependencies)
        _check_names(unfound_dependencies)
        _check_names(modules)
        if not ok:
            self.modules[name]=self.ModuleData(name, ok=False)
        else:
            # prevent recurrence
            self.modules[name]=self.ModuleData(name, ok=True)
            passmodules= self._expand_modules(modules, external)
            passdependencies= self._expand_dependencies(passmodules,
                                                        dependencies)
            self.modules[name]=self.ModuleData(name, passdependencies, dependencies,
                                               unfound_dependencies,
                                               passmodules, unfound_modules,
                                               version, external)
    def add_system(self, name, link="",
                   dependencies=[], unfound_dependencies=[], modules=[],
                        version="", ok=True):
        if not ok:
            self.systems[name]=self.SystemData(name, ok=ok)
        else:
            passmodules= self._expand_modules(modules)
            passdependencies= self._expand_dependencies(passmodules,
                                                        dependencies)
            self.systems[name]\
                =self.SystemData(name, link=link,
                                 dependencies=passdependencies,
                                 unfound_dependencies=unfound_dependencies,
                                 modules=passmodules,
                                 version=version)
    def add_example(self, name, overview):
        self.examples[utility.get_link_name_from_name(name)]=self.ExampleData(name, overview)
    def _check_module(self, m):
        ok=False
        external=False
        version="no version"
        libname=None
        modules=[]
        if m =='kernel':
            libname="imp"
            headername="IMP/kernel_config.h"
            namespace="IMP"
        else:
            libname="imp_"+m
            headername="IMP/"+m+"/"+m+"_config.h"
            namespace="IMP::"+m+""
        conf = self.env.Configure()
        if True:
            if m=="kernel":
                ln= "imp"
            else:
                ln= "imp_"+m
            if True:
                print "Checking for module "+m+"...",
                olibs= conf.env.get("LIBS", [])
                conf.env.Append(LIBS=ln)
                ret = conf.TryRun("""#include <%(headername)s>
#include <iostream>

        int main()
        {
            std::cout << %(namespace)s::get_module_version() <<std::endl;
            std::cout << %(namespace)s::get_module_modules() << std::endl;
            return 0;
        }
"""%{"namespace":namespace, "headername":headername}, '.cpp')
                conf.env["LIBS"]=olibs
                # remove trailing newline
                #print ret
                if ret[0]:
                    version, deps, junk= ret[1].split("\n")
                    if not self.env['versionchecks']:
                        version="not checked"
                    ok=True
                    external=True
                    version=version
                    libname=ln
                    modules=deps.split(":")
                    # silly insistence on invariants
                    if len(modules)==1 and modules[0]=="":
                        modules=[]
                    print version
                else:
                    print "no"
        conf.Finish()
        # put it add end so it can check for more modules
        self.add_module(m, ok=ok, external=external, version=version,
                        modules=modules)
        if external and not self.modules.has_key("test"):
            self._check_module("test")
    def get_found_modules(self, modules):
        ret=[]
        for m in modules:
            if not self.modules.has_key(m):
                self._check_module(m)
            if self.modules[m].ok:
                ret.append(m)
        return ret
    def add_to_alias(self, name, nodes):
        #print "add alias", name
        self.env.Alias(name, nodes)
    def get_alias(self, name):
        #print "get alias", name
        return self.env.Alias(name)

def get(env):
    return env["IMP_DATA"]

def add(env, data= None):
    if not data:
        data=IMPData(env)
    env["IMP_DATA"] = data


def add_module(self, name, directory="", alias="none",
               dependencies=[], unfound_dependencies=[], modules=[],
               unfound_modules=[], libname=None,
               version="", ok=True,
               external=False):
               #print name, dependencies, unfound_dependencies
    _check_names(dependencies)
    _check_names(unfound_dependencies)
    _check_names(modules)
    if not os.path.exists(Dir("#/build/modules").abspath):
        os.mkdir(Dir("#/build/modules").abspath)
    fl= open(File("#/build/modules/"+name).abspath, "w")

    data={"ok":ok}
    if ok:
        # prevent recurrence
        passmodules= self._expand_modules(modules, external)
        passdependencies= self._expand_dependencies(passmodules,
                                                    dependencies)
        data["dependencies"]=passdependencies
        data["unfound_dependencies"]= unfound_dependencies
        data["modules"]= passmodules
        data["libname"]=libname
        data["version"]=version
        if external:
            data["external"]=True

_dependencies={}

def add_dependency(name, libs=[], ok=True, variables=[],
                   includepath=None, libpath=None, pythonpath=None, version=None,
                   versioncpp="", versionheader="", local=False,
                   build_script=""):
    if type(libs) != type([]):
        utility.report_error(self.env, "lib lists must be stored as a list: "+name)
    if not os.path.exists(Dir("#/build/dependencies").abspath):
        os.makedirs(Dir("#/build/dependencies").abspath)
    fl= open(File("#/build/dependencies/"+name).abspath, "w")
    data={"ok":ok}
    if ok:
        if len(libs)>0:
            data["libs"]=libs
        if len(variables) >0:
            data["variables"]=variables
        if includepath:
            data["includepath"]=includepath
        if libpath:
            data["libpath"]=libpath
        if pythonpath:
            data["pythonpath"]=pythonpath
        if version:
            data["version"]=version
            data["versioncpp"]=versioncpp
            data["versionheader"]=versionheader
        if local:
            data["local"]=True
            data["build_script"]=build_script
    print >> fl, data
    _dependencies[name]=data

def get_dependency(name):
    return _dependencies[name]

def get_dependency_variable(varname):
    ret=[]
    for k in _dependencies.keys():
        if _dependencies[k].has_key(varname):
            ret.extend(_dependencies[k][varname])
    return ret

def get_has_configured_dependency(name):
    return _dependencies.has_key(name)

def get_found_dependencies(dependencies):
    ret=[]
    for d in dependencies:
        if get_dependency(d)["ok"]:
            ret.append(d)
    return ret
