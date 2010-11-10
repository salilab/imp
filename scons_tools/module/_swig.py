from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner, Move
import SCons
import os
import sys
import re
import scons_tools.dependency
import scons_tools.module

# standard include files
base_includes= ["IMP_macros.i",
                "IMP_exceptions.i",
                "IMP_directors.i",
                "IMP_types.i",
                "IMP_refcount.i",
                "IMP_streams_kernel.i",
                "IMP_streams.i",]


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
    vars= scons_tools.module.get_module_variables(env)
    repl1 = '"swig::%s_PySwigIterator *"' % vars['PREPROC']
    repl2 = '"swig::%s_SwigPyIterator *"' % vars['PREPROC']
    orig = 'SWIG_IMP.%s_WRAP_H_' % scons_tools.module.get_module_name(env)
    repl = 'SWIG_IMP_%s_WRAP_H_' % scons_tools.module.get_module_name(env)
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


def _action_swig_file(target, source, env):
    vars= scons_tools.module.get_module_variables(env)
    deps= env.get_module_python_modules()
    deps.reverse()
    #print "dependencies are " +str(deps)
    warning="// WARNING Generated file, do not edit, edit the swig.i-in instead."
    preface=[warning,"""

%%module(directors="1") "%s"
%%{
#include <boost/type_traits/is_convertible.hpp>
#include <boost/utility/enable_if.hpp>
#include "IMP.h"
#include "IMP/internal/swig_helpers.h"
#include "IMP/internal/swig.h"
"""%vars['module_include_path'].replace("/", ".")]
    for d in deps:
        if d != "kernel":
            preface.append("#include \"IMP/%s.h\""% d)
    if vars['module'] != 'kernel':
        preface.append("#include \"%(module_include_path)s.h\""%vars)
    preface.append("""%}
%implicitconv;
%include "std_vector.i"
%include "std_string.i"
%include "std_pair.i"

""")
    for i in base_includes:
        preface.append('%include "'+ i + '"')
    preface.append("""
%%include "typemaps.i"
"""%vars)
    preface.append(warning)

    # special case the kernel to make sure that VersionInfo is wrapped
    # before get_version_info() is wrapped.
    preface.append("""%{
#ifdef NDEBUG
#error "The python wrappers must not be built with NDEBUG"
#endif
%}
""")

    for d in deps:
        preface.append("%%import \"IMP_%s.i\""% d)
    preface.append(warning)

    preface.append("""
%%include "%(module_include_path)s/%(module)s_config.h"
"""%vars)
    preface.append(warning)
    preface.append(open(source[0].abspath, "r").read())
    preface.append(warning)
    if vars['module']== 'kernel':
        preface.append("""
namespace IMP {
const VersionInfo& get_module_version_info();
std::string get_example_path(std::string fname);
std::string get_data_path(std::string fname);
}
""")
    else:
        preface.append("""
namespace IMP {
namespace %(module)s {
const VersionInfo& get_module_version_info();
std::string get_example_path(std::string fname);
std::string get_data_path(std::string fname);
}
}
"""%vars)
    preface.append(warning)
    version=source[2].get_contents()
    preface.append("""
%pythoncode {
if get_module_version_info().get_version() != '"""+version+"""':
    sys.stderr.write("WARNING: expected version '"""+version+"""', but got "+ get_module_version_info().get_version() +" when loading module %(module)s. Please make sure IMP is properly built and installed and that matching python and C++ libraries are used.\\n")
}"""%vars)
    deps= source[3].get_contents().split(" ")
    udeps= source[4].get_contents().split(" ")
    preface.append("%pythoncode {")
    for d in deps:
        nm=scons_tools.dependency.get_dependency_string(d).lower()
        preface.append("has_"+nm +"=True")
    for d in udeps:
        nm=scons_tools.dependency.get_dependency_string(d).lower()
        preface.append("has_"+nm+"=False")
    preface.append("}")
    if vars['module'] != "kernel":
        preface.append("""
%pythoncode %{
IMP.used_modules.append(get_module_version_info())
%}""")
    else:
        preface.append("""
%pythoncode %{
used_modules.append(get_module_version_info())
%}""")
    open(target[0].abspath, "w").write("\n".join(preface))

