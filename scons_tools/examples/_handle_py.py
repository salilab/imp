from SCons.Script import Glob, Dir, File, Builder, Action
import re
import scons_tools.examples

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
def _find_used(lines, modules):
    res={}
    method='([a-z_]+)\('
    cl='([A-Z_][A-Za-z_]+)'
    res['kernel']=(re.compile('IMP\\.'+method), re.compile('IMP\\.'+cl))
    for m in modules:
        rm=re.compile('IMP\\.'+m+'\\.'+method)
        rc=re.compile('IMP\\.'+m+'\\.'+cl)
        res[m]=(rm, rc)
    methods={}
    classes={}
    for k in res.keys():
        methods[k]=[]
        classes[k]=[]
    for l in lines:
        for k in res.keys():
            mmatch=res[k][0].search(l)
            if mmatch:
                methods[k].append(mmatch.groups()[0])
            cmatch=res[k][1].search(l)
            if cmatch:
                classes[k].append(cmatch.groups()[0])
    return (methods, classes)

def _make_used_wrapper( target, source, env):
    link=source[0].get_contents()
    all= open(source[1].abspath, 'r').read()
    lines=all.split('\n')
    (methods,classes)=  _find_used(lines, ['core', 'atom', 'container', 'domino2', 'em',
                                         'saxs', ])
    out= open(target[0].abspath, 'w')
    for k in methods.keys():
        #if len(methods[k]) > 0:
        #    print "module", k, "methods", methods[k]
        if len(classes[k])+len(methods[k]) > 0:
            print >> out, "namespace IMP {"
            if k != "kernel":
                print >> out, "namespace", k, "{"
            print >> out, "/**"
            for i, c in enumerate(classes[k]):
                if c in classes[k][i+1:]:
                    continue
                if not k in excluded_classes.keys() or c not in excluded_classes[k]:
                    print >> out, "\\class ", c
                    print >> out, "-", link
            if k in included_methods.keys():
                for i,m in enumerate(methods[k]):
                    if m in methods[k][i+1:]:
                        continue
                    if m in included_methods[k].keys():
                        print >> out, "\\fn", m+included_methods[k][m]
                        print >> out, "-", link
            print >> out, "*/"
            if k != "kernel":
                print >> out, "}"
            print >> out, "}"

def print_make_used_wrapper(target, source, env):
    print "Finding used classes for", str(source[1])

_Wrapper= Builder(action=Action(_make_used_wrapper,
                                print_make_used_wrapper))

#def _action_colorize_python(target, source, env):
#    _colorize_python.Parser(open(str(source[0]), "r").read(),
#                           open(str(target[0]), "w")).format(None, None)

#def _print_colorize_python(target, source, env):
#    print "Generating doxygen pages for examples"

#_ColorizePython = Builder(action=Action(_action_colorize_python,
#                                       _print_colorize_python))

def handle_python_file(env, file):
    nm=str(file).split('/')[-1]
    basenm= nm.split('.')[0]
    link=scons_tools.examples.get_link_from_name(nm)
    #print "wrapping", str(file), "to get", "generated/"+basenm.replace(' ', '_')+".dox"
    wrapper=_Wrapper(env, target="generated/"+basenm.replace(' ', '_')+".dox", source=[env.Value(link),
                                                                     file])
    #c= ColorizePython(env, target=["generated/"+basenm+".html"],
    #                  source=[file])


if __name__ == '__main__':
    t= open('../svn/kernel/examples/nup84_cg.py', 'r').read()
    l= t.split('\n')
    _handle_python(l, ["core", "algebra", "atom"])
