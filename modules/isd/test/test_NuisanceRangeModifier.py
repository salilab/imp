#!/usr/bin/env python

#general imports
from numpy import *
from random import uniform


#imp general
import IMP
import IMP.core

#our project
from IMP.isd import Nuisance,Scale,Switching,NuisanceRangeModifier

#unit testing framework
import IMP.test

class XTransRestraint(IMP.Restraint):
    def __init__(self):
        IMP.Restraint.__init__(self)

    def unprotected_evaluate(self, accum):
        e=0
        self.values = [ p.get_value(Nuisance.get_nuisance_key())
                for p in self.get_model().get_particles()]
        return e

    def get_version_info(self):
        return IMP.VersionInfo("","")
    def do_show(self, fh):
        fh.write("Test restraint")
    def get_input_particles(self):
        return [x for x in self.get_model().get_particles()]
    def get_input_containers(self):
        return []

class Tests(IMP.test.TestCase):

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(0)
        self.m = IMP.Model()
        self.rs = XTransRestraint()
        self.m.add_restraint(self.rs)

    def test_nuisance_up(self):
        n=IMP.Particle(self.m)
        Nuisance.setup_particle(n,1.0)
        Nuisance(n).set_lower(0.5)
        Nuisance(n).set_upper(1.5)
        self.m.add_score_state(IMP.core.SingletonConstraint(
            IMP.isd.NuisanceRangeModifier(), None, n))
        n.set_value(Nuisance.get_nuisance_key(), 10.0)
        self.m.evaluate(False)
        self.assertAlmostEqual(n.get_value(Nuisance.get_nuisance_key()),
            1.5, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0],1.5)

    def test_nuisance_down(self):
        n=IMP.Particle(self.m)
        Nuisance.setup_particle(n,1.0)
        Nuisance(n).set_lower(0.5)
        Nuisance(n).set_upper(1.5)
        self.m.add_score_state(IMP.core.SingletonConstraint(
            IMP.isd.NuisanceRangeModifier(), None, n))
        n.set_value(Nuisance.get_nuisance_key(), 0.1)
        self.m.evaluate(False)
        self.assertAlmostEqual(n.get_value(Nuisance.get_nuisance_key()),
            0.5, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0],0.5)

    def test_scale_up(self):
        n=IMP.Particle(self.m)
        Scale.setup_particle(n,1.0)
        Scale(n).set_upper(1.5)
        self.m.add_score_state(IMP.core.SingletonConstraint(
            IMP.isd.NuisanceRangeModifier(), None, n))
        n.set_value(Scale.get_scale_key(), 10.0)
        self.m.evaluate(False)
        self.assertAlmostEqual(n.get_value(Scale.get_scale_key()),
            1.5, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0],1.5)

    def test_scale_down(self):
        n=IMP.Particle(self.m)
        Scale.setup_particle(n,1.0)
        self.m.add_score_state(IMP.core.SingletonConstraint(
            IMP.isd.NuisanceRangeModifier(), None, n))
        n.set_value(Scale.get_scale_key(), -0.1)
        self.m.evaluate(False)
        self.assertAlmostEqual(n.get_value(Scale.get_scale_key()),
            0.0, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0],0.0)

    def test_switching_up(self):
        n=IMP.Particle(self.m)
        Switching.setup_particle(n,0.3)
        self.m.add_score_state(IMP.core.SingletonConstraint(
            IMP.isd.NuisanceRangeModifier(), None, n))
        n.set_value(Switching.get_switching_key(), 3)
        self.m.evaluate(False)
        self.assertAlmostEqual(n.get_value(Switching.get_switching_key()),
            1.0, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0],1.0)

    def test_switching_down(self):
        n=IMP.Particle(self.m)
        Switching.setup_particle(n,0.3)
        self.m.add_score_state(IMP.core.SingletonConstraint(
            IMP.isd.NuisanceRangeModifier(), None, n))
        n.set_value(Switching.get_switching_key(), -1)
        self.m.evaluate(False)
        self.assertAlmostEqual(n.get_value(Switching.get_switching_key()),
            0.0, delta=1e-7)
        self.assertAlmostEqual(self.rs.values[0],0.0)

if __name__ == '__main__':
    IMP.test.main()
