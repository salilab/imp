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
