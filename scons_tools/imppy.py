from SCons.Script import Builder, File, Action, Glob, Return, Dir, Move, Copy, Scanner, Chmod
import os.path
import utility

template = """#!/bin/sh

@LDPATH@

@PYTHONPATH@

@BUILDROOT@

@MODULEPATH@

@PATH@

@PRECOMMAND@

mkdir -p ${IMP_BUILD_ROOT}/build/tmp

exec ${precommand} "$@"
"""



# Custom builder to generate file:
def builder_script_file(target, source, env):
    sep= utility.get_separator(env)

    outfile = file(target[0].abspath, 'w')
    template= source[0].get_contents()
    root=source[1].get_contents()
    pythonpath=source[2].get_contents().split(sep)
    ldpath=source[3].get_contents().split(sep)
    precommand=source[4].get_contents()
    path=source[5].get_contents().split(sep)

    libdir= os.path.join(root, "build", "lib")
    impdir= os.path.join(libdir, "IMP")
    bindir= os.path.join(root, "build", "bin")

    varname= utility.get_dylib_name(env)

    lines={"@LDPATH@":(varname, sep.join([libdir]+ldpath), True),
           "@PYTHONPATH@":("PYTHONPATH", sep.join([libdir]+pythonpath), True),
           "@PATH@":("PATH", sep.join([bindir]+path), True),
           "@BUILDROOT@":("IMP_BUILD_ROOT", root, True),
           "@PRECOMMAND@":("precommand", precommand, False),
           "@MODULEPATH@":("IMP_MODULE_PATH", impdir, True)}

    for line in template.split('\n'):
        line = line.rstrip('\r\n')

        if lines.has_key(line):
            val= lines[line]
            if val[0] and len(val[1])>0:
                print >> outfile, val[0]+"="+val[1]
                if val[2]:
                    print >> outfile, "export", val[0]
        else:
            print >> outfile, line
    outfile.close()
    env.Execute(Chmod(str(target[0]), 0755))

def add(env, target):
    env.Append(BUILDERS = {'ScriptFile': Builder(action=builder_script_file)})
    prec=""
    if env.get('precommand', None):
        prec= env['precommand']
    if env.get('MODELLER_MODPY', None):
        prec=prec+" "+env['MODELLER_MODPY']
    bin = env.ScriptFile(target,
                         [env.Value(template),
                          env.Value(env.Dir('#').abspath),
                          env.Value(env.get('pythonpath', "")),
                          env.Value(utility.get_ld_path(env)),
                          env.Value(prec),
                          env.Value(env['ENV']['PATH'])])
    return bin
