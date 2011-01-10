from SCons.Script import Builder, File, Action, Glob, Return, Dir, Move, Copy, Scanner, Chmod
import os.path
import utility
import data

template = """#!/bin/sh

@LDPATH@

@PYTHONPATH@

# Where to find data for the various modules
@MODULEROOTS@

# Extra places to look for imp modules
@MODULEPATH@

@PATH@

@PRECOMMAND@

@TMPDIR@

mkdir -p ${IMP_TMP_DIR}

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
    modules=source[6].get_contents().split(":")
    externmodules=[x for x in source[7].get_contents().split(":") if x != ""]
    externdata=source[8].get_contents()
    if pythonpath == ['']: pythonpath = []
    if ldpath == ['']: ldpath = []
    if path == ['']: path = []

    libdir= os.path.join(root, "build", "lib")
    impdir= os.path.join(libdir, "IMP")
    bindir= os.path.join(root, "build", "bin")
    datadir= os.path.join(root, "build", "data")
    exampledir= os.path.join(root, "build", "doc", "examples")
    tmpdir= os.path.join(root, "build", "tmp")

    varname= utility.get_dylib_name(env)

    if data.get(env).modules['kernel'].external:
        imp_module_path="IMP_MODULE_PATH"
    else:
        imp_module_path=None

    lines={"@LDPATH@":(varname, sep.join([libdir]+ldpath), True),
           "@PYTHONPATH@":("PYTHONPATH", sep.join([libdir]+pythonpath), True),
           "@PATH@":("PATH", sep.join([bindir]+path), True),
           "@PRECOMMAND@":("precommand", precommand, False),
           "@MODULEPATH@":(imp_module_path, impdir, True),
           "@TMPDIR@":("IMP_TMP_DIR", tmpdir, True)}

    for line in template.split('\n'):
        line = line.rstrip('\r\n')

        if lines.has_key(line):
            val= lines[line]
            if val[0] and len(val[1])>0:
                print >> outfile, val[0]+"='"+val[1]+"'"
                if val[2]:
                    print >> outfile, "export", val[0]
        elif line== "@MODULEROOTS@":
            for m in modules:
                varname="IMP_"+m.upper()+"_DATA"
                print >>outfile, varname+"='"+datadir+"'"
                print >>outfile, "export", varname
                varname="IMP_"+m.upper()+"_EXAMPLE_DATA"
                print >>outfile, varname+"='"+exampledir+"'"
                print >>outfile, "export", varname
            for m in externmodules:
                varname="IMP_"+m.upper()+"_DATA"
                print >>outfile, varname+"='"+externdata+"'"
                print >>outfile, "export", varname
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
    if env.get('datapath', None):
        externmodules=[x for x in data.get(env).modules.keys()\
                       if data.get(env).modules[x].external]
        externdata= env.get('datapath', "")
    else:
        externmodules=[]
        externdata=""
    bin = env.ScriptFile(target,
                         [env.Value(template),
                          env.Value(env.Dir('#').abspath),
                          env.Value(env.get('pythonpath', "")),
                          env.Value(utility.get_ld_path(env)),
                          env.Value(prec),
                          env.Value(env['ENV']['PATH']),
                          env.Value(":".join([x for x in data.get(env).modules.keys()\
                                              if not data.get(env).modules[x].external])),
                          env.Value(":".join(externmodules)),
                          env.Value(externdata)])
    return bin
