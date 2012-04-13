import atexit
import glob
import shutil
import os
import sys
from SCons.Script import Dir
import python_coverage
from python_coverage import coverage

def _build_python_coverage(env):
    covdir = Dir(env["builddir"]+"/coverage").abspath
    # Return if no coverage to report (i.e. no tests were run)
    if len(glob.glob('%s/.coverage*' % covdir)) == 0:
        return
    print >> sys.stderr, "Generating Python HTML coverage report... ",

    cov = coverage.coverage(branch=True,
                            data_file=os.path.join(covdir, '.coverage'))
    python_coverage.setup_excludes(cov)
    cov.file_locator.relative_dir = Dir(env["builddir"]).abspath + '/'
    cov.combine()
    morfs = cov.data.lines.keys()
    morfs.sort()
    cov.html_report(morfs, directory=os.path.join(covdir, 'python'))
    print >> sys.stderr, "Done"

def _build_html_coverage(env):
    if env.get('pycoverage', None):
        _build_python_coverage(env)

def register(env):
    """Set up HTML coverage"""
    covdir = Dir(env["builddir"]+"/coverage").abspath
    shutil.rmtree(covdir, ignore_errors=True)
    os.mkdir(covdir)
    atexit.register(_build_html_coverage, env)
