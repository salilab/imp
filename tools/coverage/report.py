#!/usr/bin/env python

import coverage
from optparse import OptionParser
import subprocess
import pickle
import os
import sys
import glob
import tools

class LogOutput(object):
    def __init__(self, summary, outdir, typ):
        self.outdir = outdir
        self.typ = typ
        if summary:
            self.other_modules = pickle.load(open(summary[0])).keys()
            self.other_logdir = summary[1]
        else:
            self.other_modules = {}

    def get_dir(self, name):
        if name in self.other_modules:
            return os.path.join(self.other_logdir, self.typ, name)
        else:
            return os.path.join(self.outdir, self.typ, name)


def setup_excludes(cov):
    # Try to exclude SWIG and IMP boilerplate from coverage checks
    cov.exclude("def swig_import_helper\(")
    cov.exclude("def _swig_")
    cov.exclude("class (_ostream|_DirectorObjects|"
           "IMP_\w+_SwigPyIterator)\(")
    cov.exclude("^\s+import _IMP_")
    cov.exclude("^except (Name|Attribute)Error:")
    cov.exclude("^\s+weakref_proxy =")
    cov.exclude("^except:\s*$")
    cov.exclude("^def [sg]et_check_level")
    cov.exclude("^\s+__setattr__ = lambda self, name, value: _swig_setattr")
    cov.exclude("^\s+__getattr__ = lambda self, name: _swig_getattr")
    cov.exclude("^\s+__swig_[sg]etmethods__\[\".*\"\] = lambda ")

def report_python_component(cov, morfs, name, typ, reldir, out):
    if len(morfs) > 0:
        print "Generating HTML report for %s %s Python coverage" % (name, typ)
        cov.file_locator.relative_dir = reldir
        cov.html_report(morfs=morfs, directory=out.get_dir(name))

def report_python_module(cov, modname, out):
    mods = glob.glob('lib/IMP/%s/*.py' % modname)
    mods = [x for x in mods if not x.endswith('_version_check.py')]
    report_python_component(cov, mods, modname, 'module', '', out)

def report_python_application(cov, app, srcdir, out):
    mods = tools.get_glob([os.path.join(srcdir, 'applications', app, '*.py')])
    mods = [os.path.join('bin', os.path.basename(x)) for x in mods]
    report_python_component(cov, mods, app, 'application', '', out)

def report_python_dependency(cov, dep, out):
    mods = glob.glob('src/dependency/%s/*.py' % dep)
    mods = [x for x in mods if not x.endswith('sitecustomize.py')]
    report_python_component(cov, mods, dep, 'dependency', 'src/dependency/',
                            out)

def report_python(opts, outdir):
    out = LogOutput(opts.summary, outdir, 'python')
    srcdir = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]),
                                          '..', '..'))
    def _our_abs_file(self, filename):
        return os.path.normcase(os.path.abspath(filename))
    coverage.files.FileLocator.abs_file = _our_abs_file
    cov = coverage.coverage(data_file='coverage/.coverage')
    setup_excludes(cov)
    cov.load()
    for module in opts.modules:
        report_python_module(cov, module, out)
    for app in opts.applications:
        report_python_application(cov, app, srcdir, out)
    for dep in opts.dependencies:
        report_python_dependency(cov, dep, out)

def extract_lcov(infile, outfile, matches, excludes):
    """Extract a subset from an lcov .info file.
       This is similar to lcov -e, except that it is significantly faster
       and we map symlinks back into the IMP source tree."""
    def filter_filename(filename):
        for e in excludes:
            if e in filename:
                return False
        for m in matches:
            if m in filename:
                return True
    fin = open(infile)
    fout = open(outfile, 'w')
    record = []
    write_record = False
    lines_written = False
    for line in fin:
        if line.startswith('SF:'):
            filename = line.rstrip('\r\n')[3:]
            if os.path.islink(filename):
                filename = os.readlink(filename)
                line = 'SF:' + filename + '\n'
            write_record = filter_filename(filename)
        if not line.startswith('BRDA:'):
            # We don't report branch information, so exclude this from the
            # .info file so genhtml runs faster
            record.append(line)
        if line.startswith('end_of_record'):
            if write_record:
                fout.writelines(record)
                lines_written = True
            record = []
    fin.close()
    fout.close()
    # lcov falls over if given a 0-byte .info file
    if not lines_written:
        os.unlink(outfile)
    return lines_written

