import utility

class IMPData:
    class ModuleData:
        def __init__(self, name, alias="none", dependencies=[], direct_dependencies=[],
                     unfound_dependencies=[], modules=[],
                     unfound_modules=[], libname=None,
                     version=None, external=False,
                     ok=True):
            if ok:
                #print "configuring", name, dependencies, direct_dependencies, unfound_dependencies, unfound_modules
                self.alias=alias
                self.dependencies=dependencies
                self.direct_dependencies=direct_dependencies
                self.unfound_dependencies=unfound_dependencies
                self.modules=modules
                self.unfound_modules=unfound_modules
                self.libname=libname
                self.data=False
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
    class DependencyData:
        def __init__(self, name, ok=True, variables=[], libs=[], libpath=None,
                     includepath=None, pkgconfig=False, version=None,
                     versioncpp=None, versionheader=None):
            self.ok=ok
            self.libs=libs
            self.includepath=includepath
            self.libpath=libpath
            self.variables=variables
            self.pkgconfig=pkgconfig
            self.version=version
            self.versioncpp=versioncpp
            self.versionheader=versionheader
    def __init__(self, env):
        self.modules={}
        self.applications={}
        self.systems={}
        self.dependencies={}
        self.examples={}
        self.env=env
    def _check_names(self, names):
        for n in names:
            if n=="":
                raise ValueError("Invalid empty name")
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
    def add_module(self, name, directory="", alias="none",
                   dependencies=[], unfound_dependencies=[], modules=[],
                   unfound_modules=[], libname=None,
                   version="", ok=True,
                   external=False):
        #print name, dependencies, unfound_dependencies
        self._check_names(dependencies)
        self._check_names(unfound_dependencies)
        self._check_names(modules)
        if not ok:
            self.modules[name]=self.ModuleData(name, ok=False)
        else:
            # prevent recurrence
            self.modules[name]=self.ModuleData(name, ok=True)
            passmodules= self._expand_modules(modules, external)
            passdependencies= self._expand_dependencies(passmodules,
                                                        dependencies)
            self.modules[name]=self.ModuleData(name, alias, passdependencies, dependencies,
                                               unfound_dependencies,
                                               passmodules, unfound_modules,
                                               libname,
                                               version, external)
    def add_dependency(self, name, libs=[], ok=True, variables=[], pkgconfig=False,
                       includepath=None, libpath=None, version=None,
                       versioncpp=None, versionheader=None):
        if type(libs) != type([]):
            utility.report_error(self.env, "lib lists must be stored as a list: "+name)

        self.dependencies[name]=self.DependencyData(name, libs=libs, variables=variables,
                                                    ok=ok, pkgconfig=pkgconfig,
                                                    includepath=includepath,
                                                    libpath=libpath,
                                                    version=version,
                                                    versioncpp=versioncpp,
                                                    versionheader=versionheader)
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
        elif m =='RMF':
            libname="RMF"
            headername="RMF.h"
            namespace="RMF"
        else:
            libname="imp_"+m
            headername="IMP/"+m+"/"+m+"_config.h"
            namespace="IMP::"+m+""
        conf = self.env.Configure()
        if True:
            if m=="kernel":
                ln= "imp"
            elif m =='RMF':
                ln="RMF"
            else:
                ln= "imp_"+m
            if True:
                print "Checking for module "+m+"...",
                olibs= conf.env["LIBS"]
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
                        libname=libname, modules=modules)
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
    def get_found_dependencies(self, dependencies):
        ret=[]
        for d in dependencies:
            if self.dependencies[d].ok:
                ret.append(d)
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
