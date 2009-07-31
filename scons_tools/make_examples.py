import colorize_python
from SCons.Script import Glob, Dir, File, Builder, Action
import SCons.Node.FS
import os

def nice_name(string):
    return string.title().replace("_", " ").replace("Cxx", "C++")

def find_examples(examples):
    print [str(x) for x in examples]
    pyfiles={}
    cppfiles={}
    readmes={}
    for x in examples:
        (dir, name)= os.path.split(str(x))
        #print "Name is "+str(x)
        #print "dir is " +dir
        if name == "README":
            c= open(x.abspath).read()
            pyfiles[dir]=[]
            cppfiles[dir]=[]
            readmes[dir]=c
    for x in examples:
        (dir, name)= os.path.split(str(x))
        if name != "README":
            (prefix, ext)= os.path.splitext(name)
            rm= open(os.path.join(dir, prefix+".readme")).read()
            if ext == '.cpp':
                cppfiles[dir].append((prefix, rm, str(x)))
            elif ext == '.py':
                pyfiles[dir].append((prefix, rm, str(x)))
            else:
                raise ValueError(ext)
    return (readmes, cppfiles, pyfiles)

def write_doxygen(readmes, cpps, pys, outputname):
    for path in readmes.keys():
        name= os.path.split(path)[1]
        outfile= file(outputname+"/"+name+".dox", 'w')
        outfile.write("/**\n")
        #print "working on " +path
        if name == "examples": wname="Examples"
        else: wname=name
        outfile.write("\page "+wname+ " " + nice_name(wname) +"\n\n")
        outfile.write(readmes[path]+"\n\n")
        # find subpages
        for pth in readmes.keys():
            if os.path.split(pth)[0] == path:
                outfile.write("\subpage "+os.path.split(pth)[1]+"\n\n")

        #print d
        for e in cpps[path]:
            #print "   cpp is " + e[2]
            pth = e[2]
            nm= e[0]
            outfile.write("\section " +nm + " " + nice_name(nm)+"\n\n")
            outfile.write(e[1]+"\n\n")
            outfile.write("\include "+nm+".cpp\n\n")
        for e in pys[path]:
            #print "   py is " + e[2]
            pth = e[2]
            nm= e[0]
            outfile.write("\section " +nm + " " + nice_name(nm)+"\n\n")
            outfile.write(e[1]+"\n\n")
            outfile.write("\htmlonly\n")
            colorize_python.Parser(file(pth).read(),
                                   outfile).format(None, None)
            outfile.write("\endhtmlonly\n\n")

        outfile.write("*/\n")


def _action_make_examples(target, source, env):
    (readmes, cpps, pys) = find_examples(source)
    write_doxygen(readmes, cpps, pys, os.path.dirname(target[0].path))


def _print_make_examples(target, source, env):
    print "Generating doxygen pages for examples"

MakeExamples = Builder(action=Action(_action_make_examples,
                                     _print_make_examples))
