import atexit
import pickle
import glob
import shutil
import os
import sys
import re
from SCons.Script import Dir
import python_coverage
from python_coverage import coverage

def _get_all_python_coverage(covdir):
    cov = coverage.coverage(branch=True,
                            data_file=os.path.join(covdir, '.coverage'))
    python_coverage.setup_excludes(cov)
    cov.file_locator.relative_dir = Dir("#/build").abspath + '/'
    cov.combine()
    morfs = cov.data.lines.keys()
    morfs.sort()
    return cov, morfs

def _get_module_python_coverage(f):
    cov = coverage.coverage(branch=True, data_file=f)
    python_coverage.setup_excludes(cov)
    cov.file_locator.relative_dir = Dir("#/build").abspath + '/'
    cov.load()
    morfs = cov.data.lines.keys()
    morfs.sort()
    return cov, morfs

def _get_module_morfs(covdir, modtype, modname):
    fn = os.path.join(covdir, 'morfs.' + modtype + '.' + modname)
    return pickle.load(open(fn, 'rb'))

def _report_python_module(cov, covdir, morfs, name):
    try:
        cov.html_report(morfs, directory=os.path.join(covdir, 'python', name))
    except coverage.CoverageException, detail:
        print >> sys.stderr, "Python coverage of %s failed: %s" \
                             % (name, str(detail))

def _build_python_coverage(env, single, global_cov):
    covdir = Dir("#/build/coverage").abspath
    # Return if no coverage to report (i.e. no tests were run)
    coverage_files = glob.glob('%s/.coverage*' % covdir)
    if len(coverage_files) == 0:
        return
    print >> sys.stderr, "Generating Python HTML coverage report... ",

    if single:
        cov, morfs = _get_all_python_coverage(covdir)
        cov.html_report(morfs, directory=os.path.join(covdir, 'python'))
    else:
        r = re.compile('\.coverage\.(\w+)\.(\w+)')
        if global_cov:
            cov, morfs = _get_all_python_coverage(covdir)
            for f in coverage_files:
                m = r.search(f)
                mod_morfs = _get_module_morfs(covdir, m.group(1), m.group(2))
                _report_python_module(cov, covdir, mod_morfs, m.group(2))
        else:
            for f in coverage_files:
                m = r.search(f)
                cov, morfs = _get_module_python_coverage(f)
                _report_python_module(cov, covdir, morfs, m.group(2))

    print >> sys.stderr, "Done"

def _build_cpp_coverage(env, single):
    import subprocess
    def call(args):
        r = subprocess.call(args)
        if r != 0:
            raise OSError("%s failed with exit code %d" % (args[0], r))

    covdir = Dir("#/build/coverage").abspath
    # Return if no coverage to report (i.e. no tests were run)
    info_files = glob.glob('%s/*.info' % covdir)
    if len(info_files) == 0:
        return

    cwd = os.getcwd()
    if single:
        call(['genhtml', '--demangle-cpp', '--legend', '-p', cwd,
              '--no-branch-coverage',
              '-o', os.path.join(covdir, 'cpp')] + info_files)
    else:
        r = re.compile('\w+\.(\w+)\.info')
        for f in info_files:
            m = r.search(f)
            call(['genhtml', '--demangle-cpp', '--legend', '-p', cwd,
                  '--no-branch-coverage',
                  '-o', os.path.join(covdir, 'cpp', m.group(1)), f])

    print >> sys.stderr, "Generated C++ HTML coverage report."

def _build_html_coverage(env):
    single = 'single' in env['html_coverage']
    global_cov = 'global' in env['html_coverage']
    if env.get('pycoverage', None):
        _build_python_coverage(env, single, global_cov)
    if env.get('cppcoverage', None):
        _build_cpp_coverage(env, single)

def register(env):
    """Set up HTML coverage"""
    covdir = Dir("#/build/coverage").abspath
    shutil.rmtree(covdir, ignore_errors=True)
    os.makedirs(covdir)
    atexit.register(_build_html_coverage, env)
