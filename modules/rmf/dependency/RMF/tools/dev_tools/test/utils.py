import tempfile
import shutil
import os
import sys

class TempDir(object):
    def __enter__(self):
        self.__tmpdir = tempfile.mkdtemp()
        return self.__tmpdir
    def __exit__(self, exc_type, exc_value, traceback):
        shutil.rmtree(self.__tmpdir, ignore_errors=True)


class RunInTempDir(object):
    def __enter__(self):
        self.__tmpdir = tempfile.mkdtemp()
        self.__olddir = os.getcwd()
        os.chdir(self.__tmpdir)
        return self.__tmpdir
    def __exit__(self, exc_type, exc_value, traceback):
        os.chdir(self.__olddir)
        shutil.rmtree(self.__tmpdir, ignore_errors=True)


def write_file(fname, content):
    with open(fname, "w") as fh:
        fh.write(content)

def read_file(fname):
    with open(fname, "r") as fh:
        return fh.read()

if 'coverage' in sys.modules:
    import atexit
    # Collect coverage information from subprocesses
    __site_tmpdir = tempfile.mkdtemp()
    with open(os.path.join(__site_tmpdir, 'sitecustomize.py'), 'w') as fh:
        fh.write("""
import coverage
import atexit

_cov = coverage.coverage(branch=False, data_suffix=True, auto_data=True,
                         data_file='%s/.coverage')
_cov.start()

def _coverage_cleanup(c):
    c.stop()
atexit.register(_coverage_cleanup, _cov)
""" % os.getcwd())

    os.environ['PYTHONPATH'] = __site_tmpdir

    def __cleanup(d):
        shutil.rmtree(d, ignore_errors=True)
    atexit.register(__cleanup, __site_tmpdir)
