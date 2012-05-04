import atexit
import glob
import shutil
import os
import sys
import re
from SCons.Script import Dir
import python_coverage
from python_coverage import coverage

def _build_python_coverage(env, single):
    covdir = Dir(env["builddir"]+"/coverage").abspath
    # Return if no coverage to report (i.e. no tests were run)
    coverage_files = glob.glob('%s/.coverage*' % covdir)
    if len(coverage_files) == 0:
        return
    print >> sys.stderr, "Generating Python HTML coverage report... ",

    if single:
        cov = coverage.coverage(branch=True,
                                data_file=os.path.join(covdir, '.coverage'))
        python_coverage.setup_excludes(cov)
        cov.file_locator.relative_dir = Dir(env["builddir"]).abspath + '/'
        cov.combine()
        morfs = cov.data.lines.keys()
        morfs.sort()
        cov.html_report(morfs, directory=os.path.join(covdir, 'python'))
    else:
        r = re.compile('\.coverage\.\w+\.(\w+)')
        for f in coverage_files:
            m = r.search(f)
            cov = coverage.coverage(branch=True, data_file=f)
            python_coverage.setup_excludes(cov)
            cov.file_locator.relative_dir = Dir(env["builddir"]).abspath + '/'
            cov.load()
            morfs = cov.data.lines.keys()
            morfs.sort()
            cov.html_report(morfs, directory=os.path.join(covdir, 'python',
                                                          m.group(1)))

    print >> sys.stderr, "Done"

def _build_cpp_coverage(env, single):
    import subprocess
    def call(args):
        r = subprocess.call(args)
        if r != 0:
            raise OSError("%s failed with exit code %d" % (args[0], r))

    covdir = Dir(env["builddir"]+"/coverage").abspath
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
    single = env['html_coverage'] == 'single'
    if env.get('pycoverage', None):
        _build_python_coverage(env, single)
    if env.get('cppcoverage', None):
        _build_cpp_coverage(env, single)

def register(env):
    """Set up HTML coverage"""
    covdir = Dir(env["builddir"]+"/coverage").abspath
    shutil.rmtree(covdir, ignore_errors=True)
    os.makedirs(covdir)
    atexit.register(_build_html_coverage, env)
