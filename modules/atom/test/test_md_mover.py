#!/usr/bin/env python

import IMP
import IMP.atom
import IMP.core
import IMP.test

xkey = IMP.FloatKey('x')
ykey = IMP.FloatKey('y')
zkey = IMP.FloatKey('z')
masskey = IMP.FloatKey('mass')
vxkey = IMP.FloatKey('vx')


# Conversion from derivatives (in kcal/mol/A) to acceleration (A/fs/fs)
kcal2mod = 4.1868e-4
# Mass of Carbon-12 (g/mol)
cmass = 12.011

class XTransRestraint(IMP.Restraint):
    """Attempt to move the whole system along the x axis"""
    def __init__(self, strength):
        IMP.Restraint.__init__(self)
        self.strength = strength

    def unprotected_evaluate(self, accum):
        e = 0.
        for p in self.get_model().get_particles():
            e += p.get_value(xkey) * self.strength
        if accum:
            for p in self.get_model().get_particles():
                p.add_to_derivative(xkey, self.strength, accum)
                p.add_to_derivative(ykey, 0.0, accum)
                p.add_to_derivative(zkey, 0.0, accum)
        return e
    def get_version_info(self):
        return IMP.VersionInfo("Daniel Russel", "0.5")
    def do_show(self, fh):
        fh.write("Test restraint")
    def get_input_particles(self):
        return [x for x in self.get_model().get_particles()]
    def get_input_containers(self):
        return []

class WriteTrajState(IMP.OptimizerState):
    """Write system coordinates (trajectory) into a Python list"""
    def __init__(self, traj):
        IMP.OptimizerState.__init__(self)
        self.traj = traj
    def update(self):
        model = self.get_optimizer().get_model()
        self.traj.append([(p.get_value(xkey), p.get_value(ykey),
                           p.get_value(zkey), p.get_value(vxkey)) \
                           for p in model.get_particles()])

class MDMoverTest(IMP.test.TestCase):
    "tests the MD mover by moving it along the x axis."
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.init_model()
        self.init_simulations()

    def init_model(self):
        IMP.set_log_level(IMP.NONE)
        m=IMP.Model()
        p2=IMP.core.XYZ.setup_particle(IMP.Particle(m))
        p2.set_coordinates(IMP.algebra.Vector3D(0,0,0))
        m2=IMP.atom.Mass.setup_particle(p2,1.0)
        p2.set_coordinates_are_optimized(True)
        si = XTransRestraint(1.0)
        m.add_restraint(si)
        self.m = m
        self.p2=p2

    def init_simulations(self):
        p2=self.p2
        #regular md
        md=IMP.atom.MolecularDynamics()
        md.set_model(self.m)
        md.set_time_step(1.0)
        #mover
        cont=IMP.container.ListSingletonContainer(self.m)
        cont.add_particle(p2)
        mdmv=IMP.atom.MDMover(cont, md, 0, 10)
        #store all
        self.mdmv = mdmv
        self.md = md

    def test_MDMover(self):
        p2=self.p2
        original = tuple(p2.get_coordinates())
        self.md.optimize(10)
        expected = tuple(p2.get_coordinates())
        p2.set_coordinates(original)
        self.mdmv.propose_move(1)
        self.assertAlmostEqual(expected,tuple(p2.get_coordinates()),delta=1e-6)
        self.mdmv.reset_move()
        self.assertAlmostEqual(original,tuple(p2.get_coordinates()),delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
