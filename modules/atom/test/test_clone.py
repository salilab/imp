import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper

class DecoratorTests(IMP.test.TestCase):
    def test_bonded(self):
        """Check close and destroy Hierarchy """
        m = IMP.Model()
        mh= IMP.atom.read_pdb(self.get_input_file_name("input.pdb"),
                              m)
        nump= len(m.get_particles())
        mhc= IMP.atom.create_clone(mh)
        nnump= len(m.get_particles())
        IMP.atom.destroy(mhc)
        mhc=None
        self.assertEqual(nump, len(m.get_particles()))
        IMP.atom.destroy(mh)
        mh=None
        self.assertEqual(0, len(m.get_particles()))

if __name__ == '__main__':
    unittest.main()
