import os

# Only use coverage if it's new enough
try:
    import coverage
    if not hasattr(coverage.coverage, 'combine'):
        coverage = None
    else:
        # Override default filename normalization; by default coverage passes
        # filename through os.path.realpath(), which removes the symlink.
        # We don't want this behavior, since we want to talk
        # about build/lib/IMP/foo/__init__.py, not build/src/IMP.foo.py
        def our_abs_file(self, filename):
            return os.path.normcase(os.path.abspath(filename))
        coverage.files.FileLocator.abs_file = our_abs_file
except ImportError:
    coverage = None

def setup_excludes(cov):
    # Try to exclude SWIG and IMP boilerplate from coverage checks
    cov.exclude("def swig_import_helper\(")
    cov.exclude("def _swig_")
    cov.exclude("class (_ostream|_DirectorObjects|"
           "IMP_\w+_SwigPyIterator)\(")
    cov.exclude("^\s+import _IMP_")
    cov.exclude("^except (Name|Attribute)Error:")
    cov.exclude("^\s+weakref_proxy =")
    cov.exclude("^def [sg]et_check_level")

def setup():
    """Make sure that coverage works for subprocesses"""
    f = open('build/lib/sitecustomize.py', 'w')
    f.write("""
import os
# If we're being imported from a Python application that is in turn being run
# by a test case with coverage turned on, also turn on coverage in the Python
# application.
if 'IMP_COVERAGE_APPS' in os.environ:
    apps = os.environ['IMP_COVERAGE_APPS'].split(os.pathsep)
    import coverage
    # See scons_tools/python_coverage.py for further explanation
    def _our_abs_file(self, filename):
        return os.path.normcase(os.path.abspath(filename))
    coverage.files.FileLocator.abs_file = _our_abs_file
    _cov = coverage.coverage(branch=True, data_suffix=True, auto_data=True,
                             source=[os.path.join(
                                      os.environ['IMP_BIN_DIR'], x) \\
                                     for x in apps])
    _cov.start()
""")
