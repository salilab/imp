import IMP
import IMP.test
import IMP.core
import IMP.atom

def setup_filter():
    m = IMP.Model()
    ps = []
    for i in range(10):
        p = IMP.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        ps.append(p)
    bd1 = IMP.atom.Bonded.setup_particle(ps[0])
    bd2 = IMP.atom.Bonded.setup_particle(ps[1])
    bonds = []
    bonds.append(IMP.atom.create_bond(bd1, bd2,
                               IMP.atom.Bond.SINGLE).get_particle())
    angles = []
    angles.append(IMP.atom.Angle.setup_particle(IMP.Particle(m),
                       IMP.core.XYZ(ps[1]), IMP.core.XYZ(ps[2]),
                       IMP.core.XYZ(ps[3])).get_particle())
    dihedrals = []
    dihedrals.append(IMP.atom.Dihedral.setup_particle(IMP.Particle(m),
                          IMP.core.XYZ(ps[4]), IMP.core.XYZ(ps[5]),
                          IMP.core.XYZ(ps[6]),
                          IMP.core.XYZ(ps[7])).get_particle())

    pf = IMP.atom.StereochemistryPairFilter()
    return m, pf, ps, bonds, angles, dihedrals


class Tests(IMP.test.TestCase):
    """Test StereochemistryPairFilter"""

    def test_filter(self):
        """Check the StereochemistryPairFilter exclusions"""
        m, pf, ps, bonds, angles, dihedrals = setup_filter()

        self.assertEquals(pf.get_value(
                              (ps[0], ps[1])), False)
        self.assertEquals(pf.get_value(
                              (ps[1], ps[3])), False)
        self.assertEquals(pf.get_value(
                              (ps[4], ps[7])), False)
        self.assertEquals(pf.get_value(
                              (ps[0], ps[9])), False)
        pf.set_bonds(bonds)
        self.assertEquals(pf.get_value(
                              (ps[0], ps[1])), True)
        pf.set_angles(angles)
        self.assertEquals(pf.get_value(
                              (ps[1], ps[3])), True)
        self.assertEquals(pf.get_value(
                              (ps[1], ps[2])), False)
        pf.set_dihedrals(dihedrals)
        self.assertEquals(pf.get_value(
                              (ps[4], ps[7])), True)

    def test_input_particles(self):
        """Check StereochemistryPairFilter input particles"""
        m, pf, ps, bonds, angles, dihedrals = setup_filter()
        pf.set_bonds(bonds)
        pf.set_angles(angles)
        pf.set_dihedrals(dihedrals)

        # If the atom is not bonded to anything, output == input
        self.assertInputParticlesEqual(pf, ps[8], [ps[8]])
        # If the atom is bonded, output should also contain the bond particle(s)
        self.assertInputParticlesEqual(pf, ps[0], [ps[0], bonds[0]])
        self.assertInputParticlesEqual(pf, ps[1], [ps[1], bonds[0], angles[0]])
        self.assertInputParticlesEqual(pf, ps[4], [ps[4], dihedrals[0]])
        # Particles within an angle/dihedral (but not the 1-3 or 1-4 pairs)
        # don't count as bonded
        self.assertInputParticlesEqual(pf, ps[2], [ps[2]])
        self.assertInputParticlesEqual(pf, ps[5], [ps[5]])

    def assertInputParticlesEqual(self, pf, inp, exp_outps):
        outps = pf.get_input_particles(inp)
        # Note that the order of outps is not predictable (since it is
        # generated from a map of Particles, which compare based on their
        # memory addresses, which are not predictable). So, sort both lists
        # to ensure they are similarly ordered.
        exp_outps.sort()
        outps.sort()
        self.assertEqual(outps, exp_outps)

if __name__ == '__main__':
    IMP.test.main()
