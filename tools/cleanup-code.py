#!/usr/bin/env python
import glob
from optparse import OptionParser
import subprocess
import fnmatch
import os
import multiprocessing
from Queue import Queue
from threading import Thread

parser = OptionParser()
parser.add_option("-c", "--clang-format", dest="clang_format",
                  default="clang-format-3.4",
                  help="The clang-format command.")
parser.add_option("-a", "--autopep8", dest="autopep8",
                  default="autopep8",
                  help="The autopep8 command.")
(options, args) = parser.parse_args()

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
                print e
                error = str(e)
            self.tasks.task_done()


class ThreadPool:

    """Pool of threads consuming tasks from a queue"""

    def __init__(self, num_threads=-1):
        if num_threads == -1:
            num_threads = 2 * multiprocessing.cpu_count()
        print "Creating thread pool with", num_threads
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


def _rewrite(filename, contents):
    old = open(filename, "r").read()
    #contents = open(tempfile, "r").read()
    if old != contents:
        open(filename, "w").write(contents)
        # os.unlink(tempfile)


def _do_get_files(glb, cur):
    matches = []
    dirs = []
    for n in os.listdir(cur):
        if n == "eigen3" or n == "config_templates":
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
                           stdout=subprocess.PIPE)
    output, error = pro.communicate()
    if pro.returncode != 0:
        print " ".join(cmd)
        raise RuntimeError("error running " + error)
    return output


def clean_cpp(path):
    contents = _run([options.clang_format, "--style=Google", path])
    contents = contents.replace("% template", "%template")
    _rewrite(path, contents)


def clean_py(path):
    contents = _run([options.autopep8, "--aggressive", "--aggressive",
                     "--ignore=E24,W602", path])
    if contents.find("# \\example") != -1:
        contents = "#" + contents
    _rewrite(path, contents)


def main():
    tp = ThreadPool()

    for f in _get_files(".py"):
        # print f
        tp.add_task(clean_py, f)

    for f in _get_files(".h") + _get_files(".cpp"):
        # print f
        tp.add_task(clean_cpp, f)
    tp.wait_completion()

if __name__ == '__main__':
    main()
