import IMP
import IMP.test
import IMP.core
import IMP.atom


class Tests(IMP.test.TestCase):

    def test_bonded(self):
        """Check close and destroy Hierarchy """
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        nump = len(m.get_particle_indexes())
        mhc = IMP.atom.create_clone(mh)
        nnump = len(m.get_particle_indexes())
        self.assertEqual(nump * 2, nnump)
        IMP.atom.destroy(mhc)
        mhc = None
        self.assertEqual(nump, len(m.get_particle_indexes()))
        IMP.atom.destroy(mh)
        mh = None
        self.assertEqual(0, len(m.get_particle_indexes()))

    def test_clone_mass(self):
        """Test clone of particles that have mass but are not atoms"""
        m = IMP.Model()
        p1 = IMP.Particle(m)
        h1 = IMP.atom.Hierarchy.setup_particle(p1)
        m1 = IMP.atom.Mass.setup_particle(p1, 4.0)

        h2 = IMP.atom.create_clone(h1)
        self.assertTrue(IMP.atom.Mass.get_is_setup(h2))
        m2 = IMP.atom.Mass(h2)
        self.assertAlmostEqual(m2.get_mass(), 4.0, delta=1e-4)

    def test_destroy_child(self):
        """Destroy of a child should update the parent"""
        m = IMP.Model()
        mh = IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        atoms = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 68)
        IMP.atom.destroy(atoms[0])
        # This will fail if the atom is not removed from the parent residue
        atoms = IMP.atom.get_by_type(mh, IMP.atom.ATOM_TYPE)
        self.assertEqual(len(atoms), 67)

if __name__ == '__main__':
    IMP.test.main()
