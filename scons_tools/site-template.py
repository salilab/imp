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
    if 'IMP_GLOBAL_COVERAGE' in os.environ:
        _cov = coverage.coverage(branch=True, data_suffix=True, auto_data=True,
                                 data_file=os.environ['IMP_COVERAGE_DATA_FILE'])
    else:
        _cov = coverage.coverage(branch=True, data_suffix=True, auto_data=True,
                                 data_file=os.environ['IMP_COVERAGE_DATA_FILE'],
                                 source=[os.path.join(
                                          os.environ['IMP_BIN_DIR'], x) \
                                         for x in apps])
    _cov.start()
