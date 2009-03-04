import unittest
import IMP, IMP.test
import IMP.core

class WoodsFunc(IMP.Restraint):
    """Woods function for four input values, defined as an IMP restraint"""
    def __init__(self):
        IMP.Restraint.__init__(self)
        self.index= IMP.FloatKey("x")
    def evaluate(self, accum):
        #print "Evaluating in python\n"
        (x1, x2, x3, x4) = [p.get_value(self.index) for p in self.get_model().get_particles()]
        #print "Evaluating at " +str(x1) + " " + str(x2) + " " \
        #    + str(x3) + " " + str(x4)
        a = x2 - x1 * x1
        b = x4 - x3 * x3
        e = 100.0 * a * a + (1.0 - x1) ** 2+ 90.0 * b * b + (1.0 - x3) ** 2 \
            + 10.1 * ((x2 - 1.0) ** 2 + (x4 - 1.0) ** 2) \
            + 19.8 * (x2 - 1.0) * (x4 - 1.0)
        if accum:
            dx = [-2.0 * (200.0 * x1 * a + 1.0 - x1),
                   2.0 * (100.0 * a + 10.1 * (x2 - 1.0) + 9.9 * (x4 - 1.0)),
                   -2.0 * (180.0 * x3 * b + 1.0 - x3),
                   2.0 * (90.0 * b + 10.1 * (x4 - 1.0) + 9.9 * (x2 - 1.0))]
            for (p,d) in zip(self.get_model().get_particles(), dx):
                p.add_to_derivative(self.index, d, accum);
        return e
    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")
    def show(self, fakestream):
        print "WoodsFunc"

class MCOptimizerTest(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.TERSE)
        self.xkey= IMP.FloatKey("x")
        self.rsrs=[]

    def test_c1(self):
        """test montecarlo 1"""
        (model, opt)= self._setup_opt()
        lopt= IMP.core.ConjugateGradients()
        opt.set_local_optimizer(lopt)
        self._test_starting_conditions(model, opt, (-3.0, -1.0, -3.0, -1.0), 5)
    def test_c2(self):
        """test montecarlo 2"""
        (model, opt)= self._setup_opt()
        lopt= IMP.core.ConjugateGradients()
        opt.set_local_optimizer(lopt)
        self._test_starting_conditions(model, opt, (2.0, 3.0, 8.0, -5.0), 5)
    def test_c3(self):
        """test montecarlo 3"""
        (model, opt)= self._setup_opt()
        lopt= IMP.core.ConjugateGradients()
        opt.set_local_optimizer(lopt)
        self._test_starting_conditions(model, opt, (2.0, 3.0, 8.0, -5.0), 5)

    def _setup_opt(self):
        model = IMP.Model()
        opt = IMP.core.MonteCarlo()
        opt.set_model(model)
        for value in (-3.0, -1.0, -3.0, -1.0):
            p = IMP.Particle(model)
            p.add_attribute(self.xkey, value, True)
        fk=IMP.FloatKeys([self.xkey])
        ls= IMP.core.ListSingletonContainer(model.get_particles())
        mod= IMP.core.BallMover(ls, fk, .25)
        opt.add_mover(mod)

        rsr = WoodsFunc()
        self.rsrs.append(rsr)
        model.add_restraint(rsr)
        #print rsr.evaluate(None)
        #print model.evaluate(False)
        #model.show()
        #print "Restaint TO is "+str(rsr.thisown)
        #opt.show()
        return (model,opt)

    def _test_starting_conditions(self, model, opt, starting_values, nrounds):
        """Test the optimizer with given starting conditions"""

        print "Start energy is " + str(model.evaluate(False))
        for i in range(0,nrounds):
            e=opt.optimize(100)
            print "Energy after step is " + str(e)
            for p in model.get_particles():
                val = p.get_value(self.xkey)
                #print "Particle " + str(p.get_name()) +\
                #    " is at " + str(val)

        for p in model.get_particles():
            val = p.get_value(self.xkey)
            self.assertAlmostEqual(val, 1.0, places=1)
        self.assertAlmostEqual(e, 0.0, places=2)

if __name__ == '__main__':
    unittest.main()
