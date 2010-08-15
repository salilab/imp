import imp_module
import SCons
from SCons.Script import Glob, Dir, File, Builder, Action, Exit, Scanner

def _search_for_deps(context, libname, headers, body, possible_deps):
    for i in range(0,len(possible_deps)+1):
        lc= possible_deps[0:i]
        #print "Trying "+ str(i) +" with " +str(lc)
        olibs= context.env.get('LIBS', [])
        context.env.Append(LIBS=lc)
        ret=context.sconf.CheckLibWithHeader(libname, header=headers, call=body, language='CXX',
                                             autoadd=False)
        context.env.Replace(LIBS=olibs)
        if ret:
            return (True, [libname]+lc)
    return (False, None)

def check_lib(context, lib, header, body="", extra_libs=[]):
    ret= _search_for_deps(context, lib, header, body, extra_libs)
    if not ret[0]:
        return ret
    if context.env['IMP_BUILD_STATIC']:
        imp_module.make_static_build(context.env)
        bret=_search_for_deps(context, lib, header, body, extra_libs)
        imp_module.unmake_static_build(context.env)
        # should be the sum of the two
        if bret[0]:
            return (bret[0], ret[1]+bret[1])
        else:
            return (False, None)
    return  (True, ret[1])

def nicename(name):
    lname= name.lower()
    nname=lname.replace(".", "_")
    return nname


def add_external_library(env, name, lib, header, body="", extra_libs=[],
                        alternate_name=None):
    lcname= nicename(name)
    ucname= lcname.upper()
    env.Append(IMP_EXTERNAL_LIBS=[name])
    def _check(context):
        if context.env[lcname] is "no":
            context.Message('Checking for '+name+' ...')
            context.Result("disabled")
            context.env[ucname+'_LIBS']=False
            return False
        elif context.env[lcname] is "yes":
            context.Message('Checking for '+name+' ...')
            if context.env[lcname+'libs'] is None:
                context.Result("disabled, libs not specified")
                context.env[ucname+'_LIBS']=False
                return False
            else:
                val=context.env[lcname+'libs'].split(":")
                #print val
                context.Result(" ".join(val))
                context.env[ucname+'_LIBS']=val
                return True
        else:
            ret= check_lib(context, lib=lib, header=header,
                           body=body,
                           extra_libs=extra_libs)
            context.Message('Checking for '+name+' ...')
            if ret[0]:
                context.env[ucname+'_LIBS']=ret[1]
                context.Result(" ".join(ret[1]))
            elif alternate_name:
                ret= check_lib(context, lib=alternate_name, header=header,
                                  body=body,
                                  extra_libs=extra_libs)
                if ret[0]:
                    context.env[ucname+'_LIBS']=ret[1]
                    context.Result(" ".join(ret[1]))
                else:
                    context.Result(False)
                    context.env[ucname+'_LIBS']=False
            else:
                context.Result(False)
                context.env[ucname+'_LIBS']=False
            return ret[0]
    from SCons.Script import EnumVariable
    vars = SCons.Variables.Variables(files=[File('#/config.py').abspath])
    vars.Add(SCons.Variables.EnumVariable(lcname, 'Whether to use the '+name+' package', "auto", ["yes", "no", "auto"]))
    vars.Add(lcname+'libs', 'Libs to link against when using '+name+'. Needed which "'+lcname+'" is "yes".', None)
    tenv= SCons.Environment.Environment(variables=vars)
    env['IMP_VARIABLES'].Add(SCons.Variables.EnumVariable(lcname, 'Whether to use the '+name+' package', "auto", ["yes", "no", "auto"]))
    env['IMP_VARIABLES'].Add(lcname+'libs', 'Libs to link against when using '+name+'. Needed which "'+lcname+'" is "yes".', None)
    env[lcname]= tenv[lcname]
    if tenv.get(lcname+"libs", None) is not None:
        env[lcname+"libs"]= tenv[lcname+"libs"]
    if not env.GetOption('help'):
        custom_tests = {'CheckThisLib':_check}
        conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
        if conf.CheckThisLib():
            env.Append(IMP_ENABLED=[ucname])
            env.Append(IMP_CONFIGURATION=[lcname+"='yes'"])
            env.Append(IMP_CONFIGURATION=[lcname+"libs='"+":".join(env[ucname+"_LIBS"])+"'"])
        else:
            env.Append(IMP_DISABLED=[ucname])
        conf.Finish()
