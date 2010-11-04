import utility
import SCons
from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner

def _search_for_deps(context, libname, extra_libs, headers, body, possible_deps):
    for i in range(0,len(possible_deps)+1):
        lc= extra_libs+possible_deps[0:i]
        #print "Trying "+ str(i) +" with " +str(lc)
        olibs= context.env.get('LIBS', [])
        context.env.Append(LIBS=lc)
        ret=context.sconf.CheckLibWithHeader(libname, header=headers, call=body, language='CXX',
                                             autoadd=False)
        context.env.Replace(LIBS=olibs)
        if ret:
            return (True, [libname]+lc)
    return (False, None)

def get_dependency_ok(env, dep):
    ret= dep in env['IMP_ENABLED']
    if not ret and dep not in env['IMP_DISABLED']:
        utility.report_error(env, "Unknown dependency "+dep)
    return ret

def get_found_dependencies(env, dependencies):
    ret=[]
    for d in dependencies:
        if d in env['IMP_ENABLED']:
            ret.append(d)
    return ret;

def _get_libvarname(d):
    return "IMP_"+get_dependency_string(d)+"_libs"

def get_all_known_dependencies(env):
    return env.get('IMP_KNOWN_DEPENDENCIES', [])

def get_dependency_libs(env, dependency):
    return env.get(_get_libvarname(dependency), None)

def add_dependency_link_flags(env, dependencies):
    for d in dependencies:
        env.Append(LIBS=env[_get_libvarname(d)])

def check_lib(context, lib, header, body="", extra_libs=[]):
    if type(lib) == list:
        ret=_search_for_deps(context, lib[0], lib[1:], header, body, extra_libs)
    else:
        ret=_search_for_deps(context, lib, [], header, body, extra_libs)
    if not ret[0]:
        return ret
    if context.env['IMP_BUILD_STATIC']:
        utility.make_static_build(context.env)
        if type(lib) == list:
            bret=_search_for_deps(context, lib[0], lib[1:], header, body, extra_libs)
        else:
            bret=_search_for_deps(context, lib, [], header, body, extra_libs)
        utility.unmake_static_build(context.env)
        # should be the sum of the two
        if bret[0]:
            return (bret[0], ret[1]+bret[1])
        else:
            return (False, None)
    return  (True, ret[1])

def get_dependency_string(name):
    lname= name.lower()
    nname=lname.replace(".", "_")
    return nname


def add_external_library(env, name, lib, header, body="", extra_libs=[],
                        alternate_name=None):
    lcname= get_dependency_string(name)
    ucname= lcname.upper()
    env.Append(IMP_KNOWN_DEPENDENCIES=[name])
    def _check(context):
        if context.env[lcname] is "no":
            context.Message('Checking for '+name+' ...')
            context.Result("disabled")
            context.env[_get_libvarname(name)]=False
            return False
        elif context.env[lcname] is "yes":
            context.Message('Checking for '+name+' ...')
            if context.env.get(lcname+"libs", None) is None:
                context.Result("disabled, libs not specified")
                context.env[_get_libvarname(name)]=False
                return False
            else:
                val=context.env[lcname+'libs'].split(":")
                #print val
                context.Result(" ".join(val))
                context.env[_get_libvarname(name)]=val
                return True
        else:
            ret= check_lib(context, lib=lib, header=header,
                           body=body,
                           extra_libs=extra_libs)
            context.Message('Checking for '+name+' ...')
            if ret[0]:
                context.env[_get_libvarname(name)]=ret[1]
                context.Result(" ".join(ret[1]))
            elif alternate_name:
                ret= check_lib(context, lib=alternate_name, header=header,
                                  body=body,
                                  extra_libs=extra_libs)
                if ret[0]:
                    context.env[_get_libvarname(name)]=ret[1]
                    context.Result(" ".join(ret[1]))
                else:
                    context.Result(False)
                    context.env[_get_libvarname(name)]=False
            else:
                context.Result(False)
                context.env[_get_libvarname(name)]=False
            return ret[0]
    from SCons.Script import EnumVariable
    vars = SCons.Variables.Variables(files=[File('#/config.py').abspath])
    vars.Add(SCons.Variables.EnumVariable(lcname, 'Whether to use the '+name+' package', "auto", ["yes", "no", "auto"]))
    vars.Add(lcname+'libs', 'Libs to link against when using '+name+'. Needed which "'+lcname+'" is "yes".', None)
    tenv= SCons.Environment.Environment(variables=vars)
    env['IMP_VARIABLES'].Add(SCons.Variables.EnumVariable(lcname, 'Whether to use the '+name+' package', "auto", ["yes", "no", "auto"]))
    env['IMP_VARIABLES'].Add(lcname+'libs', 'Libs to link against when using '+name+'. Needed which "'+lcname+'" is "yes".', None)
    env[lcname]= tenv[lcname]
    #env['IMP_VARIABLES'].Update(env)
    if tenv.get(lcname+"libs", None) is not None:
        env[lcname+"libs"]= tenv[lcname+"libs"]
    if not env.GetOption('help'):
        custom_tests = {'CheckThisLib':_check}
        conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
        if conf.CheckThisLib():
            env.Append(IMP_ENABLED=[name])
            env.Append(IMP_CONFIGURATION=[lcname+"='yes'"])
            env.Append(IMP_CONFIGURATION=[lcname+"libs='"+":".join(env[_get_libvarname(name)])+"'"])
        else:
            env.Append(IMP_DISABLED=[name])
        conf.Finish()
