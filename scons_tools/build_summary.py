import atexit
import checks
import SCons
def _bf_to_str(bf):
    """Convert an element of GetBuildFailures() to a string
    in a useful way."""
    import SCons.Errors
    if bf is None: # unknown targets product None in list
        return '(unknown tgt)'
    elif isinstance(bf, SCons.Errors.StopError):
        return str(bf)
    elif bf.node:
        return str(bf.node)
    elif bf.filename:
        return bf.filename
    return 'unknown failure: ' + bf.errstr

def _display_build_summary(env):
    print
    print
    found_deps=[]
    unfound_deps=[]
    for x in env['IMP_BUILD_SUMMARY']:
        print x
    if env['IMP_PROVIDE_PYTHON']:
        found_deps.append('python')
    else:
        unfound_deps.append('python')
    for l in env['IMP_EXTERNAL_LIBS']:
        if env[checks.nicename(l).upper()+"_LIBS"]:
            found_deps.append(l)
        else:
            unfound_deps.append(l)
    print "Enabled dependencies: ",", ".join(found_deps)
    print "Disabled/unfound optional dependencies: ",\
        ", ".join(unfound_deps)
    print
    disabledm=[]
    enabledm=[]
    for m in env['IMP_MODULES_ALL']:
        if not env.get(m+"_ok", False):
            disabledm.append(m)
        else:
            enabledm.append(m)
    print "Enabled modules: ", ", ".join(enabledm)
    if len(disabledm) >0:
        print "Disabled modules:", ", ".join(disabledm)
    from SCons.Script import GetBuildFailures
    abf=GetBuildFailures()
    if abf:
        print "Errors building:"
        for bf in abf:
            print "  "+_bf_to_str(bf)

def setup(env):
    atexit.register(_display_build_summary, env)
