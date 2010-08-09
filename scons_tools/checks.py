import imp_module

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
    if context.env['static']:
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


def handle_optional_lib(env, name, lib, header, vars, body="", extra_libs=[],
                        alternate_name=None,
                        liblist=[]):
    lcname= nicename(name)
    ucname= lcname.upper()
    liblist.append(name)
    def _check(context):
        if context.env[lcname] is "no":
            context.Message('Checking for '+name+' ...')
            context.Result("disabled")
            context.env[ucname+'_LIBS']=False
            return False
        elif context.env[lcname] is "yes":
            context.Message('Checking for '+name+' ...')
            if not context.env[lcname+'libs']:
                context.Result("disabled, libs not specified")
                context.env[ucname+'_LIBS']=False
                return False
            else:
                context.Result(context.env[lcname+'libs'])
                context.env[ucname+'_LIBS']=context.env[lcname+'libs']
                return True
        else:
            ret= check_lib(context, lib=lib, header=header,
                           body=body,
                           extra_libs=extra_libs)
            if ret[0]:
                context.env[ucname+'_LIBS']=ret[1]
                context.Message('Checking for '+name+' ...')
                context.Result(" ".join(ret[1]))
            elif alternate_name:
                ret= check_lib(context, lib=alternate_name, header=header,
                                  body=body,
                                  extra_libs=extra_libs)
                if ret[0]:
                    context.env[ucname+'_LIBS']=ret[1]
                    context.Message('Checking for '+name+' ...')
                    context.Result(" ".join(ret[1]))
                else:
                    context.Result(False)
                    context.env[ucname+'_LIBS']=False
            else:
                context.Message('Checking for '+name+' ...')
                context.Result(False)
                context.env[ucname+'_LIBS']=False
            return ret[0]
    from SCons.Script import EnumVariable
    vars.Add(EnumVariable(lcname, 'Whether to use the '+name+' package', "auto", ["yes", "no", "auto"]))
    vars.Add(lcname+'libs', 'Libs to link against when using '+name+'. Needed which "'+lcname+'" is "yes".', None)
    vars.Update(env)
    if not env.GetOption('help'):
        custom_tests = {'CheckThisLib':_check}
        conf = env.Configure(custom_tests=custom_tests)
    #if not env.GetOption('clean') and not env.GetOption('help'):
        if conf.CheckThisLib():
            env.Append(IMP_ENABLED=[ucname])
        else:
            env.Append(IMP_DISABLED=[ucname])
        conf.Finish()
