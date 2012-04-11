import sys
import os
import re
import imp
import os.path
import glob
from optparse import OptionParser

# Only use coverage if it's new enough
try:
    import coverage
    if not hasattr(coverage.coverage, 'combine'):
        coverage = None
except ImportError:
    coverage = None

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
    parser.add_option("--module", dest="module", type="string", default=None,
                      help="name of IMP module being tested, e.g. IMP.foo")
    parser.add_option("--application", dest="application", type="string",
                      default=None,
                      help="name of IMP application being tested, "
                           "e.g. saxs_merge")
    parser.add_option("--pyexe", dest="pyexe", type="string", default=[],
                      action="append",
                      help="Python executable installed as part of an "
                           "application; can be used multiple times")
    parser.add_option("--pycoverage", dest="pycoverage", type="choice",
                      default="no", choices=["no", "lines", "annotate"])
    parser.add_option("--output", dest="output", type="string", default="-",
                      help="write coverage output into the named "
                           "file (or stderr if '-')")
    parser.add_option("--results", dest="results", type="string", default="-",
                      help="write details of the test failures to this file")
    return parser.parse_args()

class CoverageTester(object):
    """Display a final coverage report"""
    def __init__(self, opts):
        self.opts = opts
        self.start()

    def start(self):
        cwd = os.path.dirname(sys.argv[0])
        # Don't show full paths in coverage output
        if self.opts.application and self.opts.pyexe:
            self.topdir = os.path.abspath(os.path.join(cwd, '..', 'build',
                                                       'bin'))
            self.mods = [self.topdir + '/' + x for x in self.opts.pyexe]
        elif self.opts.module:
            path = self.opts.module.replace('.', '/')
            self.topdir = os.path.abspath(os.path.join(cwd, '..', 'build',
                                                       'lib'))
            self.mods = glob.glob(self.topdir + '/%s/*.py' % path)
            self.mods = [x for x in self.mods \
                         if not x.endswith('_version_check.py')]

        # Override default filename normalization; by default coverage passes
        # filename through os.path.realpath(), which removes the symlink.
        # We don't want this behavior, since we want to talk
        # about build/lib/IMP/foo/__init__.py, not build/src/IMP.foo.py
        def our_abs_file(self, filename):
            return os.path.normcase(os.path.abspath(filename))
        coverage.files.FileLocator.abs_file = our_abs_file

        self.cov = coverage.coverage(branch=True, include=self.mods)

        # Try to exclude SWIG and IMP boilerplate from coverage checks
        self.cov.exclude("def swig_import_helper\(")
        self.cov.exclude("def _swig_")
        self.cov.exclude("class (_ostream|_DirectorObjects|"
                    "IMP_\w+_SwigPyIterator)\(")
        self.cov.exclude("^\s+import _IMP_")
        self.cov.exclude("^except (Name|Attribute)Error:")
        self.cov.exclude("^\s+weakref_proxy =")
        self.cov.exclude("^def [sg]et_check_level")

        self.cov.start()

    def report_morfs(self, morfs, modname, annotate_dir):
        if self.opts.pycoverage == 'lines':
            if self.opts.output == '-':
                outfh = sys.stderr
            else:
                outfh = open(self.opts.output, 'w')
                print >> sys.stderr, \
                      "\nPython coverage of %s written to %s." \
                      % (modname, self.opts.output)
            self.cov.report(morfs, file=outfh)
        elif self.opts.pycoverage == 'annotate':
            print >> sys.stderr, \
                  "\n%s annotated with Python coverage information " \
                  "in files with\n\",cover\" suffix under %s." \
                  % (modname, annotate_dir)
            self.cov.annotate(morfs)

    def report(self):
        self.cov.stop()
        self.cov.combine()
        self.cov.use_cache(False)

        self.cov.file_locator.relative_dir = self.topdir + '/'

        if self.opts.application and self.opts.pyexe:
            self.report_morfs(self.mods,
                              "%s application" % self.opts.application,
                              "build/bin")
        elif self.opts.module:
            path = self.opts.module.replace('.', '/')
            self.report_morfs(self.mods, "%s module" % self.opts.module,
                              "build/lib/%s" % path)

        for cov in glob.glob('.coverage.*'):
            os.unlink(cov)

def import_imp_modules(covtest):
    # Note that we need to import all IMP modules *after* we start Python
    # coverage collection, otherwise large parts of kernel/base/algebra
    # will be erroneously reported as not covered (since they were run
    # before coverage reporting started)
    global IMP, unittest
    import IMP.test
    # Make sure we use the same version of unittest as the IMP testcases
    # themselves
    from IMP.test import unittest

    # Ensure that any IMP Python scripts run by tests are themselves checked
    # for Python coverage (must be after we import IMP, since we are
    # already covering this process)
    if covtest:
        os.environ['IMP_COVERAGE'] = '1'

if __name__ == "__main__":
    opts, args = parse_options()
    covtest = None
    if opts.pycoverage != 'no':
        if coverage:
            covtest = CoverageTester(opts)
        else:
            print >> sys.stderr, "Python coverage was requested but a " \
                                 + "new enough 'coverage' module could not " \
                                 + "be found on your system"
    import_imp_modules(covtest)
    r = RegressionTest(args)
    # Hide our command line options from any module we import
    sys.argv = [sys.argv[0]]
    main = unittest.main(defaultTest="r", testRunner=IMP.test._TestRunner,
                         argv=[sys.argv[0], "-v"], exit=False)
    if opts.results:
        out= file(opts.results, "w")
        if len(main.result.errors) > 0:
            print >> out, "Errors:",", ".join([main.result.getDescription(r[0]) for r in main.result.errors])
        if len(main.result.skipped) > 0:
            print >> out, "Skips:",", ".join([main.result.getDescription(r[0]) for r in main.result.skipped])
    if covtest:
        covtest.report()
    sys.exit(not main.result.wasSuccessful())
