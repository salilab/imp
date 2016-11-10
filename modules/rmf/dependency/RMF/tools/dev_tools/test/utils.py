import tempfile
import shutil
import os
import sys
import contextlib

@contextlib.contextmanager
def TempDir():
    _tmpdir = tempfile.mkdtemp()
    yield _tmpdir
    shutil.rmtree(_tmpdir, ignore_errors=True)

@contextlib.contextmanager
def RunInTempDir():
    _tmpdir = tempfile.mkdtemp()
    _olddir = os.getcwd()
    os.chdir(_tmpdir)
    yield _tmpdir
    os.chdir(_olddir)
    shutil.rmtree(_tmpdir, ignore_errors=True)

@contextlib.contextmanager
def mocked_function(module, funcname, replacement):
    """Temporarily replace module.funcname with replacement."""
    oldfunc = getattr(module, funcname)
    setattr(module, funcname, replacement)
    yield
    setattr(module, funcname, oldfunc)

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

_cov = coverage.coverage(branch=True, data_suffix=True, auto_data=True,
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
