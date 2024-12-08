import IMP
import IMP.test
import IMP.core
import IMP.container


class WoodsFunc(IMP.Restraint):

    """Woods function for four input values, defined as an IMP restraint"""

    def __init__(self, m):
        IMP.Restraint.__init__(self, m, "WoodsFunc%1%")
        self.index = IMP.FloatKey("x")

    def unprotected_evaluate(self, accum):
        m = self.get_model()
        # print "Evaluating in python\n"
        (x1, x2, x3, x4) = [m.get_attribute(self.index, p)
                            for p in m.get_particle_indexes()]
        # print "Evaluating at " +str(x1) + " " + str(x2) + " " \
        #    + str(x3) + " " + str(x4)
        a = x2 - x1 * x1
        b = x4 - x3 * x3
        e = 100.0 * a * a + (1.0 - x1) ** 2 + 90.0 * b * b + (1.0 - x3) ** 2 \
            + 10.1 * ((x2 - 1.0) ** 2 + (x4 - 1.0) ** 2) \
            + 19.8 * (x2 - 1.0) * (x4 - 1.0)
        if accum:
            dx = [-2.0 * (200.0 * x1 * a + 1.0 - x1),
                  2.0 * (100.0 * a + 10.1 * (x2 - 1.0) + 9.9 * (x4 - 1.0)),
                  -2.0 * (180.0 * x3 * b + 1.0 - x3),
                  2.0 * (90.0 * b + 10.1 * (x4 - 1.0) + 9.9 * (x2 - 1.0))]
            for (pi, d) in zip(m.get_particle_indexes(), dx):
                p = m.get_particle(pi)
                p.add_to_derivative(self.index, d, accum)
        return e

    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")

    def do_show(self, fh):
        fh.write("WoodsFunc")

    def do_get_inputs(self):
        m = self.get_model()
        return IMP.get_particles(m, m.get_particle_indexes())


class MCOptimizerTest(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)
        self.xkey = IMP.FloatKey("x")

    def test_c1(self):
        """Test montecarlo from starting position 1"""
        (model, opt) = self._setup_opt()
        opt.set_log_level(IMP.VERBOSE)
        self._test_starting_conditions(model, opt, (-3.0, -1.0, -3.0, -1.0), 5)

    def test_c2(self):
        """Test montecarlo from starting position 2"""
        (model, opt) = self._setup_opt()
        self._test_starting_conditions(model, opt, (2.0, 3.0, 8.0, -5.0), 5)

    def test_c3(self):
        """Test montecarlo from starting position 3"""
        (model, opt) = self._setup_opt()
        self._test_starting_conditions(model, opt, (2.0, 3.0, 8.0, -5.0), 5)

    def _setup_opt(self):
        model = IMP.Model()
        lopt = IMP.core.ConjugateGradients(model)
        opt = IMP.core.MonteCarloWithLocalOptimization(lopt, 1000)
        opt.set_score_threshold(.01)
        ps = []
        for value in (-3.0, -1.0, -3.0, -1.0):
            p = IMP.Particle(model)
            p.add_attribute(self.xkey, value, True)
            ps.append(p)
        fk = IMP.FloatKeys([self.xkey])
        mod = IMP.core.BallMover(model, ps, fk, .25)
        opt.add_mover(mod)

        rsr = WoodsFunc(model)
        # print rsr.evaluate(None)
        # print model.evaluate(False)
        # model.show()
        # print "Restaint TO is "+str(rsr.thisown)
        # opt.show()
        sf = IMP.core.RestraintsScoringFunction([rsr])
        lopt.set_scoring_function(sf)
        opt.set_scoring_function(sf)
        return (model, opt)

    def _test_starting_conditions(self, model, opt, starting_values, nrounds):
        """Test the optimizer with given starting conditions"""

        print("Start energy is "
              + str(opt.get_scoring_function().evaluate(False)))
        for i in range(0, nrounds):
            e = opt.optimize(100)
            print("Energy after step is " + str(e))

        for p in model.get_particle_indexes():
            val = model.get_attribute(self.xkey, p)
            self.assertAlmostEqual(val, 1.0, delta=.1)
        self.assertAlmostEqual(e, 0.0, delta=.01)

if __name__ == '__main__':
    IMP.test.main()
