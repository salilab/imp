import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom

def setup_filter():
    m = IMP.Model()
    ps = IMP.Particles()
    for i in range(10):
        p = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        ps.append(p)
    bd1 = IMP.atom.Bonded.setup_particle(ps[0])
    bd2 = IMP.atom.Bonded.setup_particle(ps[1])
    bonds = IMP.Particles()
    bonds.append(IMP.atom.bond(bd1, bd2,
                               IMP.atom.Bond.SINGLE).get_particle())
    angles = IMP.Particles()
    angles.append(IMP.atom.Angle.setup_particle(IMP.Particle(m),
                       IMP.core.XYZ(ps[1]), IMP.core.XYZ(ps[2]),
                       IMP.core.XYZ(ps[3])).get_particle())
    dihedrals = IMP.Particles()
    dihedrals.append(IMP.atom.Dihedral.setup_particle(IMP.Particle(m),
                          IMP.core.XYZ(ps[4]), IMP.core.XYZ(ps[5]),
                          IMP.core.XYZ(ps[6]),
                          IMP.core.XYZ(ps[7])).get_particle())

    pf = IMP.atom.StereochemistryPairFilter()
    return m, pf, ps, bonds, angles, dihedrals


class StereoPairFilterTests(IMP.test.TestCase):
    """Test StereochemistryPairFilter"""

    def test_filter(self):
        """Check the StereochemistryPairFilter exclusions"""
        m, pf, ps, bonds, angles, dihedrals = setup_filter()

        self.assertEquals(pf.get_contains_particle_pair(
                              IMP.ParticlePair(ps[0], ps[1])), False)
        self.assertEquals(pf.get_contains_particle_pair(
                              IMP.ParticlePair(ps[1], ps[3])), False)
        self.assertEquals(pf.get_contains_particle_pair(
                              IMP.ParticlePair(ps[4], ps[7])), False)
        self.assertEquals(pf.get_contains_particle_pair(
                              IMP.ParticlePair(ps[0], ps[9])), False)
        pf.set_bonds(bonds)
        self.assertEquals(pf.get_contains_particle_pair(
                              IMP.ParticlePair(ps[0], ps[1])), True)
        pf.set_angles(angles)
        self.assertEquals(pf.get_contains_particle_pair(
                              IMP.ParticlePair(ps[1], ps[3])), True)
        self.assertEquals(pf.get_contains_particle_pair(
                              IMP.ParticlePair(ps[1], ps[2])), False)
        pf.set_dihedrals(dihedrals)
        self.assertEquals(pf.get_contains_particle_pair(
                              IMP.ParticlePair(ps[4], ps[7])), True)

    def test_input_particles(self):
        """Check StereochemistryPairFilter input particles"""
        m, pf, ps, bonds, angles, dihedrals = setup_filter()
        pf.set_bonds(bonds)
        pf.set_angles(angles)
        pf.set_dihedrals(dihedrals)

        # If the pair is not bonded, output == input
        self.assertInputParticlesEqual(pf, [ps[0], ps[9]], [ps[0], ps[9]])
        # If the pair is bonded, output should also contain the bond particle
        self.assertInputParticlesEqual(pf, [ps[0], ps[1]],
                                       [ps[0], ps[1], bonds[0]])
        self.assertInputParticlesEqual(pf, [ps[1], ps[3]],
                                       [ps[1], ps[3], angles[0]])
        self.assertInputParticlesEqual(pf, [ps[4], ps[7]],
                                       [ps[4], ps[7], dihedrals[0]])
        # Particles within an angle/dihedral (but not the 1-3 or 1-4 pairs)
        # don't count as bonded
        self.assertInputParticlesEqual(pf, [ps[1], ps[2]], [ps[1], ps[2]])
        self.assertInputParticlesEqual(pf, [ps[4], ps[6]], [ps[4], ps[6]])

    def assertInputParticlesEqual(self, pf, inps, exp_outps):
        outps = pf.get_input_particles(IMP.ParticlePair(inps[0], inps[1]))
        self.assertEqual(len(outps), len(exp_outps))
        for a, b in zip(outps, exp_outps):
            self.assertEqual(a, b)

if __name__ == '__main__':
    unittest.main()
