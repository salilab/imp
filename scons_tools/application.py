from scons_tools import get_bin_environment
from imp_module import dependencies_to_libs
from imp_module import check_libraries_and_headers
from imp_module import process_dependencies
from imp_module import expand_dependencies
from SCons.Script import Builder, File, Action, Glob, Return, Alias, Dir


def IMPCPPApplication(envi, target, source, required_modules=[],
                      optional_dependencies=[],
                      required_dependencies=[], required_libraries=[],
                      required_headers=[]):
    env= get_bin_environment(envi)
    if env['fastlink']:
        for l in expand_dependencies(env,required_modules, False):
            if l != 'kernel':
                env.Append(LINKFLAGS=['-limp_'+l])
            else:
                env.Append(LINKFLAGS=['-limp'])
    env.Prepend(LIBS=dependencies_to_libs(env, required_modules))
    env.Prepend(CPPPATH=["#/build/include"])
    env.Prepend(LIBPATH=["#/build/lib"])
    try:
        rp= process_dependencies(env, required_dependencies, True)
    except EnvironmentError, e:
        print "Application", str(target), "cannot be built due to missing dependencies."
        return
    op= process_dependencies(env, optional_dependencies)
    env.Append(LIBS=rp+op)
    if len(required_libraries)+len(required_headers) > 0:
        check_libraries_and_headers(env, required_libraries, required_headers)
    prog= env.Program(target=target, source=source)
    bindir = env.GetInstallDirectory('bindir')
    build= env.Install("#/build/bin", prog)
    env['application_name']=str(target)
    install = env.Install(bindir, prog)

    env.SConscript('test/SConscript', exports=['env'])
    env.Alias(env['application_name'], build)
    env.Alias(env['application_name']+"-install", install)
    env.Alias("all", build)
    env.Alias("install", install)
def IMPApplicationTest(env, python_tests=[]):
    files= ["#/tools/imppy.sh", "#/scons_tools/run-all-tests.py"]+\
        [File(x).abspath for x in python_tests]
    test = env.IMPApplicationRunTest(target="test.passed", source=files)
    env.AlwaysBuild("test.passed")
    env.Requires(test, env.Alias(env['application_name']))
    env.Alias('test', test)
