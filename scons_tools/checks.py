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
        return (bret[0], ret[1]+bret[1])
    return  (True, ret[1])
