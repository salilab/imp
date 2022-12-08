import IMP
import IMP.test
import IMP.core
import IMP.atom


def setup_filter():
    m = IMP.Model()
    ps = []
    for i in range(10):
        p = m.add_particle("P%d" % i)
        IMP.core.XYZ.setup_particle(m, p)
        ps.append(p)
    bd1 = IMP.atom.Bonded.setup_particle(m, ps[0])
    bd2 = IMP.atom.Bonded.setup_particle(m, ps[1])
    bonds = []
    bonds.append(IMP.atom.create_bond(bd1, bd2,
                                      IMP.atom.Bond.SINGLE).get_particle())
    angles = []
    angles.append(IMP.atom.Angle.setup_particle(IMP.Particle(m),
                                                IMP.core.XYZ(m, ps[1]),
                                                IMP.core.XYZ(m, ps[2]),
                                                IMP.core.XYZ(m, ps[3])).get_particle())
    dihedrals = []
    dihedrals.append(IMP.atom.Dihedral.setup_particle(IMP.Particle(m),
                                                      IMP.core.XYZ(m, ps[4]),
                                                      IMP.core.XYZ(m, ps[5]),
                                                      IMP.core.XYZ(m, ps[6]),
                                                      IMP.core.XYZ(m, ps[7])).get_particle())

    pf = IMP.atom.StereochemistryPairFilter()
    return m, pf, ps, bonds, angles, dihedrals


class Tests(IMP.test.TestCase):

    """Test StereochemistryPairFilter"""

    def test_filter(self):
        """Check the StereochemistryPairFilter exclusions"""
        m, pf, ps, bonds, angles, dihedrals = setup_filter()

        ps = IMP.get_particles(m, ps)
        self.assertEqual(pf.get_value((ps[0], ps[1])), False)
        self.assertEqual(pf.get_value((ps[1], ps[3])), False)
        self.assertEqual(pf.get_value((ps[4], ps[7])), False)
        self.assertEqual(pf.get_value((ps[0], ps[9])), False)
        pf.set_bonds(bonds)
        self.assertEqual(pf.get_value((ps[0], ps[1])), True)
        pf.set_angles(angles)
        self.assertEqual(pf.get_value((ps[1], ps[3])), True)
        self.assertEqual(pf.get_value((ps[1], ps[2])), False)
        pf.set_dihedrals(dihedrals)
        self.assertEqual(pf.get_value((ps[4], ps[7])), True)

    def test_input_particles(self):
        """Check StereochemistryPairFilter input particles"""
        m, pf, ps, bonds, angles, dihedrals = setup_filter()
        pf.set_bonds(bonds)
        pf.set_angles(angles)
        pf.set_dihedrals(dihedrals)
        # Inputs should always be empty
        for p in ps:
            self.assertEqual(pf.get_inputs(m, [p]), [])


if __name__ == '__main__':
    IMP.test.main()
