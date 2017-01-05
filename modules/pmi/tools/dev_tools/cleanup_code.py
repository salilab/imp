#!/usr/bin/env python

"""Use clang-format and autopep8 when available to clean up the listed source
   files."""

from __future__ import print_function
import glob
from optparse import OptionParser
import subprocess
import fnmatch
import os
import sys
import multiprocessing
try:
    from queue import Queue # python3
except ImportError:
    from Queue import Queue # python2
from threading import Thread
import distutils.spawn

sys.path.append(os.path.split(sys.argv[0]))
import python_tools
from python_tools.reindent import Reindenter


parser = OptionParser(usage="%prog [options] [FILENAME ...]",
               description="""Reformat the given C++ and Python files
(using the clang-format tool if available and
reindent.py, respectively). If the --all option is given, reformat all such
files under the current directory.

If the autopep8 tool is also available, it can be used instead of reindent.py
by giving the -a option. autopep8 is much more aggressive than reindent.py
and will fix other issues, such as use of old-style Python syntax. 
""")
parser.add_option("-c", "--clang-format", dest="clang_format",
                  default="auto", metavar="EXE",
                  help="The clang-format command.")
parser.add_option("-a", dest="use_ap", action="store_true", default=False,
                  help="Use autopep8 rather than reindent.py for Python files.")
parser.add_option("--all", dest="all_files", action="store_true", default=False,
                  help="Reformat all files under current directory")
parser.add_option("--autopep8", dest="autopep8",
                  default="auto", metavar="EXE",
                  help="The autopep8 command.")
parser.add_option("-e", "--exclude", dest="exclude",
                  default="eigen3:config_templates", metavar="DIRS",
                  help="Colon-separated list of dirnames to ignore.")
parser.add_option("-v", "--verbose", dest="verbose", action="store_true",
                  default=False,
                  help="Print extra info.")
(options, args) = parser.parse_args()
if not args and not options.all_files:
    parser.error("No files selected")

# clang-format-3.4",
# autopep8

# search for executables
if options.clang_format == "auto":
    options.clang_format = None
    for name in ["clang-format-3.4", "clang-format"]:
        if distutils.spawn.find_executable(name):
            options.clang_format = name
            break
if options.autopep8 == "auto":
    options.autopep8 = None
    for name in ["autopep8"]:
        if distutils.spawn.find_executable(name):
            options.autopep8 = name
            break

exclude = options.exclude.split(":")

error = None


class _Worker(Thread):

    """Thread executing tasks from a given tasks queue"""

    def __init__(self, tasks):
        Thread.__init__(self)
        self.tasks = tasks
        self.daemon = True
        self.start()

    def run(self):
        while True:
            func, args, kargs = self.tasks.get()
            try:
                func(*args, **kargs)
            except Exception as e:
                print(e)
                error = str(e)
            self.tasks.task_done()


class ThreadPool:

    """Pool of threads consuming tasks from a queue"""

    def __init__(self, num_threads=-1):
        if num_threads == -1:
            num_threads = 2 * multiprocessing.cpu_count()
            # print "Creating thread pool with", num_threads
        self.tasks = Queue(-1)
        for _ in range(num_threads):
            _Worker(self.tasks)

    def add_task(self, func, *args, **kargs):
        """Add a task to the queue"""
        #func(*args, **kargs)
        self.tasks.put((func, args, kargs))

    def wait_completion(self):
        """Wait for completion of all the tasks in the queue"""
        self.tasks.join()
        return error


def _do_get_files(glb, cur):
    matches = []
    dirs = []
    for n in os.listdir(cur):
        if n in exclude:
            continue
        name = os.path.join(cur, n)
        if os.path.isdir(name):
            if not os.path.exists(os.path.join(name, ".git")):
                matches += _do_get_files(glb, name)
        elif name.endswith(glb):
            matches.append(name)
    return matches


def _get_files(glb):
    match = []
    if len(args) == 0:
        match = _do_get_files(glb, ".")
    else:
        for a in args:
            if os.path.isdir(a):
                match += _do_get_files(glb, a)
            elif a.endswith(glb):
                match.append(a)
    return match


def _run(cmd):
    # print " ".join(cmd)
    pro = subprocess.Popen(cmd, stderr=subprocess.PIPE,
                           stdout=subprocess.PIPE, universal_newlines=True)
    output, error = pro.communicate()
    if pro.returncode != 0:
        print(" ".join(cmd))
        raise RuntimeError("error running " + error)
    return output


def clean_cpp(path):
    # skip code that isn't ours
    if "dependency" in path or "/eigen3/" in path:
        return
    if options.clang_format:
        contents = _run([options.clang_format, "--style=Google", path])
    else:
        contents = open(path, "r").read()
    contents = contents.replace("% template", "%template")
    python_tools.rewrite(path, contents)


def clean_py(path):
    if options.use_ap and options.autopep8:
        contents = _run([options.autopep8, "--aggressive", "--aggressive",
                         path])
    else:
        r = Reindenter(open(path))
        r.run()
        contents = ''.join(r.after)
    if contents.find("# \\example") != -1:
        contents = "#" + contents
    python_tools.rewrite(path, contents)


def main():
    if options.verbose:
        if options.autopep8 is None:
            print("autopep8 not found")
        else:
            print("autopep8 is `%s`" % options.autopep8)
        if options.clang_format is None:
            print("clang-format not found")
        else:
            print("clang-format is `%s`" % options.clang_format)

    tp = ThreadPool()

    if args:
        for f in args:
            if f.endswith(".py"):
                tp.add_task(clean_py, f)
            elif f.endswith(".h") or f.endswith(".cpp"):
                tp.add_task(clean_cpp, f)
    elif options.all_files:
        for f in _get_files(".py"):
            tp.add_task(clean_py, f)
        for f in _get_files(".h") + _get_files(".cpp"):
            tp.add_task(clean_cpp, f)
    tp.wait_completion()

if __name__ == '__main__':
    main()
