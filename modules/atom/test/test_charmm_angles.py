import IMP
import IMP.test
import IMP.atom

def make_bond(p1, p2):
    return IMP.atom.create_bond(IMP.atom.Bonded(p1), IMP.atom.Bonded(p2),
                                IMP.atom.Bond.SINGLE)

class Tests(IMP.test.TestCase):

    def make_atoms(self, m):
        ps = []
        for i in range(4):
            p = IMP.Particle(m)
            IMP.core.XYZ.setup_particle(p, IMP.algebra.Vector3D(0,0,0))
            IMP.atom.Mass.setup_particle(p, 1.0)
            IMP.atom.Atom.setup_particle(p, IMP.atom.AT_CA)
            IMP.atom.Bonded.setup_particle(p)
            ps.append(p)
        return ps

    def test_create_angles_not_charmm(self):
        """Test create_angles() with atoms that aren't CHARMAtom"""
        m = IMP.Model()
        atoms = self.make_atoms(m)
        bonds = [make_bond(atoms[0], atoms[1]),
                 make_bond(atoms[1], atoms[2])]

        IMP.atom.CHARMMAtom.setup_particle(atoms[0], "C1")
        IMP.atom.CHARMMAtom.setup_particle(atoms[1], "C1")
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        angles = ff.create_angles(bonds)
        self.assertEqual(len(angles), 1)
        a = IMP.atom.Angle(angles[0])
        # Angle should have no force constant or ideal value
        self.assertAlmostEqual(a.get_ideal(), -1.0, delta=1e-6)
        self.assertAlmostEqual(a.get_stiffness(), 0.0, delta=1e-6)

    def test_create_dihedrals_not_charmm(self):
        """Test create_dihedrals() with atoms that aren't CHARMAtom"""
        m = IMP.Model()
        atoms = self.make_atoms(m)
        bonds = [make_bond(atoms[0], atoms[1]),
                 make_bond(atoms[1], atoms[2]),
                 make_bond(atoms[2], atoms[3])]

        IMP.atom.CHARMMAtom.setup_particle(atoms[0], "C1")
        IMP.atom.CHARMMAtom.setup_particle(atoms[1], "C1")
        ff = IMP.atom.CHARMMParameters(IMP.atom.get_data_path("top.lib"),
                                       IMP.atom.get_data_path("par.lib"))
        dihedrals = ff.create_dihedrals(bonds)
        self.assertEqual(len(dihedrals), 1)
        a = IMP.atom.Dihedral(dihedrals[0])
        # Angle should have no force constant or ideal value
        self.assertAlmostEqual(a.get_ideal(), -1.0, delta=1e-6)
        self.assertAlmostEqual(a.get_stiffness(), 0.0, delta=1e-6)
        self.assertEqual(a.get_multiplicity(), 0)

if __name__ == '__main__':
    IMP.test.main()
