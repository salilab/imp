import re, math, unittest
import sys
import os
import random
import IMP

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


class TestCase(unittest.TestCase):
    """Super class for IMP test cases"""

    def setUp(self):
        self.__check_level = IMP.get_check_level()
        # Turn on expensive runtime checks while running the test suite:
        IMP.set_check_level(IMP.EXPENSIVE)

    def tearDown(self):
        # Restore original check level
        IMP.set_check_level(self.__check_level)

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
            dirs = os.path.dirname(sys.argv[0]).split(os.path.sep)
            for i in range(len(dirs), 0, -1):
                input = os.path.sep.join(dirs[:i] + ['input'])
                if os.path.isdir(input):
                    return os.path.join(input, filename)
        # If not found, default to the current working directory:
        return os.path.join('input', filename)


    def get_tmp_file_name(self, filename):
        """Get the full name of an output file in the build/tmp directory."""
        # Otherwise, search up from the test's directory until we find
        # the input directory
        dir = os.path.abspath(sys.argv[0])
        while len(dir) != 0:
            input = os.path.join(dir, 'build')
            #print input
            if os.path.isdir(input):
                input= os.path.join(input, 'tmp')
                if not os.path.isdir(input):
                    os.mkdir(input)
                return os.path.join(input, filename)
            dir= os.path.split(dir)[0]
        # If not found, default to the current working directory:
        return filename

    def assertInTolerance(self, num1, num2, tolerance, msg=None):
        """Assert that the difference between num1 and num2 is less than
           tolerance"""
        diff = abs(num1 - num2)
        if msg is None:
            msg = "%f != %f within %g" % (num1, num2, tolerance)
        self.assert_(diff < tolerance, msg)

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
        # Note: cannot use XYZDecorator here since that pulls in IMP.core
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
            self.assertInTolerance(d, da, max(abs(.1 *d), 0.01))

    def check_unary_function_min(self, func, lb, ub, step, expected_fmin):
        """Make sure that the minimum of the unary function func over the
           range between lb and ub is at expected_fmin"""
        fmin, vmin = lb, func.evaluate(lb)
        for f in [lb + i * step for i in range(1, int((ub-lb)/step))]:
            v = func.evaluate(f)
            if v < vmin:
                fmin, vmin = f, v
        self.assertInTolerance(fmin, expected_fmin, step)

    def create_particles_in_box(self, model, num=10,
                                lb= [0,0,0],
                                ub= [10,10,10]):
        """Create a bunch of particles in a box"""
        import IMP.algebra
        lbv=IMP.algebra.Vector3D(lb[0],lb[1],lb[2])
        ubv=IMP.algebra.Vector3D(ub[0],ub[1],ub[2])
        ps= IMP.Particles()
        for i in range(0,num):
            v = IMP.algebra.random_vector_in_box(lbv, ubv)
            p = self.create_point_particle(model, v[0], v[1], v[2])
            ps.append(p)
        return ps


class ConstPairScore(IMP.PairScore):
    def __init__(self, v):
        IMP.PairScore.__init__(self)
        self.v=v
    def evaluate(self, pa, pb, da):
        return self.v
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")
    def show(self, t):
        print "ConstPairScore "+ str(self.v)


class ConstUnaryFunction(IMP.UnaryFunction):
    def __init__(self, v):
        IMP.UnaryFunction.__init__(self)
        self.v=v
    def evaluate(self, feat):
        return self.v
    def evaluate_with_derivative(self, feat):
        return self.v, 0.0
    def show(self, *args):
        print "ConstUF with value "+str(self.v)
    def get_version_info(self):
        return IMP.VersionInfo("Me", "0.5")


class TestRefiner(IMP.Refiner):
    """A class which makes sure that the right particles are passed back"""

    def __init__(self, pr):
        IMP.Refiner.__init__(self)
        self.pr= pr
        self.dict={}

    def show(self, junk):
        print "Testing particle refiner"
        self.pr.show()

    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")

    def get_can_refine(self, p):
        print "Can refine? "+ str(p.get_name())
        print "Return is " + str(self.pr.get_can_refine(p))
        return self.pr.get_can_refine(p)

    def get_refined(self, p):
        print "Refining "+ str(p.get_name())
        ps= self.pr.get_refined(p)
        self.dict[p.get_name()] = ps
        print self.dict
        return ps


class RefCountChecker(object):
    """Check to make sure the number of C++ object references is as expected"""

    def __init__(self, testcase):
        # Make sure no director objects are hanging around; otherwise these
        # may be unexpectedly garbage collected later, decreasing the
        # live object count
        IMP._director_objects.cleanup()
        self.__testcase = testcase
        self.__basenum = IMP.RefCounted.get_number_of_live_objects()

    def assert_number(self, expected):
        t = self.__testcase
        t.assertEqual(IMP.RefCounted.get_number_of_live_objects() \
                      - self.__basenum, expected)


class DirectorObjectChecker(object):
    """Check to make sure the number of director references is as expected"""

    def __init__(self, testcase):
        IMP._director_objects.cleanup()
        self.__testcase = testcase
        self.__basenum = IMP._director_objects.get_object_count()

    def assert_number(self, expected, force_cleanup=True):
        t = self.__testcase
        if force_cleanup:
            IMP._director_objects.cleanup()
        t.assertEqual(IMP._director_objects.get_object_count() \
                      - self.__basenum, expected)
