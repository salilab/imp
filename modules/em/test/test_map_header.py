import unittest
import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class HeaderTest(IMP.test.TestCase):
    """Class to testing Map header data"""

    def setUp(self):
        IMP.test.TestCase.setUp(self)
        #Read PDB
        self.mdl=IMP.Model()
        self.mh = IMP.atom.read_pdb(self.get_input_file_name("input1.pdb"),self.mdl,IMP.atom.NonWaterSelector())
        self.ps = IMP.core.get_leaves(self.mh);
        IMP.atom.add_radii(self.mh);
        self.voxel_size=2.0
        self.dmap =IMP.em.SampledDensityMap(self.ps,10.0,self.voxel_size);
        self.dmap.show()
        print "============================="
    def test_header_values(self):
        self.assertEqual(self.dmap.get_spacing(),self.voxel_size, "the voxel size is wrong")
    def test_write_read_mrc(self):
        """test mrc format read/write """
        out_filename =  "input1.mrc"
        mrw=IMP.em.MRCReaderWriter()
        IMP.em.write_map(self.dmap,out_filename,mrw)
        scene= IMP.em.read_map(out_filename,mrw)
        # Check header size
        scene.show()
        print "============================="

        self.assertEqual(scene.get_spacing(),self.voxel_size)
       #os.unlink(out_filename)

    def test_write_read_em(self):
        """test mrc format read/write """
        out_filename =  "input1.em"
        mrw=IMP.em.EMReaderWriter()
        IMP.em.write_map(self.dmap,out_filename,mrw)
        scene= IMP.em.read_map(out_filename,mrw)
        # Check header size
        scene.show()
        print "============================="
        self.assertEqual(scene.get_spacing(),self.voxel_size)
        os.unlink(out_filename)


if __name__ == '__main__':
    unittest.main()
