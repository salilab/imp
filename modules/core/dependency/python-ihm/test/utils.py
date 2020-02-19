import os
import sys
import tempfile
import contextlib
import shutil
import unittest

# If we're using Python 2.6, add in more modern unittest convenience methods
if not hasattr(unittest.TestCase, 'assertIn'):
    def assertIn(self, member, container, msg=None):
        return self.assertTrue(member in container,
                        msg or '%s not found in %s' % (member, container))
    def assertNotIn(self, member, container, msg=None):
        return self.assertTrue(member not in container,
                        msg or '%s unexpectedly found in %s'
                        % (member, container))
    def assertIs(self, a, b, msg=None):
        return self.assertTrue(a is b, msg or '%s is not %s' % (a, b))
    def assertIsInstance(self, obj, cls, msg=None):
        return self.assertTrue(isinstance(obj, cls),
                        msg or '%s is not an instance of %s' % (obj, cls))
    def assertLess(self, a, b, msg=None):
        return self.assertTrue(a < b, msg or '%s not less than %s' % (a, b))
    def assertGreater(self, a, b, msg=None):
        return self.assertTrue(a > b, msg or '%s not greater than %s' % (a, b))
    def assertLessEqual(self, a, b, msg=None):
        return self.assertTrue(a <= b,
                        msg or '%s not less than or equal to %s' % (a, b))
    def assertGreaterEqual(self, a, b, msg=None):
        return self.assertTrue(a >= b,
                        msg or '%s not greater than or equal to %s' % (a, b))
    def assertIsNone(self, obj, msg=None):
        return self.assertTrue(obj is None, msg or '%s is not None' % obj)
    def assertIsNotNone(self, obj, msg=None):
        return self.assertTrue(obj is not None, msg or 'unexpectedly None')
    unittest.TestCase.assertIn = assertIn
    unittest.TestCase.assertNotIn = assertNotIn
    unittest.TestCase.assertIs = assertIs
    unittest.TestCase.assertIsInstance = assertIsInstance
    unittest.TestCase.assertLess = assertLess
    unittest.TestCase.assertGreater = assertGreater
    unittest.TestCase.assertLessEqual = assertLessEqual
    unittest.TestCase.assertGreaterEqual = assertGreaterEqual
    unittest.TestCase.assertIsNone = assertIsNone
    unittest.TestCase.assertIsNotNone = assertIsNotNone

def set_search_paths(topdir):
    """Set search paths so that we can import Python modules"""
    os.environ['PYTHONPATH'] = topdir + os.pathsep \
                               + os.environ.get('PYTHONPATH', '')
    sys.path.insert(0, topdir)

def get_input_file_name(topdir, fname):
    """Return full path to a test input file"""
    return os.path.join(topdir, 'test', 'input', fname)

@contextlib.contextmanager
def temporary_directory(dir=None):
    _tmpdir = tempfile.mkdtemp(dir=dir)
    yield _tmpdir
    shutil.rmtree(_tmpdir, ignore_errors=True)

if 'coverage' in sys.modules:
    import atexit
    # Collect coverage information from subprocesses
    __site_tmpdir = tempfile.mkdtemp()
    with open(os.path.join(__site_tmpdir, 'sitecustomize.py'), 'w') as fh:
        fh.write("""
import coverage
import atexit
import os

_cov = coverage.coverage(branch=True, data_suffix=True, auto_data=True,
                         data_file=os.path.join('%s', '.coverage'))
_cov.start()

def _coverage_cleanup(c):
    c.stop()
atexit.register(_coverage_cleanup, _cov)
""" % os.getcwd())

    os.environ['PYTHONPATH'] = __site_tmpdir + os.pathsep \
                               + os.environ.get('PYTHONPATH', '')

    def __cleanup(d):
        shutil.rmtree(d, ignore_errors=True)
    atexit.register(__cleanup, __site_tmpdir)
