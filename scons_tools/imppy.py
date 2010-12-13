from SCons.Script import Builder, File, Action, Glob, Return, Dir, Move, Copy, Scanner, Chmod


template = """#!/bin/sh

@LDPATH@

@PYTHONPATH@

@BUILDROOT@

@PATH@

@PRECOMMAND@

mkdir -p ${IMP_BUILD_ROOT}/build/tmp

exec ${precommand} "$@"
"""



# Custom builder to generate file:
def builder_script_file(target, source, env):
    outfile = file(target[0].abspath, 'w')

    sep=":"
    varname=None
    if env['PLATFORM'] == 'posix' or env['PLATFORM']=='sunos':
        varname= "LD_LIBRARY_PATH"
    elif env['PLATFORM'] == 'darwin':
        varname= "DYLD_LIBRARY_PATH"
    elif env['PLATFORM'] == 'win32' and not env['wine']:
        sep=";"

    for line in source[0].get_contents().split('\n'):
        line = line.rstrip('\r\n')
        if line == "@LDPATH@":
            if varname:
                libs= sep+source[5].get_contents()
                if source[4].get_contents() != "":
                    libs= libs+sep+source[4].get_contents()
                line = varname + "='"+source[1].get_contents()+"/build/lib"+\
                    libs+"'\nexport " + varname
            else:
                line=""
        if line == "@PYTHONPATH@":
            pl=[source[1].get_contents()+"/build/lib"]
            if source[2].get_contents() != "":
                pl.append(source[2].get_contents().replace(":", sep))
            line= "PYTHONPATH='"+\
                sep.join(pl)+\
                          "'\nexport PYTHONPATH"
        if line == "@PATH@":
            pl=[source[1].get_contents()+"/build/bin"]
            if source[7].get_contents() != "":
                pl.append(source[7].get_contents().replace(":", sep))
            line= "PATH='"+\
                sep.join(pl)+\
                          "'\nexport PATH"
        if line == "@BUILDROOT@":
            line= "IMP_BUILD_ROOT='%s'" \
                  % source[1].get_contents() + "\nexport IMP_BUILD_ROOT"
        if line == "@PRECOMMAND@":
            line="precommand='"
            if len(source[3].get_contents())!=0:
                line = line + source[3].get_contents()+" "
            if len(source[6].get_contents()) != 0:
                line = line+ source[6].get_contents()
            line=line+"'"
        print >> outfile, line
    outfile.close()
    env.Execute(Chmod(str(target[0]), 0755))

def add(env, target):
    env.Append(BUILDERS = {'ScriptFile': Builder(action=builder_script_file)})
    libpath=""
    ldpath=""
    prec=""
    if not env['IMP_USE_RPATH'] and env.get('ldlibpath', None):
        libpath=":".join(env['libpath'])
    if env.get('ldlibpath', None):
        ldpath= env.get('ldlibpath', "")
    prec=""
    if env.get('precommand', None):
        prec= env['precommand']
    bin = env.ScriptFile(target,
                         [env.Value(template), env.Value(env.Dir('#').abspath),
                          env.Value(env.get('pythonpath', "")),
                          env.Value(env['MODELLER_MODPY']),
                          env.Value(libpath),
                          env.Value(ldpath),
                                        env.Value(prec),
                          env.Value(env['ENV']['PATH'])])
    return bin