def _print_swig_file(target, source, env):
    print "Generating swig interface "+str(target[0]) + " from " +str(source[0])

SwigPreface = Builder(action=Action(_action_swig_file,
                                    _print_swig_file))


def _action_simple_swig(target, source, env):
    vars= scons_tools.module.get_module_variables(env)
    cppflags= ""
    for x in env.get('CPPFLAGS', []):
        if x.startswith("-I") or x.startswith("-D"):
            cppflags= cppflags+" " + x
    sv= env['SWIGVERSION'].split(".")
    if sv[0]=="1" and sv[1] == "3" and int(sv[2])<34:
        warnings=[]
    else:
        warnings=["-Wextra"]

    command = [env['SWIG'], "-castmode -interface", "_IMP%(module_suffix)s",
               "-DPySwigIterator=%(PREPROC)s_PySwigIterator",
               "-DSwigPyIterator=%(PREPROC)s_SwigPyIterator",
               "-python", "-c++", "-naturalvar",
               "-fvirtual"]+warnings
    # Signal whether we are building the kernel
    if scons_tools.module.get_module_name(env) == 'kernel':
        command.append('-DIMP_SWIG_KERNEL')
    #print base
    command=command+["-o",target[1].abspath, "-oh",target[2].abspath]
    command=command+[" -Ibuild/swig"]+ ["-I"+str(Dir(x)) for x in env.get('CPPPATH', [])]
    command.append("-DIMP_SWIG")
    command.append(source[0].abspath)
    ret= env.Execute(" ".join(command) %vars)
    return ret

def _print_simple_swig(target, source, env):
    print "Running swig on file "+str(source[0].path)

SwigIt = Builder(action=Action(_action_simple_swig,
                                _print_simple_swig))


def _get_swig_version(env):
    """Run the SWIG command line tool to get and return the version number"""
    if not env['SWIG']:
        return ""
    out = os.popen(env['SWIG'] + ' -version').read()
    match = re.search(r'SWIG Version\s+(\S+)$', out, re.MULTILINE)
    if match:
        return match.group(1)
    else:
        return ""

def generate(env):
    """Add Builders and construction variables for swig to an Environment."""
    env['SWIG']              = env.WhereIs('swig')
    env['SWIGVERSION']       = _get_swig_version(env)

def exists(env):
    return env.Detect(['swig'])


def swig_scanner(node, env, path):
    import re
    contents= node.get_contents()
    # scons recurses with the same scanner, rather than the right one
    #print "Scanning "+str(node)
    if str(node).endswith(".h"):
        # we don't care about recursive .hs for running swig
        return []
    else :
        oldret=[]
        ret= ["#/build/include/"+x for x in re.findall('\n%include\s"([^"]*.h)"', contents)]\
            + ["#/build/swig/"+x for x in re.findall('\n%include\s"(IMP_[^"]*.i)"', contents)]\
            + ["#/build/swig/"+x for x in re.findall('\n%import\s"(IMP_[^"]*.i)"', contents)]
        retset=set(ret)
        ret=list(retset)
        ret.sort()
    return ret

def inswig_scanner(node, env, path):
    if str(node).endswith(".i") or str(node).endswith(".h"):
        return swig_scanner(node, env, path)
    ret= swig_scanner(node, env, path)
    for i in base_includes:
        f= "#/build/swig/"+i
        ret.append(f)
    for m in env.get_module_python_modules():
        ret.append("#/modules/"+m+"/pyext/swig.i-in")
    ret.append('#/kernel/pyext/swig.i-in')
    return ret.sorted()

scanner= Scanner(function=swig_scanner, skeys=['.i'], name="IMPSWIG", recursive=True)
# scons likes to call the scanner on nodes which do not exist (making it tricky to parse their contents
# so we have to walk higher up in the tree
inscanner= Scanner(function=inswig_scanner, skeys=['.i-in'], name="IMPINSWIG", recursive=True)
