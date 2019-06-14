import IMP
import IMP.atom
import IMP.pmi
import IMP.test
import IMP.isd
import IMP.pmi.restraints.proteomics
import IMP.pmi.io
import IMP.pmi.restraints
import IMP.pmi.restraints.basic
import IMP.rmf
import RMF
import math
import sys

class MembraneRestraintPrototype(IMP.Restraint):

    def __init__(self,
                 m,
                 z_nuisance,
                 thickness=30.0,
                 softness=3.0,
                 plateau=0.0000000001,
                 linear=0.02):
        '''
        input a list of particles, the slope and theta of the sigmoid potential
        theta is the cutoff distance for a protein-protein contact
        '''
        IMP.Restraint.__init__(self, m, "MembraneRestraintPrototype_ %1%")
        self.set_was_used(True)
        self.thickness = thickness
        self.z_nuisance = z_nuisance
        self.softness = softness
        self.plateau = plateau
        self.particle_list_below = []
        self.particle_list_above = []
        self.particle_list_inside = []
        self.max_float = sys.float_info.max
        self.log_max_float = math.log(self.max_float)
        self.linear = linear

    def add_particles_below(self, particles):
        self.particle_list_below += particles

    def add_particles_above(self, particles):
        self.particle_list_above += particles

    def add_particles_inside(self, particles):
        self.particle_list_inside += particles

    def score_above(self, z):
        argvalue = (z - self.z_slope_center_upper) / self.softness
        prob = (1.0 - self.plateau) / (1.0 + math.exp(-argvalue))
        return -math.log(prob * self.max_float) + self.log_max_float

    def score_below(self, z):
        argvalue = (z - self.z_slope_center_lower) / self.softness
        prob = (1.0 - self.plateau) / (1.0 + math.exp(argvalue))
        return -math.log(prob * self.max_float) + self.log_max_float

    def score_inside(self, z):

        argvalue = (z - self.z_slope_center_upper) / self.softness
        prob1 = 1.0 - (1.0 - self.plateau) / (1.0 + math.exp(-argvalue))

        argvalue = (z - self.z_slope_center_lower) / self.softness
        prob2 = 1.0 - (1.0 - self.plateau) / (1.0 + math.exp(argvalue))
        return (-math.log(prob1 * self.max_float)
                - math.log(prob2 * self.max_float)
                + 2 * self.log_max_float)

    def unprotected_evaluate(self, da):

        z_center = IMP.isd.Nuisance(self.z_nuisance).get_nuisance()
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
        r2.set_was_used(True)
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
        r2.set_was_used(True)
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
        r2.set_was_used(True)
        r2.add_particles_below([atom.get_index()])

        for z_c in range(-500, 500, 100):
            z_center.set_nuisance(z_c)
            for z in range(-500, 500, 10):
                IMP.core.XYZ(atom).set_z(z)
                self.assertAlmostEqual(
                    r.unprotected_evaluate(None), r2.unprotected_evaluate(None),
                    delta=1e-4)

    def test_pmi_selection(self):
        m = IMP.Model()
        s = IMP.pmi.topology.System(m)
        st = s.create_state()
        len_helix = 40
        mol = st.create_molecule("helix",sequence='A'*len_helix, chain_id='A')
        mol.add_representation(mol,
                           resolutions=[1],
                           ideal_helix=True)
        hier = s.build()

        mr = IMP.pmi.restraints.basic.MembraneRestraint(hier,
                                                     objects_inside=[(11,30,'helix')],
                                                     objects_above=[(1,10,'helix')],
                                                     objects_below = [(31,40,'helix')])

        p_inside = mr.get_particles_inside()
        self.assertEqual(len(p_inside), 20)

        p_above = mr.get_particles_above()
        self.assertEqual(len(p_above), 10)

        p_below = mr.get_particles_below()
        self.assertEqual(len(p_below), 10)

if __name__ == '__main__':
    IMP.test.main()
