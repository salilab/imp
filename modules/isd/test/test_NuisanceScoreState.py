#!/usr/bin/env python

# general imports
from numpy import *
from random import uniform
import pickle

# imp general
import IMP
import IMP.core

# our project
from IMP.isd import Nuisance, Scale, Switching

# unit testing framework
import IMP.test


class XTransRestraint(IMP.Restraint):

    def __init__(self, m):
        IMP.Restraint.__init__(self, m, "XTransRestraint %1%")

    def unprotected_evaluate(self, accum):
        m = self.get_model()
        e = 0
        self.values = [m.get_attribute(Nuisance.get_nuisance_key(), p)
                       for p in m.get_particle_indexes()]
        return e

    def get_version_info(self):
        return IMP.VersionInfo("", "")

    def do_show(self, fh):
        fh.write("Test restraint")

    def do_get_inputs(self):
        m = self.get_model()
        return IMP.get_particles(m, m.get_particle_indexes())


class TestNuisanceScoreState(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(0)
        self.m = IMP.Model()
        self.rs = XTransRestraint(self.m)

    def test_nuisance_get_has_upper(self):
        p = IMP.Particle(self.m)
        n = Nuisance.setup_particle(p, 1.0)
        self.assertFalse(n.get_has_upper())
        n.set_upper(0.5)
        self.assertTrue(n.get_has_upper())
        n.remove_upper()
        self.assertFalse(n.get_has_upper())

    def test_nuisance_get_has_lower(self):
        p = IMP.Particle(self.m)
        n = Nuisance.setup_particle(p, 1.0)
        self.assertFalse(n.get_has_lower())
        n.set_lower(0.5)
        self.assertTrue(n.get_has_lower())
        n.remove_lower()
        self.assertFalse(n.get_has_lower())

    def test_nuisance_up(self):
        n = IMP.Particle(self.m)
        Nuisance.setup_particle(n, 1.0)
        Nuisance(n).set_lower(0.5)
        Nuisance(n).set_upper(1.5)
        n.set_value(Nuisance.get_nuisance_key(), 10.0)
        self.rs.evaluate(False)
        self.assertAlmostEqual(n.get_value(Nuisance.get_nuisance_key()),
                               1.5, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0], 1.5)

    def test_nuisance_down(self):
        n = IMP.Particle(self.m)
        Nuisance.setup_particle(n, 1.0)
        Nuisance(n).set_lower(0.5)
        Nuisance(n).set_upper(1.5)
        n.set_value(Nuisance.get_nuisance_key(), 0.1)
        self.rs.evaluate(False)
        self.assertAlmostEqual(n.get_value(Nuisance.get_nuisance_key()),
                               0.5, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0], 0.5)

    def test_scale_up(self):
        n = IMP.Particle(self.m)
        Scale.setup_particle(n, 1.0)
        Scale(n).set_upper(1.5)
        n.set_value(Scale.get_scale_key(), 10.0)
        self.rs.evaluate(False)
        self.assertAlmostEqual(n.get_value(Scale.get_scale_key()),
                               1.5, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0], 1.5)

    def test_scale_down(self):
        n = IMP.Particle(self.m)
        Scale.setup_particle(n, 1.0)
        n.set_value(Scale.get_scale_key(), -0.1)
        self.rs.evaluate(False)
        self.assertAlmostEqual(n.get_value(Scale.get_scale_key()),
                               0.0, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0], 0.0)

    def test_switching_up(self):
        n = IMP.Particle(self.m)
        Switching.setup_particle(n, 0.3)
        n.set_value(Switching.get_switching_key(), 3)
        self.rs.evaluate(False)
        self.assertAlmostEqual(n.get_value(Switching.get_switching_key()),
                               1.0, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0], 1.0)

    def test_switching_down(self):
        n = IMP.Particle(self.m)
        Switching.setup_particle(n, 0.3)
        n.set_value(Switching.get_switching_key(), -1)
        self.rs.evaluate(False)
        self.assertAlmostEqual(n.get_value(Switching.get_switching_key()),
                               0.0, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0], 0.0)

    def test_NormalMover_MC_ok(self):
        "Test nuisance scorestate with MonteCarlo mover"
        nuis = Nuisance.setup_particle(IMP.Particle(self.m), 50.0)
        lower = Nuisance.setup_particle(IMP.Particle(self.m), 10.0)
        upper = Nuisance.setup_particle(IMP.Particle(self.m), 90.0)
        nuis.set_nuisance_is_optimized(True)
        nuis.set_lower(1.0)
        nuis.set_lower(lower)
        nuis.set_upper(upper)
        nuis.set_upper(80.0)
        nmv = IMP.core.NormalMover([nuis],
                                   IMP.FloatKeys([IMP.FloatKey("nuisance")]), 10.0)
        mc = IMP.core.MonteCarlo(self.m)
        mc.set_scoring_function([self.rs])
        mc.set_return_best(False)
        mc.set_kt(1.0)
        mc.add_mover(nmv)
        for i in range(100):
            mc.optimize(10)
            self.assertTrue(nuis.get_nuisance() >= nuis.get_lower()
                            and nuis.get_nuisance() <= nuis.get_upper())

    @IMP.test.skip("ScoreState won't be called with no restraints that use it")
    def test_NormalMover_MC_fails(self):
        "Test nuisance scorestate with MonteCarlo mover"
        nuis = Nuisance.setup_particle(IMP.Particle(self.m), 50.0)
        lower = Nuisance.setup_particle(IMP.Particle(self.m), 10.0)
        upper = Nuisance.setup_particle(IMP.Particle(self.m), 90.0)
        nuis.set_lower(1.0)
        nuis.set_lower(lower)
        nuis.set_upper(upper)
        nuis.set_upper(80.0)
        nuis.set_nuisance_is_optimized(True)
        nmv = IMP.core.NormalMover([nuis],
                                   IMP.FloatKeys([IMP.FloatKey("nuisance")]), 10.0)
        mc = IMP.core.MonteCarlo(self.m)
        mc.set_return_best(False)
        mc.set_kt(1.0)
        mc.add_mover(nmv)
        for i in range(100):
            mc.optimize(10)
            self.assertTrue(nuis.get_nuisance() >= nuis.get_lower()
                            and nuis.get_nuisance() <= nuis.get_upper())

    def test_pickle_polymorphic(self):
        """Test (un-)pickle of NuisanceScoreState via polymorphic pointer"""
        nuis = Nuisance.setup_particle(IMP.Particle(self.m), 50.0)
        nuis.set_nuisance(2.)
        nuis.set_upper(1.)
        # NuisanceScoreState is not exposed to Python, but it should be in
        # the model
        dump = pickle.dumps(self.m)
        newm = pickle.loads(dump)
        # New model should contain an identical Nuisance particle
        newnuisp, = newm.get_particle_indexes()
        self.assertTrue(Nuisance.get_is_setup(newm, newnuisp))
        newnuis = Nuisance(newm, newnuisp)
        self.assertAlmostEqual(newnuis.get_nuisance(), 2., delta=1e-5)
        # Updating the original model should call NuisanceScoreState on it,
        # which should enforce the upper bound - but the new model should
        # not be affected:
        self.m.update()
        self.assertAlmostEqual(nuis.get_nuisance(), 1., delta=1e-5)
        self.assertAlmostEqual(newnuis.get_nuisance(), 2., delta=1e-5)
        # Updating the new model should enforce the upper bound there:
        newm.update()
        self.assertAlmostEqual(newnuis.get_nuisance(), 1., delta=1e-5)


if __name__ == '__main__':
    IMP.test.main()
