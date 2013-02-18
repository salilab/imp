import sys
import os
import re
import imp
import os.path
import glob
import pickle
from optparse import OptionParser
import python_coverage
from python_coverage import coverage

def _get_unique_name(name):
    if name not in sys.modules:
        return name
    else:
        for i in range(50):
            candidate = name + "_" + str(i)
            if candidate not in sys.modules:
                return candidate
        raise ValueError("Could not disambiguate test module name %s" % name)

def _import_test(name):
    """Import a Python test module. The module is given a unique name
       if necessary (_test_0, _test_1 etc.) so that modules with
       the same name but in different directories can be read without
       overwriting each other in sys.modules."""
    fh, pathname, desc = imp.find_module(name)
    try:
        return imp.load_module(_get_unique_name(name), fh, pathname, desc)
    finally:
        # Make sure the file is closed regardless of what happens
        if fh:
            fh.close()


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
    parser.add_option("--html_coverage", dest="html_coverage", type="string",
                      default=None,
                      help="directory to write HTML coverage info into")
    parser.add_option("--global_coverage", dest="global_coverage",
                      action="store_true", default=False,
                      help="whether coverage is affected by running other "
                           "modules'/applications' tests")
    parser.add_option("--output", dest="output", type="string", default="-",
                      help="write coverage output into the named "
                           "file (or stderr if '-')")
    parser.add_option("--results", dest="results", type="string", default="-",
                      help="write details of the test results to this file")
    return parser.parse_args()

class CoverageTester(object):
    """Display a final coverage report"""
    def __init__(self, opts):
        self.opts = opts
        self.start()

    def start(self):
        cwd = os.path.dirname(sys.argv[0])
        # Don't show full paths in coverage output
        if self.opts.application:
            if not self.opts.pyexe:
                # No Python coverage to report for C++-only apps
                self.cov = None
                return
            self.topdir = os.path.abspath(os.path.join(cwd, '..', 'build',
                                                       'bin'))
            self.mods = [self.topdir + '/' + x for x in self.opts.pyexe]
            # Ensure that applications started as subprocesses are
            # themselves covered
            os.environ['IMP_COVERAGE_APPS'] = os.pathsep.join(self.opts.pyexe)
            if self.opts.global_coverage:
                os.environ['IMP_GLOBAL_COVERAGE'] = '1'
            self.cov_suffix = 'app.' + self.opts.application
        elif self.opts.module:
            path = self.opts.module.replace('.', '/')
            self.topdir = os.path.abspath(os.path.join(cwd, '..', 'build',
                                                       'lib'))
            self.mods = glob.glob(self.topdir + '/%s/*.py' % path)
            self.mods = [x for x in self.mods \
                         if not x.endswith('_version_check.py')]
            modname = self.opts.module.split('.')[-1]
            if modname == 'IMP':
                modname = 'kernel'
            self.cov_suffix = 'mod.' + modname


        data_file = '.' + self.cov_suffix + '.coverage'
        os.environ['IMP_COVERAGE_DATA_FILE'] = data_file
        for cov in glob.glob(data_file + '*'):
            os.unlink(cov)

        if self.opts.global_coverage:
            self.cov = coverage.coverage(branch=True, data_file=data_file)
        else:
            # If we're only interested in this module/application, we can
            # speed up coverage by having it ignore all other files
            self.cov = coverage.coverage(branch=True, include=self.mods,
                                         data_file=data_file)
        python_coverage.setup_excludes(self.cov)
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

        if self.opts.html_coverage:
            # Add empty data for files that weren't executed at all
            for m in morfs:
                if m not in self.cov.data.lines:
                    self.cov.data.lines[m] = {}
                    self.cov.data.arcs[m] = {}

            # Save coverage info to be consolidated at the end of the build
            self.cov.data.write_file(os.path.join(self.opts.html_coverage,
                                             '.coverage.' + self.cov_suffix))
            morfs_file = os.path.join(self.opts.html_coverage,
                                      'morfs.' + self.cov_suffix)
            pickle.dump(self.mods, open(morfs_file, 'wb'), protocol=-1)

    def report(self):
        if self.cov is None:
            return
        self.cov.stop()
        self.cov.combine()
        self.cov.use_cache(True)

        self.cov.file_locator.relative_dir = self.topdir + '/'

        if self.opts.application and self.opts.pyexe:
            self.report_morfs(self.mods,
                              "%s application" % self.opts.application,
                              "build/bin")
        elif self.opts.module:
            path = self.opts.module.replace('.', '/')
            self.report_morfs(self.mods, "%s module" % self.opts.module,
                              "build/lib/%s" % path)

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
    import IMP.base
    # test cases don't clean up memory properly when run as part
    # of run-all-tests
    if IMP.base.IMP_HAS_CHECKS >= IMP.base.USAGE_AND_INTERNAL:
        IMP.base.set_show_leaked_objects(False)

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
        pickle.dump(main.result.all_tests, open(opts.results, 'wb'),
                    protocol=-1)
    if covtest:
        covtest.report()
    sys.exit(not main.result.wasSuccessful())
