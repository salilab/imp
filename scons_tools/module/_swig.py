from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner, Move
import SCons
import os
import sys
import re
import scons_tools.dependency
import scons_tools.module
import scons_tools.data

# standard include files
base_includes= ["IMP_base.macros.i",
                "IMP_base.exceptions.i",
                "IMP_base.directors.i",
                "IMP_base.types.i",
                "IMP_base.refcount.i",
                "IMP_base.streams.i",
                "IMP_base.streams_kernel.i"]
kernel_includes= ["IMP_kernel.macros.i"]


def _null_scanner(node, env, path):
    #print "null scanning", node.abspath
    return []

# 1. Workaround for SWIG bug #1863647: Ensure that the PySwigIterator class
#    (SwigPyIterator in 1.3.38 or later) is renamed with a module-specific
#    prefix, to avoid collisions when using multiple modules
# 2. If module names contain '.' characters, SWIG emits these into the CPP
#    macros used in the director header. Work around this by replacing them
#    with '_'. A longer term fix is not to call our modules "IMP.foo" but
#    to say %module(package=IMP) foo but this doesn't work in SWIG stable
#    as of 1.3.36 (Python imports incorrectly come out as 'import foo'
#    rather than 'import IMP.foo'). See also IMP bug #41 at
#    https://salilab.org/imp/bugs/show_bug.cgi?id=41
def _action_patch_swig_wrap(target, source, env):
    lines = file(source[0].path, 'r').readlines()
    vars= scons_tools.module._get_module_variables(env)
    repl1 = '"swig::%s_PySwigIterator *"' % vars['PREPROC']
    repl2 = '"swig::%s_SwigPyIterator *"' % vars['PREPROC']
    orig = 'SWIG_IMP.%s_WRAP_H_' % scons_tools.module._get_module_name(env)
    repl = 'SWIG_IMP_%s_WRAP_H_' % scons_tools.module._get_module_name(env)
    fh= file(target[0].path, 'w')
    for line in lines:
        line = line.replace('"swig::PySwigIterator *"', repl1)
        line = line.replace('"swig::SwigPyIterator *"', repl2)
        line = line.replace(orig, repl)
        line = line.replace("wrap.h-in", "wrap.h")
        # for some reason swig has issues with directors and VersionInfo
        # when %extend is used
        line = line.replace(" VersionInfo ", " IMP::VersionInfo ")
        line = line.replace("(VersionInfo ", "(IMP::VersionInfo ")
        line = line.replace("<VersionInfo ", "<IMP::VersionInfo ")
        line = line.replace("<:", "< :") # swig generates bad C++ code
        fh.write(line.replace('"swig::SwigPyIterator *"', repl2))
    fh.close()

def _print_patch_swig_wrap(target, source, env):
    print "Patching swig file "+str(target[0])

PatchSwig = Builder(action=Action(_action_patch_swig_wrap,
                                _print_patch_swig_wrap))


def _action_simple_swig(target, source, env):
    vars= scons_tools.module._get_module_variables(env)
    cppflags= ""
    for x in env.get('CPPFLAGS', []):
        if x.startswith("-I") or x.startswith("-D"):
            cppflags= cppflags+" " + x
    sv= env['SWIGVERSION'].split(".")
    if sv[0]=="1" and sv[1] == "3" and int(sv[2])<34:
        warnings=[]
    else:
        warnings=["-Wextra"]

    command = [env['SWIG'], "-castmode -interface", "%(module_pylibname)s",
               "-DPySwigIterator=%(PREPROC)s_PySwigIterator",
               "-DSwigPyIterator=%(PREPROC)s_SwigPyIterator",
               "-python", "-c++", "-naturalvar",
               "-fvirtual"]+warnings
    # if building a module by itself, we need to find swig headers
    command+= ["-I"+x for x in
               scons_tools.utility.get_env_paths(env, 'includepath')]\

    # Signal whether we are building the kernel
    if scons_tools.module._get_module_name(env) == 'kernel':
        command.append('-DIMP_SWIG_KERNEL')
    #print base
    command=command+["-o",target[1].abspath, "-oh",target[2].abspath]
    ussp=env.get('swigpath', "")
    command=command+[" -I"+Dir("#/build/swig").path]\
        + ["-I"+Dir("#/build/include").abspath]\
        + ["-I"+str(x) for x in
           scons_tools.utility.get_env_paths(env, 'swigpath')]
    command.append(source[0].abspath)
    final_command=" ".join(command) %vars
    ret= env.Execute(final_command)

    modulename = vars['module']
    oname=File("#/build/src/"+modulename+"_swig/"\
                   +vars['module_include_path'].replace("/", ".")+".py")
    #print oname.path, "moving to", target[0].path
    # scons build in Move produces an error with no explaination
    ret= env.Execute("mv "+oname.abspath+" "+target[0].abspath)
    return ret

