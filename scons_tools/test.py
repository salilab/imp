from SCons.Script import Glob, Dir, File, Builder, Action
import pyscanner



def _action_unit_test(target, source, env):
    #app = "cd %s; %s %s %s -v > /dev/null"
    fsource=[]
    for x in source[2:]:
        if str(x).endswith(".py"):
            fsource.append(x.abspath)
    app = "%s %s %s %s > /dev/null" \
          % (source[0].abspath, env['PYTHON'],
             source[1].abspath,
             " ".join(fsource))
    if env.Execute(app) == 0:
        file(str(target[0]), 'w').write('PASSED\n')
    else:
        print "IMP.%s unit tests FAILED" % env['IMP_MODULE']
        return 1


def _print_unit_test(target, source, env):
    print "Generating unit testing"

UnitTest = Builder(action=Action(_action_unit_test,
                                _print_unit_test),
                   source_scanner=pyscanner.PythonScanner)
