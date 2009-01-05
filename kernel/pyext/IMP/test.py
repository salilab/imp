import re, math, unittest
import os
import random
import IMP


class TestCase(unittest.TestCase):
    """Super class for IMP test cases"""

    def setUp(self):
        self.__check_level = IMP.get_check_level()
        # Turn on expensive runtime checks while running the test suite:
        IMP.set_check_level(IMP.EXPENSIVE)

    def tearDown(self):
        # Restore original check level
        IMP.set_check_level(self.__check_level)

    def get_test_file(self, filename):
        """Get the full name of a file in the top-level test directory."""
        # If individual tests are run manually, assume they are in
        # the current directory:
        path = os.environ.get('TEST_DIRECTORY', '.')
        return os.path.join(path, filename)

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
            # Simple finite difference approximation
            offset= step/1024
            vmn= func.evaluate(f-offset)
            vmx= func.evaluate(f+offset)
            da= (vmx-vmn)/(2*offset)
            self.assertInTolerance(d, da, abs(.1 *d)+.0001)

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
        lbv=IMP.Vector3D(lb[0],lb[1],lb[2])
        ubv=IMP.Vector3D(ub[0],ub[1],ub[2])
        ps= IMP.Particles()
        for i in range(0,num):
            v = IMP.random_vector_in_box(lbv, ubv)
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


class TestParticleRefiner(IMP.ParticleRefiner):
    """A class which makes sure that the right particles are passed back"""

    def __init__(self, pr):
        IMP.ParticleRefiner.__init__(self)
        self.pr= pr
        self.dict={}

    def show(self, junk):
        print "Testing particle refiner"
        self.pr.show()

    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")

    def get_can_refine(self, p):
        print "Can refine? "+ str(p.get_index().get_index())
        print "Return is " + str(self.pr.get_can_refine(p))
        return self.pr.get_can_refine(p)

    def get_refined(self, p):
        print "Refining "+ str(p.get_index().get_index())
        ps= self.pr.get_refined(p)
        self.dict[p.get_index().get_index()] = ps
        print self.dict
        return ps

    def cleanup_refined(self, p, ps, da):
        # test breaks if refine is called 2x with the same particle
        print "starting cleanup "+str( p.get_index().get_index() )
        print self.dict
        if not self.pr.get_can_refine(p):
            print "cleanup the unrefined"
            raise ValueError('Cleanup the unrefined')
        if not self.dict.has_key(p.get_index().get_index()):
            print "Missing particle info"
            raise ValueError("Missing particle info")
        ops= self.dict[p.get_index().get_index()]
        print "fetched"
        if len(ops) != len(ps):
            raise ValueError("Cached particles and returned particles " + \
                             "don't match on size")
        for i in range(0, len(ops)):
            if ps[i].get_index() != ops[i].get_index():
                raise ValueError("Cached particles and returned particles " + \
                                 "don't match")
        self.pr.cleanup_refined(p, ps)
