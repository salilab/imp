from SCons.Script import Glob, Dir, File, Builder, Action
import os
import scons_tools.data
import scons_tools.utility
import scons_tools.environment
import scons_tools.paths


def _write_doxygen(env, name, fnode, overview, outputname):
    #print "writing "+outputname
    outfile= file(outputname, 'w')
    outfile.write("/**\n")
    outfile.write("\\example "+ name+"\n\n")
    outfile.write(file(overview.abspath).read())
    outfile.write("*/\n")

def _action_make_example(target, source, env):
    #print source[0].get_contents(), target[0].path
    _write_doxygen(env, source[0].get_contents(),
                  source[1], source[2], target[0].path)

def _print_make_example(target, source, env):
    print "Generating doxygen page for example", str(target[0])

_Page= Builder(action=Action(_action_make_example,
                                _print_make_example))


def add_python_example(env, file, overview):
    #print "Adding", file, overview
    module= scons_tools.environment.get_current_name(env)
    ff=File(file)
    path=ff.path[ff.path.find('/')+1:]
    _Page(source=[env.Value(path), ff, File(overview)],
          target=File(scons_tools.paths.get_output_path_suffix(env, ff, ".dox",
                                                               Dir("#/build/doxygen/"+module))),
          env=env)
    return "\\ref "+path+ " \""+path+"\""
