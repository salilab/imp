from SCons.Script import Glob, Dir, File, Builder, Action
import _handle_py
import os
import scons_tools.data
import scons_tools.utility




def _write_doxygen(env, name, link, files, outputname):
    #print "writing "+outputname
    outfile= file(outputname, 'w')
    outfile.write("/**\n")
    outfile.write("\page "+link+ " " + name+"\n\n")
    seen=[]
    for f in files:
        if str(f).endswith(".py") or str(f).endswith(".cpp"):
            readme=os.path.splitext(f.abspath)[0]+".readme"
            seen.append(readme)
            rm= open(readme, "r").read()
            nm= os.path.splitext(os.path.split(str(f))[1])[0]
            outfile.write("\section " +nm + " " + nm.replace('_', ' ')+"\n\n")
            outfile.write(rm+"\n\n")
        if str(f).endswith(".py"):
            outfile.write("\pythonexample{"+ nm+"}\n\n")
        if str(f).endswith(".cpp"):
            outfile.write("\include "+nm+".cpp\n\n")
        if str(f).endswith(".readme") and str(f) not in seen:
            rm= open(os.path.splitext(f.abspath)[0]+".readme", "r").read()
            outfile.write(rm+"\n\n")
    outfile.write("*/\n")

def _action_make_examples(target, source, env):
    print source[0].get_contents(), target[0].path
    _write_doxygen(env, source[0].get_contents(), source[1].get_contents(),
                  source[2:], target[0].path)

def _print_make_examples(target, source, env):
    print "Generating doxygen page for examples"

_Page= Builder(action=Action(_action_make_examples,
                                _print_make_examples))


def add_page(env, name, files):
    #print "adding page", name, "for", [str(x) for x in files]
    linkname= scons_tools.utility.get_link_name_from_name(name)
    link= scons_tools.utility.get_link_from_name(name)
    t=File('generated/'+linkname+'.dox')
    page= _Page(env, source=[env.Value(name),
                             env.Value(linkname)]+files,
                target=[t])
    return link


def add_python_example(env, file):
    scons_tools.data.get(env).add_example(File(file))
    p=_handle_py.Process(source=file, target=File(str(file)+".parsed"),
                       env=env)
    env.AlwaysBuild(p)
