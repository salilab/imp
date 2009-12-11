from SCons.Script import Glob, Dir, File, Builder, Action

def _action_run(target, source, env):
    ret=True
    for a in source[1:]:
        app = "%s %s" \
            % (source[0].abspath, a.abspath)
        if not env.Execute(app):
            ret=False
    if ret:
        return 1
    else:
        return 0


def _print_run(target, source, env):
    print "Running "+ str(source[1])

Run = Builder(action=Action(_action_run,
                            _print_run))
