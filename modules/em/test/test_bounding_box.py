import unittest
import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class BoundingBoxTest(IMP.test.TestCase):
    """Class to testing bounding box size"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #Read PDB
        self.mdl=IMP.Model()
        self.mrw = IMP.em.MRCReaderWriter()
        self.dmap = IMP.em.read_map(self.get_input_file_name("1z5s_5.imp.mrc"),self.mrw)

    #not clear if this test should work
    def _test_header_values(self):
        bb = IMP.em.get_bounding_box(self.dmap)
        header= self.dmap.get_header()
        spacing = self.dmap.get_spacing()
        c1 = bb.get_corner(0)
        c2 = bb.get_corner(1)
        bb_x = c2[0] - c1[0]
        bb_y = c2[1] - c1[1]
        bb_z = c2[2] - c1[2]
        print int(bb_x/spacing),":",header.get_nx()
        self.assertEqual(int(bb_x/spacing),header.get_nx(), "X dimension is wrong")
        self.assertEqual(int(bb_y/spacing),header.get_ny(), "Y dimension is wrong")
        self.assertEqual(int(bb_z/spacing),header.get_nz(), "Z dimension is wrong")

if __name__ == '__main__':
    unittest.main()
