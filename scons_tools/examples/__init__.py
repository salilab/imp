from SCons.Script import Glob, Dir, File, Builder, Action
import _handle_py
import os
import scons_tools.data
import scons_tools.utility




def _write_doxygen(env, name, link, files, outputname):
    #print "writing "+outputname
    outfile= file(outputname, 'w')
    outfile.write("/**\n")
    outfile.write("\\page "+link+ " " + name+"\n\n")
    dta= scons_tools.data.get(env)
    for f in files:
        ln=scons_tools.utility.get_link_name_from_name(f)
        if ln in dta.examples.keys():
            readme=dta.examples[ln].overview
            outfile.write("\\section " +ln + " " +\
                          scons_tools.utility.get_display_from_name(f)+"\n\n")
            outfile.write(readme+"\n\n")
            outfile.write("""<table border="1">
<tr><td>\n""")
            nm=scons_tools.utility.get_without_extension(str(dta.examples[ln].file))
            outfile.write("\\pythonexample{"+ nm+"}\n\n")
            outfile.write("</td><td>\n")
            #print "for example",f,dta.examples[ln].methods, dta.examples[ln].classes
            for m in dta.examples[ln].classes.keys():
                if m=='kernel':
                    ns="IMP::"
                else:
                    ns="IMP::"+m+"::"
                for c in dta.examples[ln].classes[m]:
                    outfile.write(" - "+ns+c+"\n")
            for m in dta.examples[ln].methods.keys():
                if m=='kernel':
                    ns="IMP::"
                else:
                    ns="IMP::"+m+"::"
                for c in dta.examples[ln].methods[m]:
                    outfile.write(" - "+ns+c+"\n")
            outfile.write("</td></tr></table>\n")
        elif str(f).endswith(".py"):
            outfile.write("\\pythonexample{"+ nm+"}\n\n")
        elif str(f).endswith(".cpp"):
            outfile.write("\include "+nm+".cpp\n\n")
        elif str(f).endswith(".readme") and str(f) not in seen:
            rm= open(os.path.splitext(f.abspath)[0]+".readme", "r").read()
            outfile.write(rm+"\n\n")
    outfile.write("*/\n")

def _action_make_examples(target, source, env):
    #print source[0].get_contents(), target[0].path
    _write_doxygen(env, source[0].get_contents(), source[1].get_contents(),
                  source[2:], target[0].path)

def _print_make_examples(target, source, env):
    print "Generating doxygen page for examples", str(source[0])

_Page= Builder(action=Action(_action_make_examples,
                                _print_make_examples))


def add_page(env, name, files):
    #print "adding page", name, "for", [str(x) for x in files]
    linkname= scons_tools.utility.get_link_name_from_name(name)
    link= scons_tools.utility.get_link_from_name(name)
    t=File('generated/'+linkname+'.dox')
    parsed=[]
    for f in files:
        if str(f).endswith(".py"):
            parsed.append(str(f)+".parsed")
    page= _Page(env, source=[env.Value(name),
                             env.Value(linkname)]+files+parsed,
                target=[t])
    return link


def add_python_example(env, file, overview):
    #print "Adding", file, overview
    scons_tools.data.get(env).add_example(File(file), overview)
    p=_handle_py.Process(source=file, target=File(str(file)+".parsed"),
                       env=env)
    env.AlwaysBuild(p)