def _print_simple_swig(target, source, env):
    print "Running swig on file "+str(source[0].path)

def _action_version_check(target, source, env):
    def get_module(name):
        dta= scons_tools.data.get(env)
        ln= dta.modules[name].libname
        inm= ln.replace("imp_", "imp.").replace("imp", "IMP")
        return inm
    out= open(target[0].abspath, "w")
    print >> out, "def check_version(myversion):"
    print >> out, "  def _check_one(name, expected, found):"
    print >> out, "    if expected != found:"
    print >> out, "      raise RuntimeError('Expected version '+expected+' but got '+ found \\"
    print >> out, "           +' when loading module '+name\\"
    print >> out, "            +'. Please make sure IMP is properly built and installed and that matching python and C++ libraries are used.\\n')"
    myversion= source[0].get_contents()
    print >> out, "  _check_one('"+scons_tools.module._get_module_name(env)+\
          "', '"+myversion+"', myversion)"
    for i in range(1,len(source), 2):
        mn= source[i].get_contents()
        ver= source[i+1].get_contents()
        print >> out, "  import "+get_module(mn)
        print >> out, "  _check_one('"+mn+\
          "', '"+ver+"', "+get_module(mn)+".get_module_version())"

def _print_version_check(target, source, env):
    print "Building version check "+target[0].abspath

VersionCheck = Builder(action=Action(_action_version_check,
                                _print_version_check))


def swig_scanner(node, env, path):
    import re
    contents= node.get_contents()
    # scons recurses with the same scanner, rather than the right one
    # print "Scanning "+str(node)
    dta= scons_tools.data.get(env)
    if str(node).endswith(".h"):
        # we don't care about recursive .hs for running swig
        return []
    else :
        oldret=[]
        ret=[]
        for x in re.findall('\n%include\s"([^"]*.h)"', contents):
            if x.startswith("IMP/"):
                xc= x[4:]
                if xc.find("/") != -1:
                    module= xc[0:xc.find("/")]
                else:
                    module="kernel"
                if module=="internal":
                    module="kernel"
                if not dta.modules[module].external:
                    ret.extend(["#/build/include/"+x])

        for x in re.findall('\n%include\s"IMP_([^"]*).i"', contents)\
                +re.findall('\n%import\s"IMP_([^"]*).i"', contents):
            mn= x.split(".")[0]
            if not dta.modules[mn].external:
                ret.append("#/build/swig/IMP_"+x+".i")
        retset=set(ret)
        ret=list(retset)
        ret.sort()
    return ret

def inswig_scanner(node, env, path):
    if str(node).endswith(".i") or str(node).endswith(".h"):
        return swig_scanner(node, env, path)
    ret= swig_scanner(node, env, path)
    for i in base_includes:
        if not dta.modules[i].external:
            f= "#/build/swig/"+i
            ret.append(f)
    for m in scons_tools.module._get_module_python_modules(env):
        if not dta.modules[m].external:
            ret.append("#/modules/"+m+"/pyext/swig.i-in")
    return ret.sorted()

scanner= Scanner(function=swig_scanner, skeys=['.i'], name="IMPSWIG", recursive=True)
# scons likes to call the scanner on nodes which do not exist (making it tricky to parse their contents
# so we have to walk higher up in the tree

def get_swig_action(env):
    comstr="%sRunning swig on %s$SOURCE%s"%(env['IMP_COLORS']['purple'],
                                            env['IMP_COLORS']['end'],
                                            env['IMP_COLORS']['end'])
    return Builder(action=Action(_action_simple_swig,
                                 _print_simple_swig,
                                 comstr=comstr),
                   source_scanner= scanner,
                   comstr=comstr)
