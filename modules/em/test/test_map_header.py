import os
import IMP
import IMP.em
import IMP.test
import IMP.core

class Tests(IMP.test.TestCase):
    """Class to testing Map header data"""

    def setUp(self):
        IMP.base.set_log_level(IMP.base.SILENT)
        IMP.test.TestCase.setUp(self)
        #Read PDB
        self.mdl=IMP.Model()
        self.mh = IMP.atom.read_pdb(self.get_input_file_name("input1.pdb"),self.mdl,IMP.atom.CAlphaPDBSelector())
        self.ps = IMP.core.get_leaves(self.mh)
        IMP.atom.add_radii(self.mh)
        self.voxel_size=2.0
        print "before"
        self.dmap =IMP.em.SampledDensityMap(self.ps,10.0,self.voxel_size)
        print "after"
    def test_header_values(self):
        self.assertEqual(self.dmap.get_spacing(),self.voxel_size, "the voxel size is wrong")
    def test_write_read_mrc(self):
        """test mrc format read/write """
        out_filename =  "input1.mrc"
        mrw=IMP.em.MRCReaderWriter()
        IMP.em.write_map(self.dmap,out_filename,mrw)
        scene= IMP.em.read_map(out_filename,mrw)
        # Check header size
        self.assertEqual(scene.get_spacing(),self.voxel_size)
        os.unlink(out_filename)

    def test_write_read_em(self):
        """test em format read/write """
        out_filename =  "input1.em"
        mrw=IMP.em.EMReaderWriter()
        IMP.em.write_map(self.dmap,out_filename,mrw)
        scene= IMP.em.read_map(out_filename,mrw)
        # Check header size
        self.assertEqual(scene.get_spacing(),self.voxel_size)
        os.unlink(out_filename)

    def test_get_header_memory(self):
        """Check memory management of DensityHeader"""
        m= IMP.em.read_map(self.get_input_file_name('1z5s.mrc'))
        hr = m.get_header()
        hw = m.get_header_writable()
        del m
        # Since DensityHeaders are returned as pointers into the DensityMap
        # object m accessing hr or hw after the deletion of m will cause
        # invalid memory access. The Python wrappers should ensure that
        # hr and hw keep a reference to m to prevent this from occurring.
        for h in hr, hw:
            a, b = h.alpha, h.beta
            h.alpha, h.beta = 45., 90.


if __name__ == '__main__':
    IMP.test.main()
