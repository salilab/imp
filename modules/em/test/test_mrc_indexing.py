from __future__ import print_function
import IMP
import IMP.test
import IMP.em
import os


class Tests(IMP.test.TestCase):
    """Test that MRC files with non-standard indexing are transposed
       correctly"""

    def test_invalid_axes(self):
        """Check handling of MRCs with invalid axes"""
        # axes should be a permutation of (1,2,3). This test file has
        # (10,11,12) instead, which should raise an error.
        self.assertRaises(
            IOError, IMP.em.read_map,
            self.get_input_file_name('invalid_axes.mrc'))

    def test_centroid_similarity(self):
        """The centroids of experimental and model-derived map should
           be similar"""
        in_mrc = self.get_input_file_name("emd-20362.mrc")
        in_pdb = self.get_input_file_name("emd-20362.pdb")

        mrw = IMP.em.MRCReaderWriter()
        self.dmap = IMP.em.read_map(in_mrc, mrw)
        header = self.dmap.get_header()

        # Ensure indexing is non-standard
        self.assertNotEqual((header.mapc, header.mapr, header.maps), (1, 2, 3))

        self.dmap.get_header_writable().resolution = 1.8

        # Make sure that our density map does not get mangled
        # (e.g. transposed from non-standard to standard indexing again) if we
        # read it in a second time
        IMP.em.write_map(self.dmap, 'standard_indexing.mrc')
        newdmap = IMP.em.read_map('standard_indexing.mrc')
        self.assertLess(
            IMP.algebra.get_distance(self.dmap.get_centroid(),
                                     newdmap.get_centroid()), 5.0)
        newheader = newdmap.get_header()
        self.assertEqual((newheader.mapc, newheader.mapr, newheader.maps),
                         (1, 2, 3))
        os.unlink('standard_indexing.mrc')

        mdl = IMP.Model()
        self.rh = IMP.atom.read_pdb(in_pdb, mdl)
        self.ps = IMP.atom.get_leaves(self.rh)
        self.smap = IMP.em.SampledDensityMap(
            self.ps, 1.8, self.dmap.get_header().get_spacing())

        self.assertLess(
            IMP.algebra.get_distance(self.dmap.get_centroid(),
                                     self.smap.get_centroid()), 5.0)


if __name__ == '__main__':
    IMP.test.main()
