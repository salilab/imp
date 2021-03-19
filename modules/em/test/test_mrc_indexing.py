from __future__ import print_function
import IMP
import IMP.test
import IMP.em


class Tests(IMP.test.TestCase):
    """Test that MRC files with non-standard indexing are transposed
       correctly"""

    def test_centroid_similarity(self):
        """The centroids of experimental and model-derived map should
           be similar"""
        in_mrc = self.get_input_file_name("emd-20362.mrc")
        in_pdb = self.get_input_file_name("emd-20362.pdb")

        mrw = IMP.em.MRCReaderWriter()
        self.dmap = IMP.em.read_map(in_mrc, mrw)
        header = self.dmap.get_header()

        # Ensure indexing is non-standard
        self.assertNotEqual((header.mapc, header.mapr, header.maps), (0, 1, 2))

        self.dmap.get_header_writable().resolution = 1.8

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
