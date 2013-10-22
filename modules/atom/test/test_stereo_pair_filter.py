import IMP
import IMP.test
import IMP.core
import IMP.atom

def setup_filter():
    m = IMP.kernel.Model()
    ps = []
    for i in range(10):
        p = IMP.kernel.Particle(m)
        IMP.core.XYZ.setup_particle(p)
        ps.append(p)
    bd1 = IMP.atom.Bonded.setup_particle(ps[0])
    bd2 = IMP.atom.Bonded.setup_particle(ps[1])
    bonds = []
    bonds.append(IMP.atom.create_bond(bd1, bd2,
                               IMP.atom.Bond.SINGLE).get_particle())
    angles = []
    angles.append(IMP.atom.Angle.setup_particle(IMP.kernel.Particle(m),
                       IMP.core.XYZ(ps[1]), IMP.core.XYZ(ps[2]),
                       IMP.core.XYZ(ps[3])).get_particle())
    dihedrals = []
    dihedrals.append(IMP.atom.Dihedral.setup_particle(IMP.kernel.Particle(m),
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
        # Inputs should always be empty
        for p in ps:
            self.assertEqual(pf.get_input_particles(p), [])


if __name__ == '__main__':
    IMP.test.main()
