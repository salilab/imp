import utility

excluded_classes={"kernel":["Model", "Particle", "Particles",
                            "SILENT", "TERSE", "VERBOSE", "NONE",
                            "FloatKey", "IntKey", "ParticleKey", "StringKey"],
                  "core":["XYZs", "XYZsTemp", "XYZRs", "XYZRsTemp", "GenericHierarchies",
                          "GenericHierarchiesTemp"],
                  "atom":["CHAIN_TYPE", "ATOM_TYPE", "RESIDUE_TYPE", "Hierarchies"]}
typedef_classes={"algebra":{"Vector3D":"VectorD",
                            "BoundingBox3D":"BoundingBoxD",
                            "Sphere3D":"SphereD",
                            "NearestNeighbor3D":"NearestNeighborD"}}
included_methods={"kernel":{"set_check_level":("void", "(CheckLevel)"),
                            "set_log_level":("void", "(LogLevel)"),
                            "add_failure_handler":("void", "(FailureHandler*)"),
                            "get_dependency_graph":("DependencyGrah", "(const RestraintsTemp&)"),
                            "get_pruned_dependency_graph":("DependencyGraph", "(const RestraintsTemp&)"),},
                  "atom":{"read_pdb":("Hierarchy", "(TextInput, Model*)"),
                          "create_protein":\
                          ("Hierarchy", "(Model*,std::string,double,int,int,double)"),
                          "create_simplified_along_backbone":("Hierarchy", "(Chain,int)"),
                          "get_leaves":("Hierarchies", "(Hierarchy)"),
                          "create_distance_restraint":\
                          ("Restraint", "(const Selection &,const Selection &,double, double)"),
                          "create_connectivity_restraint":("Restraint", "(const Selections &,double)"),
                          "create_excluded_volume_restraint":("Restraint", "(const Hierarchies &,double)"),
                          "create_cover":("core::Cover", "(Selection,std::string)"),
                          "setup_as_approximation":("void", "(Hierarchy)"),
                          "create_clone":("Hierarchy", "(Hierarchy)"),
                          "destroy":("void", "(Hierarchy)")
                          },
                  "display":{"get_display_color":("Color", "(unsigned int)"),
                             "get_jet_color":("Color", "(double)")},
                  "statistics":{"create_lloyds_kmeans":\
                                ("PartitionalClusteringWithCenter", "(Embedding*,unsigned int, unsigned int)"),
                                "get_connectivity_clustering":\
                                ("PartitionalClustering", "(Embedding*,double)")},
                  "algebra":{"get_random_vector_in":("Vector3D", "(const SphereD<D>&)"),
                             "get_random_vector_on":("Vector3D", "(const SphereD<D>&)")},
                  "rmf":{"add_hierarchy":("void", "(RMF::RootHandle, atom::Hierarchy)"),
                         "save_frame":("void", "(RMF::RootHandle, unsigned int, atom::Hierarchy)"),
                         "load_frame":("void", "(RMF::RootHandle, unsigned int, atom::Hierarchy)"),
                         "create_hierarchies":("atom::Hierarchies", "(RMF::RootHandle, Model*)"),
                         "set_hierarchies":("void", "(RMF::RootHandle, atom::Hierarchies, bool)"),
                         "add_geometry":("void", "(RMF::RootHandle, display::Geometry*)"),
                         "save_frame":("void", "(RMF::RootHandle, int, display::Geometry*)"),
                         "create_geometries":("display::Geometries", "(RMF::RootHandle, int)"),
                         "add_particle":("void", "(RMF::RootHandle, Particle*)"),
                         "create_particles":("ParticlesTemp", "(RMF::RootHandle, Model*)"),
                          },
                  "RMF":{"create_hdf5_file":("RMF::HDF5Group", "(std::string)"),
                         "open_hdf5_file":("RMF::HDF5Group", "(std::string)"),
                          },

                  "domino":{"set_assignments":("void", "(AssignmentContainer *, const Subset &, const ParticlesTemp, RMF::HDF5IndexDataSet2D)"),
                            "create_assignments_container":("AssignmentContainer*", "(RMF::HDF5IndexDataSet2D,const Subset &,const ParticlesTemp&)"),
                            "get_junction_tree":("SubsetGraph", "(const InteractionGraph&)"),
                            "get_merge_tree":("SubsetGraph", "(const SubsetGraph&)")
                      },
                  }



