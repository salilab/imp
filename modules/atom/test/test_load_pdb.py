import unittest
import IMP.atom
import IMP.test
from IMP.algebra import *

class Tests(IMP.test.TestCase):
    def test_navigation(self):
        """Test loading of rigid bodies from pdbs"""
        m= IMP.Model()
        h= IMP.atom.read_pdb(self.get_input_file_name("dna.pdb"),
                             m)
        rbp= IMP.atom.create_rigid_body(h)
        rf= rbp.get_reference_frame().get_transformation_to()
        rbp.set_reference_frame(IMP.algebra.ReferenceFrame3D())
        IMP.atom.read_pdb(self.get_input_file_name("dna.pdb"), 0,
                          h)
        nrf=rbp.get_reference_frame().get_transformation_to()
        print rf, nrf
        self.assertAlmostEqual((rf.get_translation()
                                -nrf.get_translation()).get_magnitude(),
                               0, delta=.1)
        self.assertAlmostEqual((rf.get_rotation().get_quaternion()
                                -nrf.get_rotation().get_quaternion()).get_magnitude(),
                               0, delta=.1)

if __name__ == '__main__':
    unittest.main()
