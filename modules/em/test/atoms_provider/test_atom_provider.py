import unittest
from particles_provider import ParticlesProvider
import IMP.test

class AtomProviderTests(IMP.test.TestCase):
    def test_atom_read(self):
        '''test particles providers that reads atoms'''
        # Check number of atoms
        atoms = ParticlesProvider()
        fn_coords = self.get_input_file_name('1tdx_atoms')
        atoms.read(fn_coords)
        self.assertEqual(499,atoms.get_size())

if __name__=='__main__':
    unittest.main()
