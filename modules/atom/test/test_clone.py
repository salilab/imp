import IMP
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    def test_bonded(self):
        """Check close and destroy Hierarchy """
        m = IMP.Model()
        mh= IMP.atom.read_pdb(self.get_input_file_name("mini.pdb"), m)
        nump= len(m.get_particles())
        mhc= IMP.atom.create_clone(mh)
        nnump= len(m.get_particles())
        self.assertEqual(nump * 2, nnump)
        IMP.atom.destroy(mhc)
        mhc=None
        self.assertEqual(nump, len(m.get_particles()))
        IMP.atom.destroy(mh)
        mh=None
        self.assertEqual(0, len(m.get_particles()))

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
