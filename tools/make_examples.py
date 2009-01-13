import colorize_python
from SCons.Script import Glob, Dir, File, Builder, Action
import SCons.Node.FS
import os

example_path = Dir('#/examples')

def nice_name(string):
    return string.title().replace("_", " ").replace("Cxx", "C++")

def find_examples(top):
    files = Glob("%s/*" % top.abspath)
    subdirs = [x for x in files if isinstance(x, SCons.Node.FS.Dir)]
    examples = [x for x in files if (x.path.endswith('py') \
                or x.path.endswith('cpp'))]

    sublist = []
    dirdescr = file(top.File('README').abspath).read()
    elist=[]
    for ex in examples:
        readme = os.path.splitext(ex.abspath)[0] + ".readme"
        edesc = file(readme).read()
        elist.append((ex, edesc))
    sublist.append((top, dirdescr, elist))
    for x in subdirs:
        sublist.extend(find_examples(x))
    return sublist

def write_doxygen(tree, outputname):
    for d in tree:
        path= d[0].path
        name= os.path.split(path)[1]
        outfile= file(outputname+"/"+name+".dox", 'w')
        outfile.write("/**\n")

        descr= d[1]
        if name == "": wname="Examples"
        else: wname=name
        outfile.write("\page "+wname+ " " + nice_name(wname) +"\n\n")
        outfile.write(descr+"\n\n")
        # find subpages
        for dp in tree:
            pth = dp[0].path
            parent= os.path.split(os.path.split(pth)[0])[1]
            if parent is name and pth is not name:
                outfile.write("\subpage "+os.path.split(pth)[1]+"\n\n")

        #print d
        for e in d[2]:
            pth = e[0].path
            nm= os.path.splitext(os.path.split(pth)[1])[0]
            path=os.path.splitext(pth)[0]
            outfile.write("\section " +nm + " " + nice_name(nm)+"\n\n")
            outfile.write(e[1]+"\n\n")
            if pth.endswith('py'):
                outfile.write("\htmlonly\n")
                colorize_python.Parser(file(pth).read(),
                                       outfile).format(None, None)
                outfile.write("\endhtmlonly\n\n")
            else:
                outfile.write("\include "+nm+".cpp\n\n")
        outfile.write("*/\n")


def _action_make_examples(target, source, env):
    tree = find_examples(example_path)
    write_doxygen(tree, os.path.dirname(target[0].path))

def _emit_make_examples(target, source, env):
    tree = find_examples(example_path)
    dir = Dir(os.path.dirname(target[0].abspath))
    source = []
    for d in tree:
        # Only add Python files as sources, since they are directly
        # incorporated into the .dox files (not .cpp files, which are
        # \include'd, and thus don't change the .dox files themselves)
        source.extend([x[0] for x in d[2] if x[0].path.endswith('.py')])
    target = [dir.File(os.path.split(x[0].path)[1] + '.dox') for x in tree]
    return (target, source)

def _print_make_examples(target, source, env):
    print "Generating doxygen pages for examples"

MakeExamples = Builder(action=Action(_action_make_examples,
                                     _print_make_examples),
                       emitter=_emit_make_examples)