def report_cpp_component(name, typ, matches, excludes, prefix, out):
    info_file = 'coverage/%s.%s.info' % (typ, name)
    if extract_lcov('coverage/all.info', info_file, matches, excludes):
        print "Generating HTML report for %s %s C++ coverage" % (name, typ)
        cmd = ['genhtml', '-p', prefix, info_file, '-o',
               out.get_dir(name), '--no-branch-coverage',
               '--legend', '--demangle-cpp']
        print " ".join(cmd)
        subprocess.check_call(cmd)

def report_cpp_module(module, srcdir, out):
    report_cpp_component(module, "module", ['/modules/%s/' % module],
                         ['/dependency/'], srcdir, out)

def report_cpp_application(app, srcdir, out):
    report_cpp_component(app, "application", ['/applications/%s/' % app], [],
                         srcdir, out)

def report_cpp_dependency(dep, srcdir, out):
    # Currently works only for RMF
    report_cpp_component(dep, "dependency", ['/rmf/dependency/%s/' % dep], [],
                         os.path.join(srcdir, 'modules', 'rmf', 'dependency'),
                         out)

def report_cpp(opts, outdir):
    out = LogOutput(opts.summary, outdir, 'cpp')
    srcdir = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]),
                                          '..', '..'))
    for module in opts.modules:
        report_cpp_module(module, srcdir, out)
    for app in opts.applications:
        report_cpp_application(app, srcdir, out)
    for dep in opts.dependencies:
        report_cpp_dependency(dep, srcdir, out)

def _get_components(opt, all_comps):
    if opt is None:
        return all_comps
    elif opt == '':
        return []
    else:
        return opt.split(":")

def parse_args():
    parser = OptionParser(usage="""%prog [options] outdir

Generate HTML coverage reports for IMP C++/Python code in the given directory.
""")
    parser.add_option("--report", type="choice",
                      choices=['python', 'cpp', 'both'], default='both',
                      help="Generate reports for Python code ('python'), "
                           "C++ code ('cpp') or both Python and C++ ('both'). "
                           "Default 'both'.")
    parser.add_option("--modules", metavar='STR', default=None,
                      help="Report only for the given colon-separated list of "
                           "IMP modules, e.g. 'base:kernel'. By default, "
                           "coverage for all modules is reported.")
    parser.add_option("--applications", metavar='STR', default=None,
                      help="Report only for the given colon-separated list of "
                           "IMP applications, e.g. 'foxs:saxs_merge'. By "
                           "default, coverage for all applications is "
                           "reported.")
    parser.add_option("--dependencies", metavar='STR', default=None,
                      help="Report only for the given colon-separated list of "
                           "IMP dependencies, e.g. 'RMF'. By default, coverage "
                           "for all supported dependencies (currently only "
                           "RMF) is reported.")
    parser.add_option("--summary", nargs=2, default=None, metavar="PCK DIR",
                      help="Takes two (space-separated) arguments. Any "
                           "component named in the first argument (a Python "
                           "pickle, as output by build_all.py) will have its "
                           "coverage report written to the directory named by "
                           "the second argument.")
    opts, args = parser.parse_args()
    if len(args) != 1:
        parser.error("wrong number of arguments")
    opts.modules = _get_components(opts.modules, tools.get_sorted_order())
    opts.applications = _get_components(opts.applications,
                                        tools.get_all_configured_applications())
    opts.dependencies = _get_components(opts.dependencies, ['RMF'])
    return opts, args[0]

def main():
    opts, outdir = parse_args()
    if opts.report in ('both', 'python'):
        report_python(opts, outdir)
    if opts.report in ('both', 'cpp'):
        report_cpp(opts, outdir)

if __name__ == '__main__':
    main()
