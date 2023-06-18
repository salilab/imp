import IMP
import IMP.core
import IMP.isd
import IMP.algebra
import IMP.test
import pickle


def score_1state(p0, p1):
    pass

class TestAtomicXL_1State(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()

        # create two test points
        p0 = IMP.Particle(self.m)
        p1 = IMP.Particle(self.m)
        self.d0 = IMP.core.XYZ.setup_particle(p0, [0, 0, 0])
        self.d1 = IMP.core.XYZ.setup_particle(p1, [2, 0, 0])

        # create nuisance parameters
        self.sig0 = IMP.Particle(self.m)
        self.sig1 = IMP.Particle(self.m)
        psip = IMP.Particle(self.m)
        IMP.isd.Scale.setup_particle(self.sig0, 0.9)
        IMP.isd.Scale.setup_particle(self.sig1, 0.9)
        IMP.isd.Scale.setup_particle(psip, 0.2)

        # create restraint
        xlen = 10
        slope = 0.0
        self.xl = IMP.isd.AtomicCrossLinkMSRestraint(self.m, xlen, psip, slope)
        self.xl.add_contribution([p0, p1], [self.sig0, self.sig1])
        self.sf = IMP.core.RestraintsScoringFunction([self.xl])

    def test_atomic_xl_derivatives(self):
        """Test single-contribution derivs with no slope"""
        for i in range(10):
            self.randomize_particles([self.d0, self.d1], 2)
            self.sf.evaluate(True)
            #print 'n', IMP.test.xyz_numerical_derivatives(self.m, self.d0, 0.01), 'a', self.d0.get_derivatives()
            self.assertXYZDerivativesInTolerance(
                self.sf,
                self.d0,
                tolerance=1e-2,
                percentage=3.0)

            self.assertXYZDerivativesInTolerance(
                self.sf,
                self.d1,
                tolerance=1e-2,
                percentage=3.0)


    def test_atomic_xl_derivatives_with_slope(self):
        """Test single-contribution derivs with slope"""
        self.xl.set_slope(0.1)
        for i in range(10):
            self.randomize_particles([self.d0, self.d1], 2)
            self.sf.evaluate(True)
            self.assertXYZDerivativesInTolerance(
                self.sf,
                self.d0,
                tolerance=1e-2,
                percentage=3.0)
            self.assertXYZDerivativesInTolerance(
                self.sf,
                self.d1,
                tolerance=1e-2,
                percentage=3.0)

    def test_serialize(self):
        """Test (un-)serialize of AtomicCrossLinkMSRestraint"""
        self.assertAlmostEqual(self.xl.evaluate(False), 0.22314, delta=1e-4)
        self.xl.set_name("foo")
        dump = pickle.dumps(self.xl)
        newxl = pickle.loads(dump)
        self.assertEqual(newxl.get_name(), "foo")
        self.assertAlmostEqual(newxl.evaluate(False), 0.22314, delta=1e-4)

    def test_serialize_polymorphic(self):
        """Test (un-)serialize of AtomicCrossLinkMSRestraint via polymorphic
           pointer"""
        self.xl.set_name("foo")
        dump = pickle.dumps(self.sf)
        newsf = pickle.loads(dump)
        newxl, = newsf.restraints
        self.assertEqual(newxl.get_name(), "foo")
        self.assertAlmostEqual(newxl.evaluate(False), 0.22314, delta=1e-4)


class TestAtomicXL_ManyState(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.m = IMP.Model()

        # create 10
        ps = []
        psigs = []
        self.xyzs = []
        for i in range(10):
            p = IMP.Particle(self.m)
            xyz = IMP.core.XYZ.setup_particle(p, [0, 0, 0])
            ps.append(p)
            self.xyzs.append(xyz)
            psig = IMP.Particle(self.m)
            IMP.isd.Scale.setup_particle(psig, 1)
            psigs.append(psig)

        # create restraint
        xlen = 10
        slope = 0.01
        psip = IMP.Particle(self.m)
        IMP.isd.Scale.setup_particle(psip, 1)
        self.xl = IMP.isd.AtomicCrossLinkMSRestraint(self.m, xlen, psip, slope,
                                                     False)

        # make pairs and add contributions
        pairs = [[i, i + 1] for i in range(9)]
        for pp in pairs:
            self.xl.add_contribution(
                [ps[pp[0]], ps[pp[1]]], [psigs[pp[0]], psigs[pp[1]]])

    def test_atomic_xl_derivatives_many(self):
        """ test the atomix XL restraint derivatives"""
        self.sf = IMP.core.RestraintsScoringFunction([self.xl])
        for i in range(10):
            self.randomize_particles(self.xyzs, 6)
            self.sf.evaluate(True)
            for d in self.xyzs:
                self.assertXYZDerivativesInTolerance(
                    self.sf,
                    d,
                    tolerance=1e-2,
                    percentage=5.)

    def test_log_derivatives(self):
        """ test the atomix XL restraint derivatives when in a LogWrapper"""
        self.xl.set_part_of_log_score(True)
        log_wrapper = IMP.isd.LogWrapper([self.xl], 1.0)
        self.sf = IMP.core.RestraintsScoringFunction([log_wrapper])
        for i in range(10):
            self.randomize_particles(self.xyzs, 6)
            self.sf.evaluate(True)
            for d in self.xyzs:
                #print 'n', IMP.test.xyz_numerical_derivatives(self.m, d, 0.01), 'a', d.get_derivatives()
                self.assertXYZDerivativesInTolerance(
                    self.sf,
                    d,
                    tolerance=1e-2,
                    percentage=5.)

if __name__ == '__main__':
    IMP.test.main()
