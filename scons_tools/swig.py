
import imp_module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner
import os
import sys
import re

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
        line = line.replace("wrap.h", "patched_wrap.h")
        fh.write(line.replace('"swig::SwigPyIterator *"', repl2))
    fh.close()

def _print_patch_swig_wrap(target, source, env):
    print "Patching swig file "+str(target)

PatchSwig = Builder(action=Action(_action_patch_swig_wrap,
                                _print_patch_swig_wrap))


def _action_swig_file(target, source, env):
    vars= imp_module.make_vars(env)
    deps=(imp_module.expand_dependencies(env, env['IMP_REQUIRED_MODULES']))
    deps.reverse()
    #print "dependencies are " +str(deps)
    preface=["""/*
 *  WARNING Generated file, do not edit, edit the swig.i instead
 */

%%module(directors="1") "%s"

%%{
#include "IMP.h"
"""%vars['module_include_path'].replace("/", ".")]
    for d in deps:
        if d != "kernel":
            preface.append("#include \"IMP/%s.h\""% d)
    if vars['module'] != 'kernel':
        preface.append("#include \"%(module_include_path)s.h\""%vars)
    preface.append("""%%}
%%include "std_vector.i"
%%include "std_string.i"
%%include "std_pair.i"

%%include "IMP_macros.i"
%%include "IMP_exceptions.i"
%%include "IMP_directors.i"
%%include "IMP_refcount.i"
%%include "IMP_streams_kernel.i"
%%include "IMP_streams.i"
%%include "IMP_decorators.i"
%%include "IMP_typemaps.i"

%%include "typemaps.i"

/* Don't wrap classes that provide no methods usable in Python */
%%ignore IMP::ValidDefault;
%%ignore IMP::NullDefault;
%%ignore IMP::UninitializedDefault;
%%ignore IMP::Comparable;
"""%vars)

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


    preface.append("""
%%include "%(module_include_path)s/config.h"
"""%vars)

    preface.append(open(source[0].abspath, "r").read())

    if vars['module']== 'kernel':
        preface.append("""
namespace IMP {
const VersionInfo& get_module_version_info();
}
""")
    else:
        preface.append("""
namespace IMP {
namespace %(module)s {
const VersionInfo& get_module_version_info();
}
}
"""%vars)

    preface.append("""
%%pythoncode {
if get_module_version_info().get_version() != "%(version)s":
    sys.stderr.write("WARNING: expected version %(version)s, but got "+ get_module_version_info().get_version() +" when loading module %(module)s.")
}"""%vars)

    open(target[0].abspath, "w").write("\n".join(preface))

def _print_swig_file(target, source, env):
    print "Generating swig preface "+str(target)

SwigPreface = Builder(action=Action(_action_swig_file,
                                    _print_swig_file))


def _action_simple_swig(target, source, env):
    vars= imp_module.make_vars(env)
    cppflags= ""
    for x in env.get('CPPFLAGS', []):
        if x.startswith("-I") or x.startswith("-D"):
            cppflags= cppflags+" " + x

    base = env['SWIG'] + " -Wall -w473 -interface  _IMP%(module_suffix)s -DPySwigIterator=%(PREPROC)s_PySwigIterator -DSwigPyIterator=%(PREPROC)s_SwigPyIterator -python -c++ -naturalvar "%vars
    #print base
    out= "-o "+ target[1].abspath
    doti= source[0].abspath
    includes= " -I"+Dir("#/build/swig").abspath+" "+" ".join(["-I"+str(x) for x in env.get('CPPPATH', []) if not x.startswith("#")]) #+ " -I"+Dir("#/build/include").abspath
    # scons puts cppflags before includes, so we should too
    command=base + " " +out + " "\
         + " " +cppflags+ " -Ibuild/include "+ includes + " -DIMP_SWIG " + doti
    return env.Execute(command)

def _print_simple_swig(target, source, env):
    print "Generating swig file "+str(target[0])

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
    needversion = [1,3,31]
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
    fname= node.abspath
    try:
        contents= open(fname, 'r').read()
        # swig is dumb and gets the paths wrong
    except:
        try:
            fname=(File("#/"+env['repository']+"/"+str(node))).abspath
            #os.path.join(env['repository'],str(node))
            contents= open(fname, 'r').read()
        except:
            print "error opening file " +str(node) +" at "+ fname#+ " trying source tree"
            return []
    ret= [File("#/build/include/"+x) for x in re.findall('\n%include\s"([^"]*.h)"', contents)]\
        + [File("#/build/swig/"+x) for x in re.findall('\n%include\s"(IMP_[^"]*.i)"', contents)]\
        + [File("#/build/swig/"+x) for x in re.findall('\n%import\s"(IMP_[^"]*.i)"', contents)]
        #\
            #+ [File('#/build/include/IMP/macros.h')] \
            #+ [File('#/build/include/IMP/container_macros.h')]
    #print "deps for " +fname + " are "+str([str(x) for x in ret])
    return ret

scanner= Scanner(function=swig_scanner, skeys=['.i'], name="IMPSWIG")