class IMPData:
    class ModuleData:
        def __init__(self, name, dependencies=[], direct_dependencies=[],
                     unfound_dependencies=[], modules=[],
                     unfound_modules=[], libname=None,
                     python_modules=[], version=None, external=False,
                     python=True, data=True, ok=True):
            self.build=[]
            if ok:
                #print "configuring", name, dependencies, direct_dependencies, unfound_dependencies, unfound_modules
                self.dependencies=dependencies
                self.direct_dependencies=direct_dependencies
                self.unfound_dependencies=unfound_dependencies
                self.modules=modules
                self.unfound_modules=unfound_modules
                self.python_modules=python_modules
                self.libname=libname
                if name=="kernel":
                    self.path=""
                    self.nicename="IMP"
                else:
                    self.path=name
                    self.nicename="IMP."+name
                self.version=version
                self.data=data
                self.python=python
            self.ok=ok
            self.external=external

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
        def __init__(self, name, overview):
            self.processed=False
            self.file=name
            self.name=utility.get_display_from_name(name)
            self.link= utility.get_link_from_name(name)
            self.overview= overview
            #print "for", name, "got", self.name, "and", self.overview
        def set_processed(self, classes, methods):
            self.processed=True
            fclasses={}
            #print "processed", methods.keys()
            for k in classes.keys():
                if len(classes[k])==0:
                    continue
                fclasses[k]=[]
                for i,c in enumerate(classes[k]):
                    if c in classes[k][i+1:]:
                        continue
                    if typedef_classes.has_key(k) and\
                       typedef_classes[k].has_key(c):
                        c= typedef_classes[k][c]
                    if not excluded_classes.has_key(k) or c not in excluded_classes[k]:
                        fclasses[k].append(c)
            fmethods={}
            for k in methods.keys():
                if len(methods[k])==0 or not included_methods.has_key(k):
                    continue
                fmethods[k]=[]
                for i,m in enumerate(methods[k]):
                    if m in methods[k][i+1:]:
                        continue
                    if included_methods[k].has_key(m):
                        fmethods[k].append(m+included_methods[k][m][1])
            #print "for", self.file, "got", fclasses, fmethods
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
    def _expand_modules(self, imodules):
        modules=imodules[:]
        ml=[]
        for m in modules:
            ml+=[m]+ self.modules[m].modules
        mret=[]
        for i,m in enumerate(ml):
            if not m in ml[i+1:]:
                mret.append(m)
        return mret
    def _expand_python_modules(self, imodules):
        modules=imodules[:]
        ml=[]
        for m in modules:
            ml+=[m]+ self.modules[m].python_modules
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
    def add_module(self, name, directory="",
                   dependencies=[], unfound_dependencies=[], modules=[],
                   unfound_modules=[], libname=None,
                   python_modules=[], version="", ok=True,
                   external=False, python=True, data=True):
        #print name, dependencies, unfound_dependencies
        self._check_names(dependencies)
        self._check_names(unfound_dependencies)
        self._check_names(modules)
        self._check_names(python_modules)
        if not ok:
            self.modules[name]=self.ModuleData(name, ok=ok)
        else:
            passmodules= self._expand_modules(modules)
            passpythonmodules= self._expand_python_modules(python_modules)
            passdependencies= self._expand_dependencies(passpythonmodules,
                                                        dependencies)
            self.modules[name]=self.ModuleData(name, passdependencies, dependencies,
                                               unfound_dependencies,
                                               passmodules, unfound_modules,
                                               libname,
                                               passpythonmodules, version, external,
                                               python, data)
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
                        python_modules=[], version=None, ok=True):
        if ok and not version:
            utility.report_error("Version must be specified for modules")
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
            self.systems[name]=self.SystemData(name, ok=ok)
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
    def add_example(self, name, overview):
        self.examples[utility.get_link_name_from_name(name)]=self.ExampleData(name, overview)
    def _check_module(self, m):
        if m =='kernel':
            libname="imp"
            headername="IMP/kernel_config.h"
            namespace="IMP"
            modname="IMP"
        else:
            libname="imp_"+m
            headername="IMP/"+m+"/"+m+"_config.h"
            namespace="IMP::"+m+""
            modname="IMP."+m
        conf = self.env.Configure()
        ret=conf.CheckLibWithHeader(libname, header=[headername],
                                    call=namespace+"::get_module_version();",
                                    language="CXX", autoadd=False)
        # get version number
        if ret:
            print "Checking for version info in "+modname+"...",
            version= utility.get_python_result(self.env,
                                               "import "+modname,
                                               modname+".get_module_version()")
                #print "version", version
            if version:
                print version
                self.add_module(m, ok=True, external=True, version=version)
            else:
                print "not found"
                self.add_module(m, ok=False)
        else:
            self.add_module(m, ok=False)
        conf.Finish()
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
