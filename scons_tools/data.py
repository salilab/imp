class IMPData:
    class ModuleData:
        def __init__(self, name, dependencies=[], unfound_dependencies=[], modules=[],
                     python_modules=[], version=""):
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
    class ApplicationData:
        def __init__(self, name, dependencies=[], unfound_dependencies=[], modules=[],
                     python_modules=[], version=""):
            self.dependencies=dependencies
            self.unfound_dependencies=unfound_dependencies
            self.modules=modules
            self.python_modules=python_modules
            self.version=version
    class SystemData:
        def __init__(self, name, dependencies=[], unfound_dependencies=[], modules=[],
                     python_modules=[]):
            self.dependencies=dependencies
            self.unfound_dependencies=unfound_dependencies
            self.modules=modules
            self.python_modules=python_modules
    def __init__(self):
        self.modules={}
        self.applications={}
        self.systems={}


def get(env):
    return env["IMP_DATA"]

def add(env):
    env["IMP_DATA"] = IMPData()
