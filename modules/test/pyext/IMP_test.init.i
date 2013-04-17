%pythoncode %{

"""@namespace IMP::test
   @brief Methods and classes for testing the IMP kernel and modules.
   @ingroup python
"""

import re, math
import sys
import os
import re
import tempfile
import random
import IMP
import time
import types
import shutil
import _compat_python
import _compat_python.unittest2
import datetime

# Fall back to the sets.Set class on older Pythons that don't have
# the 'set' builtin type.
try:
    set = set
except NameError:
    import sets
    set = sets.Set

# Load a new enough unittest package (should have the 'skip' decorator)
# - On Python 2.7 or 3.2, the standard 'unittest' package will work.
# - On older Pythons, use the 'unittest2' package if available, otherwise use
#   our bundled version of this package.
def __load_unittest_package():
    errors = []
    for modname, fromlist in (('unittest', []),
                              ('unittest2', []),
                              ):
        try:
            u = __import__(modname, {}, {}, fromlist)
            if hasattr(u, 'skip'):
                return u
            else:
                errors.append("'%s' does not have the 'skip' decorator" \
                              % modname)
        except ImportError, e:
            errors.append(str(e))
        #u = __import__("_compat_python.unittest2
        return _compat_python.unittest2
    raise ImportError("IMP.test requires a newer version of Python's unittest "
                      "package than is available. Either upgrade to a new "
                      "enough Python (at least 2.7 or 3.2) or install the "
                      "unittest2 package. Encountered errors: %s" \
                      % "; ".join(errors))
unittest = __load_unittest_package()

# Expose some unittest decorators for convenience
expectedFailure = unittest.expectedFailure
skip = unittest.skip
skipIf = unittest.skipIf
skipUnless = unittest.skipUnless

class RunInTempDir(object):
    """Simple RAII-style class to run in a temporary directory.
       When the object is created, the temporary directory is created
       and becomes the current working directory. When the object goes out
       of scope, the working directory is reset and the temporary directory
       deleted."""
    def __init__(self):
        self.origdir = os.getcwd()
        self.tmpdir = tempfile.mkdtemp()
        os.chdir(self.tmpdir)
    def __del__(self):
        os.chdir(self.origdir)
        shutil.rmtree(self.tmpdir, ignore_errors=True)


def numerical_derivative(func, val, step):
    """Calculate the derivative of the single-value function `func` at
       point `val`. The derivative is calculated using simple finite
       differences starting with the given `step`; Richardson extrapolation
       is then used to extrapolate the derivative at step=0."""
    maxsteps = 50
    con = 1.4
    safe = 2.0
    err = 1.0e30
    f1 = func(val + step)
    f2 = func(val - step)
    # create first element in triangular matrix d of derivatives
    d = [[(f1 - f2) / (2.0 * step)]]
    retval = None
    for i in range(1, maxsteps):
        d.append([0.] * (i + 1))
        step /= con
        f1 = func(val + step)
        f2 = func(val - step)
        d[i][0] = (f1 - f2) / (2.0 * step)
        fac = con * con
        for j in range(1, i + 1):
            d[i][j] = (d[i][j-1] * fac - d[i-1][j-1]) / (fac - 1.)
            fac *= con * con
            errt = max(abs(d[i][j] - d[i][j-1]),
                       abs(d[i][j] - d[i-1][j-1]))
            if errt <= err:
                err = errt
                retval = d[i][j]
        if abs(d[i][i] - d[i-1][i-1]) >= safe * err:
            break
    if retval is None:
        raise ValueError("Cannot calculate numerical derivative")
    return retval


def xyz_numerical_derivatives(model, xyz, step):
    """Calculate the x,y and z derivatives of `model`'s scoring function
       on the `xyz` particle. The derivatives are approximated numerically
       using the numerical_derivatives() function."""
    class _XYZDerivativeFunc(object):
        def __init__(self, xyz, basis_vector):
            self._xyz = xyz
            self._model = xyz.get_particle().get_model()
            self._basis_vector = basis_vector
            self._starting_coordinates = xyz.get_coordinates()

        def __call__(self, val):
            self._xyz.set_coordinates(self._starting_coordinates + \
                                      self._basis_vector * val)
            return self._model.evaluate(False)

    return tuple([IMP.test.numerical_derivative(_XYZDerivativeFunc(xyz,
                                          IMP.algebra.Vector3D(*x)), 0, 0.01) \
                  for x in ((1,0,0), (0,1,0), (0,0,1))])


class TestCase(unittest.TestCase):
    """Super class for IMP test cases"""

    def setUp(self):
        self.__check_level = IMP.base.get_check_level()
        # Turn on expensive runtime checks while running the test suite:
        IMP.base.set_check_level(IMP.base.USAGE_AND_INTERNAL)
        # python ints are bigger than C++ ones, so we need to make sure it fits
        # otherwise python throws fits
        IMP.base.random_number_generator.seed(hash(time.time())%2**30)

    def tearDown(self):
        # Restore original check level
        IMP.base.set_check_level(self.__check_level)

    def get_input_file_name(self, filename):
        """Get the full name of an input file in the top-level
           test directory."""
        # If we ran from run-all-tests.py, it set an env variable for us with
        # the top-level test directory
        if 'TEST_DIRECTORY' in os.environ:
            top = os.environ['TEST_DIRECTORY']
            return os.path.join(top, 'input', filename)
        else:
            # Otherwise, search up from the test's directory until we find
            # the input directory
            testdir = os.path.dirname(os.path.abspath(sys.argv[0]))
            dirs = testdir.split(os.path.sep)
            for i in range(len(dirs), 0, -1):
                input = os.path.sep.join(dirs[:i] + ['input'])
                if os.path.isdir(input):
                    return os.path.join(input, filename)
        # If not found, default to the current working directory:
        ret= os.path.join('input', filename)
        if not open(ret, "r"):
            raise IOError("Test input file "+ret+" does not exist")

    def open_input_file(self, filename, mode='rb'):
        """Open and return an input file in the top-level test directory."""
        return open(self.get_input_file_name(filename), mode)

    def get_tmp_file_name(self, filename):
        """Get the full name of an output file in the build/tmp directory."""
        dirpath=os.environ['IMP_TMP_DIR']
        if not os.path.exists(dirpath):
            os.mkdir(dirpath)
        return os.path.join(dirpath, filename)

    def get_magnitude(self, vector):
        return sum([x*x for x in vector], 0)**.5

    def assertXYZDerivativesInTolerance(self, model, xyz, tolerance=0,
                                        percentage=0):
        """Assert that x,y,z analytical derivatives match numerical within
           a tolerance, or a percentage (of the analytical value), whichever
           is larger."""
        model.evaluate(True)
        derivs = xyz.get_derivatives()
        num_derivs = xyz_numerical_derivatives(model, xyz, 0.01)
        pct = percentage / 100.0
        self.assertAlmostEqual(self.get_magnitude(derivs-num_derivs),0,
                               delta=tolerance+percentage*self.get_magnitude(num_derivs),
                               msg="Don't match "+str(derivs) + str(num_derivs))
        self.assertAlmostEqual(derivs[0], num_derivs[0],
                               delta=max(tolerance, abs(derivs[0]) * pct))
        self.assertAlmostEqual(derivs[1], num_derivs[1],
                               delta=max(tolerance, abs(derivs[1]) * pct))
        self.assertAlmostEqual(derivs[2], num_derivs[2],
                               delta=max(tolerance, abs(derivs[2]) * pct))

    def create_point_particle(self, model, x, y, z):
        """Make a particle with optimizable x, y and z attributes, and
           add it to the model."""
        p = IMP.Particle(model)
        p.add_attribute(IMP.FloatKey("x"), x, True)
        p.add_attribute(IMP.FloatKey("y"), y, True)
        p.add_attribute(IMP.FloatKey("z"), z, True)
        return p

    def probabilistic_test(self, testcall, chance_of_failure):
        """Help handle a test which is expected to fail some fraction of
        the time. The test is run multiple times and an exception
        is thrown only if it fails too many times."""
        prob=chance_of_failure
        tries=1
        while prob > .001:
            tries=tries+1
            prob= prob*chance_of_failure
        for i in range(0, tries):
            try:
                eval(testcall)
            except:
                pass
            else:
                return
        eval(testcall)
        raise AssertError("Too many failures")

    def failure_probability(self, testcall):
        """Estimate how like a given block of code is to raise an
        AssertionError."""
        failures=0
        tries=0.0
        while failures < 10 and tries <1000:
            try:
                eval(testcall)
            except:
                failures=failures+1
            tries=tries+1
        return failures/tries

    def randomize_particles(self, particles, deviation):
        """Randomize the xyz coordinates of a list of particles"""
        # Note: cannot use XYZ here since that pulls in IMP.core
        xkey = IMP.FloatKey("x")
        ykey = IMP.FloatKey("y")
        zkey = IMP.FloatKey("z")
        for p in particles:
            p.set_value(xkey, random.uniform(-deviation, deviation))
            p.set_value(ykey, random.uniform(-deviation, deviation))
            p.set_value(zkey, random.uniform(-deviation, deviation))

    def particle_distance(self, p1, p2):
        """Return distance between two given particles"""
        xkey = IMP.FloatKey("x")
        ykey = IMP.FloatKey("y")
        zkey = IMP.FloatKey("z")
        dx = p1.get_value(xkey) - p2.get_value(xkey)
        dy = p1.get_value(ykey) - p2.get_value(ykey)
        dz = p1.get_value(zkey) - p2.get_value(zkey)
        return math.sqrt(dx*dx + dy*dy + dz*dz)

    def check_unary_function_deriv(self, func, lb, ub, step):
        """Check the unary function func's derivatives against numerical
           approximations between lb and ub"""
        for f in [lb + i * step for i in range(1, int((ub-lb)/step))]:
            (v,d)= func.evaluate_with_derivative(f)
            da = numerical_derivative(func.evaluate, f, step / 10.)
            self.assertAlmostEqual(d, da, delta=max(abs(.1 *d), 0.01))

    def check_unary_function_min(self, func, lb, ub, step, expected_fmin):
        """Make sure that the minimum of the unary function func over the
           range between lb and ub is at expected_fmin"""
        fmin, vmin = lb, func.evaluate(lb)
        for f in [lb + i * step for i in range(1, int((ub-lb)/step))]:
            v = func.evaluate(f)
            if v < vmin:
                fmin, vmin = f, v
        self.assertAlmostEqual(fmin, expected_fmin, delta=step)

    def create_particles_in_box(self, model, num=10,
                                lb= [0,0,0],
                                ub= [10,10,10]):
        """Create a bunch of particles in a box"""
        import IMP.algebra
        lbv=IMP.algebra.Vector3D(lb[0],lb[1],lb[2])
        ubv=IMP.algebra.Vector3D(ub[0],ub[1],ub[2])
        ps= []
        for i in range(0,num):
            v = IMP.algebra.get_random_vector_in(IMP.algebra.BoundingBox3D(lbv, ubv))
            p = self.create_point_particle(model, v[0], v[1], v[2])
            ps.append(p)
        return ps
    def _get_type(self, module, name):
        return eval('type('+module+"."+name+')')
    def assertValueObjects(self, module, exceptions):
        "Check that all the C++ classes in the module are values or objects."
        all= dir(module)
        bad=[]
        for name in all:
            if self._get_type(module.__name__, name)==types.TypeType and not name.startswith("_"):
                if name.find("SwigPyIterator") != -1:
                    continue
                # Exclude Python-only classes
                if not eval('hasattr(%s.%s, "__swig_destroy__")' \
                            % (module.__name__, name)):
                    continue
                if name in exceptions:
                    continue
                if name not in eval(module.__name__+"._value_types")\
                       and name not in eval(module.__name__+"._object_types")\
                       and name not in eval(module.__name__+"._raii_types")\
                       and name not in eval(module.__name__+"._plural_types"):
                    bad.append(name)
        message="All IMP classes should be labeled values or as objects to get memory management correct in python. The following are not:\n%s\nPlease add an IMP_SWIG_OBJECT or IMP_SWIG_VALUE call to the python wrapper, or if the class has a good reason to be neither, add the name to the value_object_exceptions list in the IMPModuleTest call." \
                          % (str(bad))
        self.assertEquals(len(bad), 0,
                          message)
        for e in exceptions:
            self.assertTrue(e not in eval(module.__name__+"._value_types")\
                       + eval(module.__name__+"._object_types")\
                       + eval(module.__name__+"._raii_types")\
                       + eval(module.__name__+"._plural_types"),
                        "Value/Object exception "+e+" is not an exception")

    def _check_spelling(self, word, words):
        """Check that the word is spelled correctly"""
        if "words" not in dir(self):
            wordlist= open(IMP.test.get_data_path("linux.words"), "r").read().split("\n")
            # why is "all" missing on my mac?
            custom_words=["info", "prechange", "int", "ints", "optimizeds", "graphviz",
                          "voxel", "voxels", "endian", 'rna', 'dna',
                          "xyzr", "pdbs", "fft", "ccc"]
            self.words=set(wordlist+custom_words)
        if self.words:
            for i in "0123456789":
                if i in word:
                    return True
            if word in words:
                return True
            if word in self.words:
                return True
            else:
                return False
        else:
            return True
    def assertClassNames(self, module, exceptions, words):
        """Check that all the classes in the module follow the imp naming conventions."""
        all= dir(module)
        misspelled = []
        bad=[]
        cc=re.compile("([A-Z][a-z]*)")
        for name in all:
            if self._get_type(module.__name__, name)==types.TypeType and not name.startswith("_"):
                if name.find("SwigPyIterator") != -1:
                    continue
                for t in re.findall(cc, name):
                    if not self._check_spelling(t.lower(), words):
                        misspelled.append(t.lower())
                        bad.append(name)

        self.assertEquals(len(bad), 0,
                          "All IMP classes should be properly spelled. The following are not: %s.\nMisspelled words: %s. Add words to the spelling_exceptions variable of the IMPModuleTest if needed." \
                          % (str(bad), ", ".join(set(misspelled))))

        for name in all:
            if self._get_type(module.__name__, name)==types.TypeType and not name.startswith("_"):
                if name.find("SwigPyIterator") != -1:
                    continue
                if name.find('_') != -1:
                    bad.append(name)
                if name.lower== name:
                    bad.append(name)
                for t in re.findall(cc, name):
                    if not self._check_spelling(t.lower(), words):
                        print "misspelled", t, "in", name
                        bad.append(name)
        message="All IMP classes should have CamelCase names. The following do not: %s." \
                          % ("\n".join(bad))
        self.assertEquals(len(bad), 0,
                          message)


    def _check_function_name(self, prefix, name, verbs, all, exceptions, words,
                             misspelled):
        if prefix:
            fullname=prefix+"."+name
        else:
            fullname=name
        old_exceptions=['unprotected_evaluate', "unprotected_evaluate_if_good",
                        "unprotected_evaluate_if_below",
                        "after_evaluate", "before_evaluate", "has_attribute",
                        "decorate_particle","particle_is_instance"]
        if name in old_exceptions:
            return []
        #print "name", fullname
        if fullname in exceptions:
            return []
        if name.endswith("swigregister"):
            return []
        if name.lower() != name:
            if name[0].lower() != name[0] and name.split('_')[0] in all:
                # static methods
                return []
            else:
                return [fullname]
        starts=False
        for v in verbs:
            if name.startswith(v):
                starts=True
                break
        if not starts:
            return [fullname]
        tokens= name.split("_")
        for t in tokens:
            if not self._check_spelling(t, words):
                misspelled.append(t)
                print "misspelled", t, "in", name
                return [fullname]
        return []
    def _check_function_names(self, module, prefix, names, verbs, all, exceptions, words, misspelled):
        bad=[]
        #print "names", module, prefix
        for name in names:
            if name.startswith("_") or name =="weakref_proxy":
                continue
            if self._get_type(module, name)==types.BuiltinMethodType\
                   or self._get_type(module, name)==types.MethodType:
                bad.extend(self._check_function_name(prefix, name, verbs, all, exceptions, words, misspelled))
            if self._get_type(module, name)==types.TypeType and name.find("SwigPyIterator")==-1:
                #print "sub", module+"."+name
                members=eval("dir("+module+"."+name+")")
                #print members
                bad.extend(self._check_function_names(module+"."+name,
                                                      name,
                                                      members,
                                                      verbs, [], exceptions, words, misspelled))
        return bad



    def assertFunctionNames(self, module, exceptions, words):
        """Check that all the functions in the module follow the imp naming conventions."""
        all= dir(module)
        verbs=["add", "remove", "get", "set", "evaluate", "compute", "show", "create", "destroy",
               "push", "pop", "write", "read", "do", "show", "load", "save", "reset",
               "accept", "reject",
               "clear", "handle", "update", "apply", "optimize", "reserve", "dump",
               "propose", "setup", "teardown", "visit", "find", "run", "swap", "link",
               "validate"]
        misspelled = []
        bad=self._check_function_names(module.__name__, None, all, verbs, all, exceptions, words, misspelled)
        message="All IMP methods should have lower case names separated by underscores and beginning with a verb, preferably one of ['add', 'remove', 'get', 'set', 'create', 'destroy']. Each of the words should be a properly spelled english word. The following do not (given our limited list of verbs that we check for):\n%(bad)s\nIf there is a good reason for them not to (eg it does start with a verb, just one with a meaning that is not covered by the normal list), add them to the function_name_exceptions variable in the IMPModuleTest call. Otherwise, please fix. The current verb list is %(verbs)s" \
                          % {"bad":"\n".join(bad), "verbs":verbs}
        if len(misspelled) > 0:
            message += "\nMisspelled words: " + ", ".join(set(misspelled)) \
                       + ". Add words to the spelling_exceptions variable " \
                       + "of the IMPModuleTest if needed."
        self.assertEquals(len(bad), 0,
                          message)


    def assertShow(self, modulename, exceptions):
        """Check that all the classes in modulename have a show method"""
        all= dir(modulename)
        not_found=[]
        for f in all:
            # Exclude Python-only classes; they are all showable
            if not eval('hasattr(%s.%s, "__swig_destroy__")' \
                        % (modulename.__name__, f)):
                continue
            if self._get_type(modulename.__name__, f) == types.TypeType\
                   and not f.startswith("_") \
                   and not f.endswith("_swigregister")\
                   and f not in exceptions\
                   and not f.endswith("Temp") and not f.endswith("Iterator")\
                   and not f.endswith("Exception") and\
                   f not in eval(modulename.__name__+"._raii_types") and \
                   f not in eval(modulename.__name__+"._plural_types"):
                if not hasattr(getattr(modulename, f), 'show'):
                    not_found.append(f)
        message="All IMP classes should support show and __str__. The following do not:\n%s\n If there is a good reason for them not to, add them to the show_exceptions variable in the IMPModuleTest call. Otherwise, please fix." \
                          % "\n".join(not_found)
        self.assertEquals(len(not_found), 0,
                          message)
        for e in exceptions:
            self.assertIn(e, all, "Show exception "+e+" is not a class in module")
            self.assertTrue(not hasattr(getattr(modulename, e), 'show'),
                             "Exception "+e+" is not really a show exception")

    def run_example(self, filename):
        """Run the named example script.
           A dictionary of all the script's global variables is returned.
           This can be queried in a test case to make sure the example
           performed correctly."""
        class _FatalError(Exception): pass

        # Add directory containing the example to sys.path, so it can import
        # other Python modules in the same directory
        path, name = os.path.split(filename)
        oldsyspath = sys.path[:]
        olssysargv= sys.argv[:]
        sys.path.insert(0, path)
        sys.argv=[filename]
        vars = {}
        try:
            try:
                exec open(filename) in vars
            # Catch sys.exit() called from within the example; a non-zero exit
            # value should cause the test case to fail
            except SystemExit, e:
                if e.code != 0 and e.code is not None:
                    raise _FatalError("Example exit with code %s" % str(e.code))
        finally:
            # Restore sys.path (note that Python 2.3 does not allow
            # try/except/finally, so we need to use nested trys)
            sys.path = oldsyspath
            sys.argv= olssysargv

        return _ExecDictProxy(vars)

    def run_python_module(self, module, args):
        """Run a Python module as if with "python -m <modname>",
           with the given list of arguments as sys.argv.

           If module is an already-imported Python module, run its 'main'
           function and return the result.

           If module is a string, run the module in a subprocess and return
           a subprocess.Popen-like object containing the child stdin,
           stdout and stderr.
        """
        if type(module) == type(os):
            mod = module
        else:
            mod = __import__(module, {}, {}, [''])
        modpath = mod.__file__
        if modpath.endswith('.pyc'):
            modpath = modpath[:-1]
        if type(module) == type(os):
            old_sys_argv = sys.argv
            try:
                sys.argv = [modpath] + args
                return module.main()
            finally:
                sys.argv = old_sys_argv
        else:
            return _SubprocessWrapper(sys.executable, [modpath] + args)

    def check_runnable_python_module(self, module):
        """Check a Python module designed to be runnable with 'python -m'
           to make sure it supports standard command line options."""
        # --help should return with exit 0, no errors
        r = self.run_python_module(module, ['--help'])
        out, err = r.communicate()
        self.assertEqual(r.returncode, 0)
        self.assertEqual(err, "")
        self.assertNotEqual(out, "")


class _ExecDictProxy(object):
    """exec returns a Python dictionary, which contains IMP objects, other
       Python objects, as well as base Python modules (such as sys and
       __builtins__). If we just delete this dictionary, it is entirely
       possible that base Python modules are removed from the dictionary
       *before* some IMP objects. This will prevent the IMP objects' Python
       destructors from running properly, so C++ objects will not be
       cleaned up. This class proxies the base dict class, and on deletion
       attempts to remove keys from the dictionary in an order that allows
       IMP destructors to fire."""
    def __init__(self, d):
        self._d = d
    def __del__(self):
        # Try to release example objects in a sensible order
        module_type = type(IMP)
        d = self._d
        for k in d.keys():
            if type(d[k]) != module_type:
                del d[k]

    for meth in ['__contains__', '__getitem__', '__iter__', '__len__',
                 'get', 'has_key', 'items', 'keys', 'values']:
        exec("def %s(self, *args, **keys): "
             "return self._d.%s(*args, **keys)" % (meth, meth))


class _TestResult(unittest.TextTestResult):

    def __init__(self, stream=None, descriptions=None, verbosity=None):
        super(_TestResult, self).__init__(stream, descriptions, verbosity)
        self.all_tests = []
        self._test_names = {}
        self._duplicated_tests = {}

    def stopTestRun(self):
        # Check for multiple tests which have the same name. Since tests are
        # tracked by name, duplicates will make it difficult for developers
        # to figure out which tests are failing. Report duplicates as an
        # extra test failure.
        class _DuplicateTest(object):
            def shortDescription(self):
                return 'Duplicate test names found'

        if len(self._duplicated_tests) > 0:
            self.errors.append((_DuplicateTest(),
                                'Test case names must be unique, so that '
                                'failures can be easily tracked.\n'
                                'Please rename test(s) so that they are. '
                                'The following test case names\n'
                                'are duplicated:\n' \
                                + '\n'.join(self._duplicated_tests.keys())))
        super(_TestResult, self).stopTestRun()

    def startTest(self, test):
        super(_TestResult, self).startTest(test)
        test.start_time=datetime.datetime.now()

    def _test_finished(self, test, state, detail=None):
        delta = datetime.datetime.now() - test.start_time
        try:
            pv= delta.total_seconds()
        except AttributeError:
            pv = (float(delta.microseconds) \
                  + (delta.seconds + delta.days * 24 * 3600) * 10**6) / 10**6
        if pv > 1:
            self.stream.write("in %.3fs ... " % pv)
        if detail is not None and not isinstance(detail, str):
            detail = self._exc_info_to_string(detail, test)
        test_name = self.getDescription(test)
        if test_name in self._test_names:
            self._duplicated_tests[test_name] = None
        else:
            self._test_names[test_name] = None
        self.all_tests.append({'name': test_name,
                               'time': pv, 'state': state, 'detail': detail})

    def addSuccess(self, test):
        self._test_finished(test, 'OK')
        super(_TestResult, self).addSuccess(test)

    def addError(self, test, err):
        self._test_finished(test, 'ERROR', err)
        super(_TestResult, self).addError(test, err)

    def addFailure(self, test, err):
        self._test_finished(test, 'FAIL', err)
        super(_TestResult, self).addFailure(test, err)

    def addSkip(self, test, reason):
        self._test_finished(test, 'SKIP', reason)
        super(_TestResult, self).addSkip(test, reason)

    def addExpectedFailure(self, test, err):
        self._test_finished(test, 'EXPFAIL', err)
        super(_TestResult, self).addExpectedFailure(test, err)

    def addUnexpectedSuccess(self, test):
        self._test_finished(test, 'UNEXPSUC', err)
        super(_TestResult, self).addUnexpectedSuccess(test)

    def getDescription(self, test):
        doc_first_line = test.shortDescription()
        if self.descriptions and doc_first_line:
            return doc_first_line
        else:
            return str(test)


class _TestRunner(unittest.TextTestRunner):
    def _makeResult(self):
        return _TestResult(self.stream, self.descriptions, self.verbosity)


def main(*args, **keys):
    """Run a set of tests; essentially the same as unittest.main(). Obviates
       the need to separately import the 'unittest' module, and ensures that
       main() is from the same unittest module that the IMP.test testcases
       are."""
    return unittest.main(testRunner=_TestRunner, *args, **keys)

try:
    import subprocess
    class _SubprocessWrapper(subprocess.Popen):
        def __init__(self, app, args):
            # For (non-Python) applications to work on Windows, the
            # PATH must include the directory containing built DLLs
            if sys.platform == 'win32' and app != sys.executable:
                # Hack to find the location of build/lib/
                libdir = os.environ['PYTHONPATH'].split(';')[0]
                env = os.environ.copy()
                env['PATH'] += ';' + libdir
            else:
                env = None
            subprocess.Popen.__init__(self, [app]+list(args),
                                      stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE,
                                      stderr=subprocess.PIPE, env=env)
except ImportError:
    import threading
    import popen2
    # Provide a subprocess workalike for Python 2.3 systems (e.g. old Macs)
    class _SubprocessWrapper(object):
        def __init__(self, app, args):
            self.popen = popen2.Popen3(app + " " + " ".join(args), True)
            self.stdin = self.popen.tochild
            self.stdout = self.popen.fromchild
            self.stderr = self.popen.childerr

        def _readerthread(self, fh, buffer):
            buffer.append(fh.read())

        def communicate(self, input=None):
            stdout = []
            stderr = []
            stdout_thread = threading.Thread(target=self._readerthread,
                                             args=(self.stdout, stdout))
            stdout_thread.setDaemon(True)
            stdout_thread.start()
            stderr_thread = threading.Thread(target=self._readerthread,
                                             args=(self.stderr, stderr))
            stderr_thread.setDaemon(True)
            stderr_thread.start()

            if input:
                self.stdin.write(input)
            self.stdin.close()
            stdout_thread.join()
            stderr_thread.join()
            self.returncode = self.popen.wait()
            return stdout[0], stderr[0]


class ApplicationTestCase(TestCase):
    """Super class for simple IMP application test cases"""
    def _get_application_file_name(self, filename):
        # If we ran from run-all-tests.py, it set an env variable for us with
        # the top-level test directory
        if sys.platform == 'win32':
            filename += '.exe'
        #if 'IMP_BUILD_ROOT' in os.environ:
        #    testdir = os.environ['IMP_BUILD_ROOT']
        #    return os.path.join(testdir, "build", "bin", filename)
        return filename

    def run_application(self, app, args):
        """Run an application with the given list of arguments.
           @return a subprocess.Popen-like object containing the child stdin,
                   stdout and stderr.
        """
        filename = self._get_application_file_name(app)
        if sys.platform == 'win32':
            # Cannot rely on PATH on wine builds, so use full pathname
            return _SubprocessWrapper(os.path.join(os.environ['IMP_BIN_DIR'],
                                                   filename), args)
        else:
            return _SubprocessWrapper(filename, args)

    def run_python_application(self, app, args):
        """Run a Python application with the given list of arguments.
           The Python application should be self-runnable (i.e. it should
           be executable and with a #! on the first line).
           @return a subprocess.Popen-like object containing the child stdin,
                   stdout and stderr.
        """
        # Handle platforms where /usr/bin/python doesn't work
        if sys.executable != '/usr/bin/python':
            return _SubprocessWrapper(sys.executable,
                         [os.path.join(os.environ['IMP_BIN_DIR'], app)] + args)
        else:
            return _SubprocessWrapper(app, args)

    def import_python_application(self, app):
        """Import an installed Python application, rather than running it.
           This is useful to directly test components of the application.
           @return the Python module object."""
        import imp
        return imp.load_source(os.path.splitext(app)[0],
                               os.path.join(os.environ['IMP_BIN_DIR'], app))

    def run_script(self, app, args):
        """Run an application with the given list of arguments.
           @return a subprocess.Popen-like object containing the child stdin,
                   stdout and stderr.
        """
        return _SubprocessWrapper(sys.executable, [app]+args)

    def assertApplicationExitedCleanly(self, ret, error):
        """Assert that the application exited cleanly, i.e. that the
           return value is zero."""
        if ret < 0:
            raise OSError("Application exited with signal %d\n" % -ret\
                          +error)
        else:
            self.assertEqual(ret, 0,
                       "Application exited uncleanly, with exit code %d\n" % ret\
                             + error)

    def read_shell_commands(self, doxfile):
        """Read and return a set of shell commands from a doxygen file.
           Each command is assumed to be in a \code{.sh}...\endcode block.
           The doxygen file is specified relative to the test file itself.
           This is used to make sure the commands shown in an application
           example actually work (the testcase can also check the resulting
           files for correctness)."""
        def fix_win32_command(cmd):
            # Make substitutions so a Unix shell command works on Windows
            if cmd.startswith('cp -r '):
                return 'xcopy /E ' + cmd[6:]
            elif cmd.startswith('cp '):
                return 'copy ' + cmd[3:]
            else:
                return cmd
        d = os.path.dirname(sys.argv[0])
        doc = os.path.join(d, doxfile)
        inline = False
        cmds = []
        example_path = os.path.abspath(IMP.get_example_path('..'))
        for line in open(doc).readlines():
          if '\code{.sh}' in line:
              inline = True
          elif '\endcode' in line:
              inline = False
          elif inline:
              cmds.append(line.rstrip('\r\n').replace('<imp_example_path>',
                                                      example_path))
        if sys.platform == 'win32':
            cmds = [fix_win32_command(x) for x in cmds]
        return cmds

    def run_shell_command(self, cmd):
        "Print and run a shell command, as returned by read_shell_commands()"
        import subprocess
        print cmd
        p = subprocess.call(cmd, shell=True)
        if p != 0:
            raise OSError("%s failed with exit value %d" % (cmd, p))


class RefCountChecker(object):
    """Check to make sure the number of C++ object references is as expected"""

    def __init__(self, testcase):
        # Make sure no director objects are hanging around; otherwise these
        # may be unexpectedly garbage collected later, decreasing the
        # live object count
        IMP.base._director_objects.cleanup()
        self.__testcase = testcase
        if IMP.base.get_check_level() >= IMP.base.USAGE_AND_INTERNAL:
            self.__basenum = IMP.base.RefCounted.get_number_of_live_objects()
            self.__names= IMP.base.get_live_object_names()

    def assert_number(self, expected):
        "Make sure that the number of references matches the expected value."
        t = self.__testcase
        IMP.base._director_objects.cleanup()
        if IMP.base.get_check_level() >= IMP.base.USAGE_AND_INTERNAL:
            newnames=[x for x in IMP.base.get_live_object_names() if x not in self.__names]
            newnum=IMP.base.RefCounted.get_number_of_live_objects()-self.__basenum
            t.assertEqual(newnum, expected,
                          "Number of objects don't match: "\
                           +str(newnum)\
                            +" != "+ str(expected) +" found "+\
                            str(newnames))



class DirectorObjectChecker(object):
    """Check to make sure the number of director references is as expected"""

    def __init__(self, testcase):
        IMP.base._director_objects.cleanup()
        self.__testcase = testcase
        self.__basenum = IMP.base._director_objects.get_object_count()

    def assert_number(self, expected, force_cleanup=True):
        """Make sure that the number of references matches the expected value.
           If force_cleanup is set, clean up any unused references first before
           doing the assertion.
        """
        t = self.__testcase
        if force_cleanup:
            IMP.base._director_objects.cleanup()
        t.assertEqual(IMP.base._director_objects.get_object_count() \
                      - self.__basenum, expected)

# Make sure that the IMP binary directory (build/bin) is in the PATH, if
# we're running under wine (the imppy.sh script normally ensures this, but
# wine overrides the PATH). This is needed so that tests of imported Python
# applications can successfully spawn C++ applications (e.g. idock.py tries
# to run recompute_zscore.exe). build/lib also needs to be in the PATH, since
# that's how Windows locates dependent DLLs such as libimp.dll.
if sys.platform == 'win32' and 'PYTHONPATH' in os.environ \
   and 'IMP_BIN_DIR' in os.environ:
    libdir = os.environ['PYTHONPATH'].split(';')[0]
    bindir = os.environ['IMP_BIN_DIR']
    path = os.environ['PATH']
    if libdir not in path or bindir not in path:
        os.environ['PATH'] = bindir + ';' + libdir + ';' + path
%}
