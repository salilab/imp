import atexit
import SCons
import dependency
import data
import scons_tools.module
def _bf_to_str(bf):
    """Convert an element of GetBuildFailures() to a string
    in a useful way."""
    import SCons.Errors
    if bf is None: # unknown targets product None in list
        return '(unknown tgt)'
    elif isinstance(bf, SCons.Errors.StopError):
        return str(bf)
    else:
        try:
            if bf.node:
                return str(bf.node)
        except:
            pass
        try:
            if bf.filename:
                return bf.filename
        except:
            pass
        return 'unknown failure: ' + bf.errstr

def _list(env, name, table):
    ok=[]
    notok=[]
    for k in table.keys():
        if table[k].ok:
            ok.append(k)
        else:
            notok.append(k)
    print "Enabled", name+": ", ", ".join(ok)
    if len(notok) >0:
        print "Disabled", name+":", ", ".join(notok)
    print

def _display_build_summary(env):
    print
    print
    for x in env['IMP_BUILD_SUMMARY']:
        print x
    d= data.get(env)
    _list(env, "dependencies", d.dependencies)
    _list(env, "modules", d.modules)
    _list(env, "applications", d.applications)
    _list(env, "systems", d.systems)


    from SCons.Script import GetBuildFailures
    abf=GetBuildFailures()
    if abf:
        print "Errors building:"
        for bf in abf:
            print "  "+_bf_to_str(bf)

def setup(env):
    atexit.register(_display_build_summary, env)
