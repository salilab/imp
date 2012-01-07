import sys
import os
import re
import imp
import os.path
import IMP.test
import glob
from IMP.test._compat_python import coverage
from optparse import OptionParser

# Make sure we use the same version of unittest as the IMP testcases themselves
from IMP.test import unittest

class _TestModuleImporter(object):
    """Import a Python test module. The module
       is given a unique name (_test_0, _test_1 etc.) so that modules with
       the same name but in different directories can be read without
       overwriting each other in sys.modules."""
    def __init__(self):
        self._serial = 0
    def __call__(self, name):
        fh, pathname, desc = imp.find_module(name)
        self._serial += 1
        try:
            return imp.load_module('_test_%d' % self._serial, fh,
                                   pathname, desc)
        finally:
            # Make sure the file is closed regardless of what happens
            if fh:
                fh.close()
_import_test = _TestModuleImporter()

class RegressionTest(object):
    """Run all tests in files called test_*.py in current directory and
       subdirectories"""

    def __init__(self, files):
        self._files = files

    def __call__(self):
        if len(self._files) ==0:
            return unittest.TestSuite([])
        a_file=self._files[0]
        # evil hack
        os.environ['TEST_DIRECTORY'] = a_file[0:a_file.rfind("/test/")+6]
        #return 0
        modobjs = []
        for f in self._files:
            nm= os.path.split(f)[1]
            dir= os.path.split(f)[0]
            modname = os.path.splitext(nm)[0]
            sys.path.insert(0, dir)
            modobjs.extend([_import_test(modname)])
            sys.path.pop(0)

        tests = [unittest.defaultTestLoader.loadTestsFromModule(o) \
                 for o in modobjs]
        return unittest.TestSuite(tests)

def parse_options():
    parser = OptionParser()
    parser.set_defaults(pycoverage=False)
    parser.add_option("--module", dest="module", type="string", default=None,
                      help="name of IMP module being tested, e.g. IMP.foo")
    parser.add_option("--pycoverage", dest="pycoverage", type="string",
                      default=None,
                      help="report on Python coverage of tests to the named "
                           "file (or stderr if '-'); use with --module")
    return parser.parse_args()

def start_coverage():
    # Try to exclude SWIG and IMP boilerplate from coverage checks
    coverage.exclude("def swig_import_helper\(")
    coverage.exclude("def _swig_")
    coverage.exclude("class (_ostream|_DirectorObjects|"
                     "IMP_\w+_SwigPyIterator)\(")
    coverage.exclude("^\s+import _IMP_")
    coverage.exclude("^except (Name|Attribute)Error:")
    coverage.exclude("^\s+weakref_proxy =")
    coverage.exclude("^def [sg]et_check_level")
    coverage.start()

def report_coverage(opts):
    coverage.stop()
    coverage.the_coverage.collect()
    coverage.use_cache(False)

    # Don't show full paths to modules in coverage output
    cwd = os.path.dirname(sys.argv[0])
    topdir = os.path.abspath(os.path.join(cwd, '..', 'build', 'lib')) + '/'
    coverage.the_coverage.relative_dir = topdir

    if opts.module:
        path = opts.module.replace('.', '/')
        mods = [topdir + '%s/*.py' % path]
        if opts.pycoverage == '-':
            outfh = sys.stderr
        else:
            outfh = open(opts.pycoverage, 'w')
            print >> sys.stderr, \
                  "\nPython coverage of %s module written to %s." \
                  % (opts.module, opts.pycoverage)
        coverage.report(mods, file=outfh,
                        omit_prefixes=['%s/_version_check' % path])

    for cov in glob.glob('.coverage.*'):
        os.unlink(cov)

if __name__ == "__main__":
    opts, args = parse_options()
    if opts.pycoverage:
        start_coverage()
    r = RegressionTest(args)
    main = unittest.main(defaultTest="r", testRunner=IMP.test._TestRunner,
                         argv=[sys.argv[0], "-v"], exit=False)
    if opts.pycoverage:
        report_coverage(opts)
    sys.exit(not main.result.wasSuccessful())
