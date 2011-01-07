import IMP.test
from IMP.test import unittest
import sys
import os
import re

def get_unmet_module_deps(f, disabled_modules):
    unmet_deps = []
    def check_disabled(modname):
        if modname.startswith("IMP.") and modname[4:] in disabled_modules:
            unmet_deps.append(modname[4:])
    # Note that we currently only match import lines with no indentation. It is
    # assumed that indented imports are within try/except blocks, and thus
    # these examples should *not* be skipped if the module is disabled (the
    # script will take appropriate action in this case)
    import_re = re.compile('import\s+(.*?)(\s+as\s+.+)?\s*$')
    from_re = re.compile('from\s+(\S+)\s+import\s+(.*?)(\s+as\s+.+)?\s*$')
    for line in open(f):
        # Parse lines of the form 'import a.b, a.c (as foo)'
        m = import_re.match(line)
        if m:
            for modname in [x.strip() for x in m.group(1).split(',')]:
                check_disabled(modname)
        # Parse lines of the form 'from a import b, c (as foo)'
        m = from_re.match(line)
        if m:
            # Make sure 'a' is not disabled
            check_disabled(m.group(1))
            for modname in [x.strip() for x in m.group(2).split(',')]:
                # Make sure 'a.b' is not disabled (in case b is a module)
                check_disabled(m.group(1) + '.' + modname)
    return unmet_deps

def scan_tested_examples(filename, tested_examples):
    # Not a great regex; could miss some examples, or those in subdirs
    r = re.compile('get_example_path\(\"([^"]+)\"\)')
    for line in open(filename):
        m = r.search(line)
        if m:
            tested_examples[m.group(1)] = None

def test_example(filename, shortname, disabled_modules):
    skip = "pass"
    unmet_deps = get_unmet_module_deps(filename, disabled_modules)
    if len(unmet_deps) > 0:
        if len(unmet_deps) == 1:
            skip = "self.skipTest(\"module '%s' is disabled\")" % unmet_deps[0]
        else:
            skip = "self.skipTest(\"modules %s are disabled\")" \
                   % ", ".join(["'%s'" % x for x in unmet_deps])
    exec(
"""class RunExample(IMP.test.TestCase):
        def test_run_example(self):
            "Run example %s"
            %s
            self.run_example("%s")
            return""" % (shortname, skip, filename))
    return RunExample("test_run_example")

class RegressionTest(object):

    def __init__(self, files, excluded_modules):
        self._files = files
        self._excluded_modules = excluded_modules

    def __call__(self):
        modobjs = []
        tested_examples = {}
        for f in self._files:
            nm= os.path.split(f)[1]
            dir= os.path.split(f)[0]
            if nm.startswith("test_"):
                scan_tested_examples(f, tested_examples)
                modname = os.path.splitext(nm)[0]
                sys.path.insert(0, dir)
                modobjs.append(__import__(modname))
                sys.path.pop(0)
        tests = [unittest.defaultTestLoader.loadTestsFromModule(o) \
                 for o in modobjs]
        suite = unittest.TestSuite(tests)

        # For all examples that don't have an explicit test to exercise them,
        # just run them to make sure they don't crash
        for f in self._files:
            nm= os.path.split(f)[1]
            dir= os.path.split(f)[0]
            if not nm.startswith("test_") and not nm in tested_examples:
                suite.addTest(test_example(f, nm, self._excluded_modules))

        return suite


if __name__ == "__main__":
    if len(sys.argv) <2:
        exit(0)
    excluded_modules = sys.argv[1].split(":")
    files = sys.argv[2:]
    sys.argv=[sys.argv[0], "-v"]
    r = RegressionTest(files, excluded_modules)
    unittest.main(defaultTest="r", testRunner=IMP.test._TestRunner)
