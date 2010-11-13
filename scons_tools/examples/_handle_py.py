from SCons.Script import Glob, Dir, File, Builder, Action
import re
import scons_tools.data
import scons_tools.examples
import scons_tools.utility


def _find_used(lines, modules):
    res={}
    method='([a-z_0-9]+)\('
    cl='([A-Z_][A-Za-z_0-9]+)'
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
    #print "scanning file", source[0]
    data= scons_tools.data.get(env)
    all= open(source[0].abspath, 'r').read()
    lines=all.split('\n')
    (methods,classes)=  _find_used(lines, scons_tools.data.get(env).modules.keys())
    #print data.examples.keys()
    name= source[0].abspath
    data.examples[scons_tools.utility.get_link_name_from_name(name)].set_processed(classes, methods)
    open(target[0].abspath, "w").write("hi")

def _print_make_used_wrapper( target, source, env):
    print "Parsing example", source[0]

Process= Builder(action=Action(_make_used_wrapper,
                               _print_make_used_wrapper))
