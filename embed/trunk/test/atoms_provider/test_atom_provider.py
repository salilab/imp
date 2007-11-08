
import unittest
from particles_provider import particles_provider
import EM 

class test(unittest.TestCase):
        def test_atom_read(self):
            '''test particles providers that reads atoms'''
            # Check number of atoms
            atoms=particles_provider()
            fn_coords='data/1tdx_atoms'
            atoms.read(fn_coords)
            self.assertEqual(499,atoms.get_size())
if __name__=='__main__':
	unittest.main()
