import atexit
import SCons
import dependency
import data
import os
import scons_tools.module
import scons_tools.config_py
from SCons.Script import File, Dir
import StringIO
import pickle
import glob

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
        try:
            return 'unknown failure: ' + bf.errstr
        except:
            return 'really unknown failure: ' + str(bf)

def _list(env, name, table, check_external=False):
    ok=[]
    notok=[]
    for k in table.keys():
        if check_external and table[k].external:
            continue
        if table[k].ok:
            ok.append(k)
        else:
            notok.append(k)
    if len(ok) > 0:
        print "Enabled", name+": ", ", ".join([env['IMP_COLORS']['green']+x+env['IMP_COLORS']['end'] for x in ok])
    if len(notok) >0:
        print "Disabled", name+":", ", ".join([env['IMP_COLORS']['red']+x+env['IMP_COLORS']['end'] for x in notok])
    if len(ok) >0 or len(notok) > 0:
        print

def _list_files(env, name, directory):
    ok=[]
    notok=[]
    for f in glob.glob(Dir(directory).abspath+"/*"):
        nm= os.path.split(f)[1]
        dta= eval(open(f, "r").read())
        if dta.has_key("external"):
            continue
        if dta["ok"]:
            ok.append(nm)
        else:
            notok.append(nm)
    if len(ok) > 0:
        print "Enabled", name+": ", ", ".join([env['IMP_COLORS']['green']+x+env['IMP_COLORS']['end'] for x in ok])
    if len(notok) >0:
        print "Disabled", name+":", ", ".join([env['IMP_COLORS']['red']+x+env['IMP_COLORS']['end'] for x in notok])
    if len(ok) >0 or len(notok) > 0:
        print


def _display_build_summary(env):
    print
    print
    print "IMP configuration:"
    s= StringIO.StringIO()
    scons_tools.config_py.write_config(s, env)
    print s.getvalue()
    print
    for x in env.get('IMP_BUILD_SUMMARY', []):
        print x
    d= data.get(env)
    _list_files(env, "dependencies", "#/build/dependencies")

    testmessage=[]
    skipmessage=[]
    all_tests = {}
    for x in env.get('IMP_TESTS', []):
        try:
            module_tests = pickle.load(open(x[1], "r"))
            all_tests[x[0]] = module_tests
        except:
            print "no file", x[1]
            continue
        errors = [t['name'] for t in module_tests \
                  if t['state'] in ('FAIL', 'ERROR', 'UNEXPSUC')]
        skips = [t['name'] for t in module_tests if t['state'] == 'SKIP']
        if len(errors) > 0:
            testmessage.append("  "+x[0]+":")
            testmessage.extend(["    "+n for n in errors])
        if len(skips) > 0:
            skipmessage.append("  "+x[0]+":")
            skipmessage.extend(["    "+n for n in skips])

    if os.path.exists('build/test'):
        pickle.dump(all_tests, open('build/test/test.results', 'w'),
                    protocol=-1)
    if len(testmessage) >0:
        print "Failed tests:"
        print "\n".join([env['IMP_COLORS']['red']+x+env['IMP_COLORS']['end'] for x in testmessage])
        print

    if len(skipmessage) >0:
        print "Skipped tests:"
        print "\n".join([env['IMP_COLORS']['yellow']+x+env['IMP_COLORS']['end'] for x in skipmessage])
        print

    from SCons.Script import GetBuildFailures
    abf=GetBuildFailures()
    buildmessage=[]
    if abf:
        for bf in abf:
            if _bf_to_str(bf).endswith(".results"):
                continue
            buildmessage.append("  "+_bf_to_str(bf))
    if len(buildmessage) > 0:
        print "Errors building:"
        print "\n".join([env['IMP_COLORS']['red']+x+env['IMP_COLORS']['end'] for x in buildmessage])
def setup(env):
    atexit.register(_display_build_summary, env)

def add(env, line):
    env.Append(IMP_BUILD_SUMMARY=line)
