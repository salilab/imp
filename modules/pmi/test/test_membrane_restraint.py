import IMP
import IMP.atom
import IMP.pmi
import IMP.test


class MembraneRestraintPrototype(IMP.Restraint):
    import math
    import IMP.isd
    import sys

    def __init__(self, m, z_nuisance, thickness=30.0, softness=3.0, plateau=0.0000000001, linear=0.02):
        '''
        input a list of particles, the slope and theta of the sigmoid potential
        theta is the cutoff distance for a protein-protein contact
        '''
        IMP.Restraint.__init__(self, m, "MembraneRestraintPrototype_ %1%")
        self.thickness = thickness
        self.z_nuisance = z_nuisance
        self.softness = softness
        self.plateau = plateau
        self.particle_list_below = []
        self.particle_list_above = []
        self.particle_list_inside = []
        self.max_float = self.sys.float_info.max
        self.log_max_float = self.math.log(self.max_float)
        self.linear = linear

    def add_particles_below(self, particles):
        self.particle_list_below += particles

    def add_particles_above(self, particles):
        self.particle_list_above += particles

    def add_particles_inside(self, particles):
        self.particle_list_inside += particles

    def score_above(self, z):
        argvalue = (z - self.z_slope_center_upper) / self.softness
        prob = (1.0 - self.plateau) / (1.0 + self.math.exp(-argvalue))
        return -self.math.log(prob * self.max_float) + self.log_max_float

    def score_below(self, z):
        argvalue = (z - self.z_slope_center_lower) / self.softness
        prob = (1.0 - self.plateau) / (1.0 + self.math.exp(argvalue))
        return -self.math.log(prob * self.max_float) + self.log_max_float

    def score_inside(self, z):

        argvalue = (z - self.z_slope_center_upper) / self.softness
        prob1 = 1.0 - (1.0 - self.plateau) / (1.0 + self.math.exp(-argvalue))

        argvalue = (z - self.z_slope_center_lower) / self.softness
        prob2 = 1.0 - (1.0 - self.plateau) / (1.0 + self.math.exp(argvalue))
        return (-self.math.log(prob1 * self.max_float)
                - self.math.log(prob2 * self.max_float)
                + 2 * self.log_max_float)

    def unprotected_evaluate(self, da):

        z_center = self.IMP.isd.Nuisance(self.z_nuisance).get_nuisance()
        self.z_slope_center_lower = z_center - self.thickness / 2.0
        self.z_slope_center_upper = z_center + self.thickness / 2.0

        score_above = sum([self.score_above(IMP.core.XYZ(p).get_z())
                          for p in self.particle_list_above])
        score_below = sum([self.score_below(IMP.core.XYZ(p).get_z())
                          for p in self.particle_list_below])
        score_inside = sum([self.score_inside(IMP.core.XYZ(p).get_z())
                           for p in self.particle_list_inside])

        return score_above + score_below + score_inside

    def do_get_inputs(self):
        particle_list = self.particle_list_above + \
            self.particle_list_inside + self.particle_list_below

        return particle_list


class MembraneRestraint(IMP.test.TestCase):

    def test_inside(self):
        m = IMP.Model()

        atom = IMP.Particle(m)

        d = IMP.core.XYZ.setup_particle(atom)

        p = IMP.Particle(m)

        z_center = IMP.isd.Nuisance.setup_particle(p)
        z_center.set_nuisance(0.0)

        r = MembraneRestraintPrototype(m, z_center)
        r.add_particles_inside([atom])

        r2 = IMP.pmi.MembraneRestraint(
            m, z_center.get_particle_index(), 30.0, 3.0, 0.0000000001, 0.02)
        r2.add_particles_inside([atom.get_index()])
        for z_c in range(-500, 500, 100):
            z_center.set_nuisance(z_c)
            for z in range(-500, 500, 10):
                IMP.core.XYZ(atom).set_z(z)
                self.assertAlmostEqual(
                    r.unprotected_evaluate(None), r2.unprotected_evaluate(None),
                    delta=1e-4)

    def test_above(self):
        m = IMP.Model()

        atom = IMP.Particle(m)

        d = IMP.core.XYZ.setup_particle(atom)

        p = IMP.Particle(m)

        z_center = IMP.isd.Nuisance.setup_particle(p)
        z_center.set_nuisance(0.0)

        r = MembraneRestraintPrototype(m, z_center)
        r.add_particles_above([atom])

        r2 = IMP.pmi.MembraneRestraint(
            m, z_center.get_particle_index(), 30.0, 3.0, 0.0000000001, 0.02)
        r2.add_particles_above([atom.get_index()])
        for z_c in range(-500, 500, 100):
            z_center.set_nuisance(z_c)
            for z in range(-500, 500, 10):
                IMP.core.XYZ(atom).set_z(z)
                self.assertAlmostEqual(
                    r.unprotected_evaluate(None), r2.unprotected_evaluate(None),
                    delta=1e-4)

    def test_below(self):
        m = IMP.Model()

        atom = IMP.Particle(m)

        d = IMP.core.XYZ.setup_particle(atom)

        p = IMP.Particle(m)

        z_center = IMP.isd.Nuisance.setup_particle(p)
        z_center.set_nuisance(0.0)

        r = MembraneRestraintPrototype(m, z_center)
        r.add_particles_below([atom])

        r2 = IMP.pmi.MembraneRestraint(
            m, z_center.get_particle_index(), 30.0, 3.0, 0.0000000001, 0.02)
        r2.add_particles_below([atom.get_index()])

        for z_c in range(-500, 500, 100):
            z_center.set_nuisance(z_c)
            for z in range(-500, 500, 10):
                IMP.core.XYZ(atom).set_z(z)
                self.assertAlmostEqual(
                    r.unprotected_evaluate(None), r2.unprotected_evaluate(None),
                    delta=1e-4)


if __name__ == '__main__':
    IMP.test.main()
