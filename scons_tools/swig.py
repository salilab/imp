
import imp_module
from SCons.Script import Glob, Dir, File, Builder, Action, Exit
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
    print "Patching swig file"

PatchSwig = Builder(action=Action(_action_patch_swig_wrap,
                                _print_patch_swig_wrap))


def _action_simple_swig(target, source, env):
    vars= imp_module.make_vars(env)
    #print [x.abspath for x in source]
    cppflags= ""
    for x in env['CPPFLAGS']:
        if x.startswith("-I") or x.startswith("-D"):
            cppflags= cppflags+" " + x
    base="swig -interface _IMP%(module_suffix)s -DPySwigIterator=%(PREPROC)s_PySwigIterator -DSwigPyIterator=%(PREPROC)s_SwigPyIterator -Ibuild/include -python -c++ -naturalvar "%vars
    #print base
    out= "-o "+ target[0].abspath
    doti= source[0].abspath
    includes= " ".join(["-I"+str(x) for x in env['CPPPATH']])\
        +" -I"+Dir("#/build/swig").abspath #+ " -I"+Dir("#/build/include").abspath
    command=base + " " +out + " "\
         + " "+ includes + " " +cppflags + "-DIMP_SWIG " + doti
    #print command
    env.Execute(command)

def _print_simple_swig(target, source, env):
    print "Generating swig file"

SwigIt = Builder(action=Action(_action_simple_swig,
                                _print_simple_swig))


def _get_swig_version(env):
    """Run the SWIG command line tool to get and return the version number"""
    #print "swig version"
    out = os.popen('swig' + ' -version').read()
    match = re.search(r'SWIG Version\s+(\S+)$', out, re.MULTILINE)
    print match
    if match:
        #print "Found " + match.group(1)
        return match.group(1)
    else:
        return ""


def _check(context):
    context.Message('Checking for swig version ')
    try:
        out = os.popen('swig' + ' -version').read()
        match = re.search(r'SWIG Version\s+(\S+)$', out, re.MULTILINE)
        version= match.group(1)
    except:
        print sys.exc_info()
        context.Result("failed")
        return False
    sv= version.split(".")
    v= [int(sv[0]), int(sv[1]), int(sv[2])]
    success = v> [1,3,24]
    if success:
        context.Result(version)
        return True
    else:
        return False

def configure_check(env):
    custom_tests = {'CheckSWIG':_check}
    conf = env.Configure(custom_tests=custom_tests)
    if not env.GetOption('clean') and not env.GetOption('help') \
       and conf.CheckSWIG() is False:
        print """SWIG with a version > 1.3.34 must be installed to support python. Please make sure 'swig' is found in the path passed to scons."""
        env['python']=False
    conf.Finish()
