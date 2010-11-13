import utility

excluded_classes={"kernel":["Model", "Particle", "Particles",
                            "SILENT", "TERSE", "VERBOSE", "NONE",
                            "FloatKey", "IntKey", "ParticleKey", "StringKey"],
                  "core":["XYZs", "XYZsTemp", "XYZRs", "XYZRsTemp"],
                  "atom":["CHAIN_TYPE", "ATOM_TYPE", "RESIDUE_TYPE"]}
included_methods={"kernel":{},
                  "atom":{"read_pdb":"(TextInput, Model*)",
                          "create_protein":\
                          "(Model*,std::string,double,int,int,double)",
                          "create_simplified_along_backbone":"(Chain,int)",
                          "create_distance_restraint":\
                          "(const Selection &,const Selection &,double, double)",
                          "create_connectivity_restraint":"(const Selections &,double)",
                          "create_excluded_volume_restraint":"(const Hierarchies &,double)",
                          "setup_as_approximation":"(Hierarchy)"
                          }
                  }


class IMPData:
    class ModuleData:
        def __init__(self, name, dependencies=[], unfound_dependencies=[], modules=[],
                     python_modules=[], version="", ok=True):
            if ok:
                self.dependencies=dependencies
                self.unfound_dependencies=unfound_dependencies
                self.modules=modules
                self.python_modules=python_modules
                if name=="kernel":
                    self.path=""
                    self.nicename="IMP"
                else:
                    self.path=name
                    self.nicename="IMP."+name
                self.version=version
            self.ok=ok

    class ApplicationData:
        def __init__(self, name, link="",
                     dependencies=[], unfound_dependencies=[], modules=[],
                     python_modules=[], version="", ok=True):
            self.dependencies=dependencies
            self.unfound_dependencies=unfound_dependencies
            self.modules=modules
            self.link=link
            self.python_modules=python_modules
            self.version=version
            self.ok=ok
    class ExampleData:
        def __init__(self, name):
            self.processed=False
            self.file=name
            self.name=utility.get_display_from_name(name)
            self.link= utility.get_link_from_name(name)
            self.overview= utility.get_without_extension(name)+".readme"
            #print "for", name, "got", self.name, "and", self.overview
        def set_processed(self, classes, methods):
            self.processed=True
            fclasses={}
            for k in classes.keys():
                if len(classes[k])==0:
                    continue
                fclasses[k]=[]
                for i,c in enumerate(classes[k]):
                    if c in classes[k][i+1:]:
                        continue
                    if not k in excluded_classes.keys() or c not in excluded_classes[k]:
                        fclasses[k].append(c)
            fmethods={}
            for k in methods.keys():
                if len(methods[k])==0 or k not in included_methods.keys():
                    continue
                fmethods[k]=[]
                for i,m in enumerate(methods[k]):
                    if m in methods[k][i+1:]:
                        continue
                    if m in included_methods[k].keys():
                        fmethods[k].append(m+included_methods[k][m])
            self.classes=fclasses
            self.methods=fmethods
    class SystemData:
        def __init__(self, name, link="",
                     dependencies=[], unfound_dependencies=[], modules=[],
                     python_modules=[], version="", ok=True):
            self.dependencies=dependencies
            self.link=link
            self.unfound_dependencies=unfound_dependencies
            self.modules=modules
            self.python_modules=python_modules
            self.version=version
            self.ok=ok
    class DependencyData:
        def __init__(self, name, ok=True, libs=[]):
            self.ok=ok
            self.libs=libs
    def __init__(self):
        self.modules={}
        self.applications={}
        self.systems={}
        self.dependencies={}
        self.examples={}
    def _expand_modules(self, modules):
        ml=[]
        for m in modules:
            ml+=[m]+ self.modules[m].modules
        mret=[]
        for i,m in enumerate(ml):
            if not m in ml[i+1:]:
                mret.append(m)
        return mret
    def _expand_dependencies(self, modules, dependencies):
        dret=[]
        for m in modules:
            dependencies+= self.modules[m].dependencies
        for i,m in enumerate(dependencies):
            if not m in dependencies[i+1:]:
                dret.append(m)
        return dret
    def add_module(self, name, directory="",
                   dependencies=[], unfound_dependencies=[], modules=[],
                   python_modules=[], version="", ok=True):
        if not ok:
            self.modules[name]=self.ModuleData(name, ok=ok)
        else:
            passmodules= self._expand_modules(modules)
            passpythonmodules= self._expand_modules(python_modules)
            passdependencies= self._expand_dependencies(passpythonmodules,
                                                        dependencies)
            self.modules[name]=self.ModuleData(name, passdependencies, unfound_dependencies,
                                          passmodules, passpythonmodules, version)
    def add_dependency(self, name, libs=[], ok=True):
        self.dependencies[name]=self.DependencyData(name, libs=libs, ok=ok)
    def add_application(self, name, link="",
                        dependencies=[], unfound_dependencies=[], modules=[],
                        python_modules=[], version="", ok=True):
        if not ok:
            self.applications[name]=self.ApplicationData(name, ok=ok)
        else:
            passmodules= self._expand_modules(modules)
            passpythonmodules= self._expand_modules(python_modules)
            passdependencies= self._expand_dependencies(passpythonmodules,
                                                        dependencies)
            self.applications[name]\
                =self.ApplicationData(name, link=link,
                                      dependencies=passdependencies,
                                      unfound_dependencies=unfound_dependencies,
                                      modules=passmodules,
                                      python_modules=passpythonmodules,
                                      version=version)
    def add_system(self, name, link="",
                   dependencies=[], unfound_dependencies=[], modules=[],
                        python_modules=[], version="", ok=True):
        if not ok:
            self.systems[name]=self.ApplicationData(name, ok=ok)
        else:
            passmodules= self._expand_modules(modules)
            passpythonmodules= self._expand_modules(python_modules)
            passdependencies= self._expand_dependencies(passpythonmodules,
                                                        dependencies)
            self.systems[name]\
                =self.SystemData(name, link=link,
                                 dependencies=passdependencies,
                                 unfound_dependencies=unfound_dependencies,
                                 modules=passmodules,
                                 python_modules=passpythonmodules,
                                 version=version)
    def add_example(self, name):
        self.examples[utility.get_link_name_from_name(name)]=self.ExampleData(name)
    def get_found_modules(self, modules):
        ret=[]
        for m in modules:
            if self.modules[m].ok:
                ret.append(m)
        return ret
    def get_found_dependencies(self, dependencies):
        ret=[]
        for d in dependencies:
            if self.dependencies[d].ok:
                ret.append(d)
        return ret

def get(env):
    return env["IMP_DATA"]

def add(env, data= IMPData()):
    env["IMP_DATA"] = data
