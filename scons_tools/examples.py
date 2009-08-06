import colorize_python
from SCons.Script import Glob, Dir, File, Builder, Action
import SCons.Node.FS
import os
import os.path


def _action_colorize_python(target, source, env):
    colorize_python.Parser(open(str(source[0]), "r").read(),
                           open(str(target[0]), "w")).format(None, None)

def _print_colorize_python(target, source, env):
    print "Generating doxygen pages for examples"

ColorizePython = Builder(action=Action(_action_colorize_python,
                                       _print_colorize_python))


def nice_name(string):
    return string.title().replace("_", " ").replace("Cxx", "C++")

def write_doxygen(env, name, files, outputname):
    #print "writing "+outputname
    outfile= file(outputname, 'w')
    outfile.write("/**\n")
    outfile.write("\page "+name.replace(".", "_")+"_examples"+ " " + name+" examples\n\n")
    for f in files:
        if str(f).endswith(".py") or str(f).endswith(".cpp"):
            rm= open(os.path.splitext(f.abspath)[0]+".readme", "r").read()
            nm= os.path.splitext(os.path.split(str(f))[1])[0]
            outfile.write("\section " +nm + " " + nice_name(nm)+"\n\n")
            outfile.write(rm+"\n\n")
        if str(f).endswith(".py"):
            outfile.write("\htmlinclude "+ nm+".py.html\n\n")
        if str(f).endswith(".cpp"):
            outfile.write("\include "+nm+".cpp\n\n")

    outfile.write("*/\n")


def _action_make_examples(target, source, env):
    name= env['IMP_MODULE_NICENAME']
    write_doxygen(env, name, source, target[0].path)

def _print_make_examples(target, source, env):
    print "Generating doxygen page for examples"

MakeDox = Builder(action=Action(_action_make_examples,
                                _print_make_examples))



def handle_example_dir(env, dirpath, name, includepath, files):
    build=[]
    install=[]
    exampledir = env.GetInstallDirectory('docdir')+"/examples"
    split= os.path.split(includepath)
    prefix=split[1]
    if len(split[0])==0:
        prefix=""
    for f in files:
        if str(f).endswith(".py"):
            c= env._IMPColorizePython(str(f)+".html", f.abspath)
            build.append(c)
            install.append(env.Install(exampledir+"/"+prefix, f.abspath))
        elif str(f).endswith(".readme"):
            install.append(env.Install(exampledir+"/"+prefix, f.abspath))
    test= env._IMPModuleTest('tests.passed', ["#/bin/imppy.sh", "#/tools/run-all-tests.py"]\
                                 +[x for x in files if str(x).endswith(".py") and str(x).find("fragment")==-1])
    env.AlwaysBuild("test.passed")
    doxpage= env._IMPExamplesDox(File(str(dirpath)+"/examples.dox"), files)
    build.append(doxpage)
    return (build, install, test)
