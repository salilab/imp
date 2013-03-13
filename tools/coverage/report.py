#!/usr/bin/env python

import coverage
from optparse import OptionParser
import subprocess
import os
import sys
import glob
import pickle
import tools

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

def report_python_component(cov, morfs, name, typ, reldir, outdir):
    if len(morfs) > 0:
        print "Generating HTML report for %s %s Python coverage" % (name, typ)
        sys.stdout.flush()
        cov.file_locator.relative_dir = reldir
        cov.html_report(morfs=morfs, directory=os.path.join(outdir, 'python',
                                                            name))

def report_python_module(cov, modname, outdir):
    mods = glob.glob('lib/IMP/%s/*.py' % modname)
    mods = [x for x in mods if not x.endswith('_version_check.py')]
    report_python_component(cov, mods, modname, 'module', '', outdir)

def report_python_application(cov, app, srcdir, outdir):
    mods = tools.get_glob([os.path.join(srcdir, 'applications', app, '*.py')])
    mods = [os.path.join('bin', os.path.basename(x)) for x in mods]
    report_python_component(cov, mods, app, 'application', '', outdir)

def report_python_dependency(cov, dep, outdir):
    mods = glob.glob('src/dependency/%s/*.py' % dep)
    mods = [x for x in mods if not x.endswith('sitecustomize.py')]
    report_python_component(cov, mods, dep, 'dependency', 'src/dependency/',
                            outdir)

def report_python(opts, outdir):
    srcdir = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]),
                                          '..', '..'))
    def _our_abs_file(self, filename):
        return os.path.normcase(os.path.abspath(filename))
    coverage.files.FileLocator.abs_file = _our_abs_file
    cov = coverage.coverage(data_file='coverage/.coverage')
    setup_excludes(cov)
    cov.load()
    for module in opts.modules:
        report_python_module(cov, module, outdir)
    for app in opts.applications:
        report_python_application(cov, app, srcdir, outdir)
    for dep in opts.dependencies:
        report_python_dependency(cov, dep, outdir)

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

def report_cpp_component(name, typ, matches, excludes, prefix, outdir):
    info_file = 'coverage/%s.%s.info' % (typ, name)
    if extract_lcov('coverage/all.info', info_file, matches, excludes):
        print "Generating HTML report for %s %s C++ coverage" % (name, typ)
        cmd = ['genhtml', '-p', prefix, info_file, '-o',
               os.path.join(outdir, 'cpp', name), '--no-branch-coverage',
               '--legend', '--demangle-cpp']
        print " ".join(cmd)
        sys.stdout.flush()
        subprocess.check_call(cmd)

def report_cpp_module(module, srcdir, outdir):
    report_cpp_component(module, "module", ['/modules/%s/' % module],
                         ['/dependency/'], srcdir, outdir)

def report_cpp_application(app, srcdir, outdir):
    report_cpp_component(app, "application", ['/applications/%s/' % app], [],
                         srcdir, outdir)

def report_cpp_dependency(dep, srcdir, outdir):
    # Currently works only for RMF
    report_cpp_component(dep, "dependency", ['/rmf/dependency/%s/' % dep], [],
                         os.path.join(srcdir, 'modules', 'rmf', 'dependency'),
                         outdir)

def report_cpp(opts, outdir):
    srcdir = os.path.abspath(os.path.join(os.path.dirname(sys.argv[0]),
                                          '..', '..'))
    for module in opts.modules:
        report_cpp_module(module, srcdir, outdir)
    for app in opts.applications:
        report_cpp_application(app, srcdir, outdir)
    for dep in opts.dependencies:
        report_cpp_dependency(dep, srcdir, outdir)

def _get_components(opt, all_comps, exclude):
    if opt is None:
        cs = all_comps
    elif opt == '':
        return []
    else:
        cs = opt.split(":")
    return [x for x in cs if x not in exclude]

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
    parser.add_option("--exclude", metavar='PCK', default=None,
                      help="Don't report coverage for any of the components "
                           "listed in the given summary file (as generated by "
                           "build_all.py).")
    opts, args = parser.parse_args()
    if len(args) != 1:
        parser.error("wrong number of arguments")
    if opts.exclude:
        exclude = pickle.load(open(opts.exclude))
    else:
        exclude = {}
    opts.modules = _get_components(opts.modules, tools.get_sorted_order(),
                                   exclude)
    opts.applications = _get_components(opts.applications,
                                        tools.get_all_configured_applications(),
                                        exclude)
    opts.dependencies = _get_components(opts.dependencies, ['RMF'], exclude)
    return opts, args[0]

def main():
    opts, outdir = parse_args()
    if opts.report in ('both', 'python'):
        report_python(opts, outdir)
    if opts.report in ('both', 'cpp'):
        report_cpp(opts, outdir)

if __name__ == '__main__':
    main()
