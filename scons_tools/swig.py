
import imp_module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner
import SCons
import os
import sys
import re

# standard include files
base_includes= ["IMP_macros.i",
                "IMP_exceptions.i",
                "IMP_directors.i",
                "IMP_refcount.i",
                "IMP_streams_kernel.i",
                "IMP_streams.i",
                "IMP_decorators.i",
                "IMP_typemaps.i"]


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
    repl1 = '"swig::%s_PySwigIterator *"' % env['IMP_MODULE_PREPROC']
    repl2 = '"swig::%s_SwigPyIterator *"' % env['IMP_MODULE_PREPROC']
    orig = 'SWIG_IMP.%s_WRAP_H_' % env['IMP_MODULE']
    repl = 'SWIG_IMP_%s_WRAP_H_' % env['IMP_MODULE']
    fh= file(target[0].path, 'w')
    for line in lines:
        line = line.replace('"swig::PySwigIterator *"', repl1)
        line = line.replace('"swig::SwigPyIterator *"', repl2)
        line = line.replace(orig, repl)
        line = line.replace("wrap.h-in", "wrap.h")
        line = line.replace("<:", "< :") # swig generates bad C++ code
        fh.write(line.replace('"swig::SwigPyIterator *"', repl2))
    fh.close()

def _print_patch_swig_wrap(target, source, env):
    print "Patching swig file "+str(target[0])

PatchSwig = Builder(action=Action(_action_patch_swig_wrap,
                                _print_patch_swig_wrap))


def _action_swig_file(target, source, env):
    vars= imp_module.make_vars(env)
    deps=(imp_module.expand_dependencies(env, env['IMP_REQUIRED_MODULES']))
    deps.reverse()
    #print "dependencies are " +str(deps)
    warning="// WARNING Generated file, do not edit, edit the swig.i-in instead."
    preface=[warning,"""

%%module(directors="1") "%s"

%%{
#include "IMP.h"
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

/* Don't wrap classes that provide no methods usable in Python */
%%ignore IMP::ValidDefault;
%%ignore IMP::NullDefault;
%%ignore IMP::UninitializedDefault;
%%ignore IMP::Comparable;
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
%%include "%(module_include_path)s/config.h"
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
    preface.append("""
%%pythoncode {
if get_module_version_info().get_version() != "%(version)s":
    sys.stderr.write("WARNING: expected version %(version)s, but got "+ get_module_version_info().get_version() +" when loading module %(module)s. Please make sure IMP is properly built and installed and that matching python and C++ libraries are used.")
}"""%vars)

    open(target[0].abspath, "w").write("\n".join(preface))

def _print_swig_file(target, source, env):
    print "Generating swig interface "+str(target[0]) + " from " +str(source[0])

SwigPreface = Builder(action=Action(_action_swig_file,
                                    _print_swig_file))


def _action_simple_swig(target, source, env):
    vars= imp_module.make_vars(env)
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
               "-python", "-c++", "-naturalvar"]+warnings
    # Signal whether we are building the kernel
    if env['IMP_MODULE'] == 'kernel':
        command.append('-DIMP_SWIG_KERNEL')
    #print base
    command=command+["-o",target[1].abspath, "-oh",target[2].abspath]
    command=command+[" -Ibuild/swig"]+ ["-I"+str(Dir(x)) for x in env.get('CPPPATH', [])]
    command.append("-DIMP_SWIG")
    command.append(source[0].abspath)
    return env.Execute(" ".join(command) %vars)

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


def _check(context):
    needversion = [1,3,40]
    needversion_str = ".".join([str(x) for x in needversion])
    failmsg = """
SWIG version %s or later must be installed to support Python, but
%s.
Please make sure 'swig' is found in the path passed to scons.

In particular, if you have SWIG installed in a non-standard location,
please use the 'path' option to add this location to the search path.
For example, if you have SWIG installed in /opt/local/bin/, edit (or create)
config.py and add the line

path='/opt/local/bin'

Since SWIG could not be found, proceeding to build IMP without Python support.

"""
    context.Message('Checking for SWIG version >= %s... ' % needversion_str)
    version = context.env['SWIGVERSION']
    try:
        v = [int(x) for x in version.split(".")]
    except ValueError:
        context.Result('not found')
        print failmsg % (needversion_str,
                         "it could not be found on your system")
        return False
    if v >= needversion:
        context.Result('ok, %s found' % version)
        return True
    else:
        context.Result('no, %s found' % version)
        print failmsg % (needversion_str,
                         "only an older version (%s) " % version + \
                         "was found on your system")
        return False

def configure_check(env):
    custom_tests = {'CheckSWIG':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if not env.GetOption('clean') and not env.GetOption('help') \
       and conf.CheckSWIG() is False:
        env['python']=False
    conf.Finish()


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
    for m in env['IMP_REQUIRED_MODULES']:
        ret.append("#/build/swig/IMP_"+m+".i")
    return ret

scanner= Scanner(function=swig_scanner, skeys=['.i'], name="IMPSWIG", recursive=True)
# scons likes to call the scanner on nodes which do not exist (making it tricky to parse their contents
# so we have to walk higher up in the tree
inscanner= Scanner(function=inswig_scanner, skeys=['.i-in'], name="IMPINSWIG", recursive=True)
