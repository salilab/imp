import IMP.test
import IMP.em
import os

class Tests(IMP.test.TestCase):

    def test_em_maps(self):
        """Check volume reading and writing"""
        # Read in Xmipp format
        rw1 = IMP.em.SpiderMapReaderWriter(
                        self.get_input_file_name("media_mon_iter3.xmp"),
                        False,False,True)
        rw2 = IMP.em.MRCReaderWriter()

        m= IMP.em.read_map(self.get_input_file_name("media_mon_iter3.xmp"), rw1)
        # Compare against known voxel values to make sure we're reading the
        # file correctly
        self.assertAlmostEqual(m.get_value(m.xyz_ind2voxel(24,28,25)),
                               0.04647, delta=0.001)
        self.assertAlmostEqual(m.get_value(m.xyz_ind2voxel(23,29,25)),
                               0.03346, delta=0.001)
        IMP.em.write_map(m, "test.mrc",rw2)
        IMP.em.write_map(m, "test.xmp",rw1)
        m2= IMP.em.read_map("test.xmp",rw1)
        # Check that the two maps have the same values
        for k in xrange(0,m.get_header().get_nz()):
            for j in xrange(0,m.get_header().get_ny()):
                for i in xrange(0,m.get_header().get_nx()):
                    self.assertEqual(m.get_value(m.xyz_ind2voxel(i,j,k)),
                                    m2.get_value(m.xyz_ind2voxel(i,j,k)))
        # Cleanup
        os.unlink('test.mrc')
        os.unlink('test.xmp')

if __name__ == '__main__':
    IMP.test.main()
