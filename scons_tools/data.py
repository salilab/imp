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
        def __init__(self, name, dependencies=[], unfound_dependencies=[], modules=[],
                     python_modules=[], version="", ok=True):
            self.dependencies=dependencies
            self.unfound_dependencies=unfound_dependencies
            self.modules=modules
            self.python_modules=python_modules
            self.version=version
            self.ok=ok
    class ExampleData:
        def __init__(self, link, classes, methods):
            self.link=link
            self.classes=classes
            self.methods=methods
    class SystemData:
        def __init__(self, name, dependencies=[], unfound_dependencies=[], modules=[],
                     python_modules=[], version="", ok=True):
            self.dependencies=dependencies
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
    def add_application(self, name, dependencies=[], unfound_dependencies=[], modules=[],
                        python_modules=[], version="", ok=True):
        if not ok:
            self.applications[name]=self.ApplicationData(name, ok=ok)
        else:
            passmodules= self._expand_modules(modules)
            passpythonmodules= self._expand_modules(python_modules)
            passdependencies= self._expand_dependencies(passpythonmodules,
                                                        dependencies)
            self.applications[name]\
                =self.ApplicationData(name, dependencies=passdependencies,
                                      unfound_dependencies=unfound_dependencies,
                                      modules=passmodules,
                                      python_modules=passpythonmodules,
                                      version=version)
    def add_system(self, name, dependencies=[], unfound_dependencies=[], modules=[],
                        python_modules=[], version="", ok=True):
        if not ok:
            self.systems[name]=self.ApplicationData(name, ok=ok)
        else:
            passmodules= self._expand_modules(modules)
            passpythonmodules= self._expand_modules(python_modules)
            passdependencies= self._expand_dependencies(passpythonmodules,
                                                        dependencies)
            self.systems[name]\
                =self.SystemData(name, dependencies=passdependencies,
                                 unfound_dependencies=unfound_dependencies,
                                 modules=passmodules,
                                 python_modules=passpythonmodules,
                                 version=version)
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
