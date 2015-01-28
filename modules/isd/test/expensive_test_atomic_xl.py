import IMP
import IMP.core
import IMP.isd
import IMP.algebra
import IMP.test

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
        self.xl = IMP.isd.AtomicCrossLinkMSRestraint(self.m,
                                                     xlen,
                                                     psip.get_index(),
                                                     slope)
        self.xl.add_contribution([p0.get_index(), p1.get_index()],
                                 [self.sig0.get_index(), self.sig1.get_index()])

        self.m.add_restraint(self.xl)

    def test_atomic_xl_derivatives(self):
        """Test single-contribution derivs with no slope"""
        for i in range(10):
            self.randomize_particles([self.d0, self.d1], 2)
            self.m.evaluate(True)
            #print 'n', IMP.test.xyz_numerical_derivatives(self.m, self.d0, 0.01), 'a', self.d0.get_derivatives()
            self.assertXYZDerivativesInTolerance(
                self.m,
                self.d0,
                tolerance=1e-2,
                percentage=3.0)

            self.assertXYZDerivativesInTolerance(
                self.m,
                self.d1,
                tolerance=1e-2,
                percentage=3.0)


    def test_atomic_xl_derivatives_with_slope(self):
        """Test single-contribution derivs with slope"""
        self.xl.set_slope(0.1)
        for i in range(10):
            self.randomize_particles([self.d0, self.d1], 2)
            self.m.evaluate(True)
            self.assertXYZDerivativesInTolerance(
                self.m,
                self.d0,
                tolerance=1e-2,
                percentage=3.0)
            self.assertXYZDerivativesInTolerance(
                self.m,
                self.d1,
                tolerance=1e-2,
                percentage=3.0)


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
        self.xl = IMP.isd.AtomicCrossLinkMSRestraint(self.m,
                                                     xlen,
                                                     psip.get_index(),
                                                     slope,
                                                     False)

        # make pairs and add contributions
        pairs = [[i, i + 1] for i in range(9)]
        for pp in pairs:
            self.xl.add_contribution(
                [ps[pp[0]].get_index(), ps[pp[1]].get_index()],
                [psigs[pp[0]].get_index(),
                 psigs[pp[1]].get_index()])

    def test_atomic_xl_derivatives_many(self):
        """ test the atomix XL restraint derivatives"""
        self.m.add_restraint(self.xl)
        for i in range(10):
            self.randomize_particles(self.xyzs, 6)
            self.m.evaluate(True)
            for d in self.xyzs:
                self.assertXYZDerivativesInTolerance(
                    self.m,
                    d,
                    tolerance=1e-2,
                    percentage=5.)

    def test_log_derivatives(self):
        """ test the atomix XL restraint derivatives when in a LogWrapper"""
        self.xl.set_part_of_log_score(True)
        log_wrapper = IMP.isd.LogWrapper([self.xl], 1.0)
        self.m.add_restraint(log_wrapper)
        for i in range(10):
            self.randomize_particles(self.xyzs, 6)
            self.m.evaluate(True)
            for d in self.xyzs:
                #print 'n', IMP.test.xyz_numerical_derivatives(self.m, d, 0.01), 'a', d.get_derivatives()
                self.assertXYZDerivativesInTolerance(
                    self.m,
                    d,
                    tolerance=1e-2,
                    percentage=5.)

if __name__ == '__main__':
    IMP.test.main()
