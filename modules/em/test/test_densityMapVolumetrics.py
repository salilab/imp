import IMP
import IMP.test
import IMP.atom
import IMP.em


class Tests(IMP.test.TestCase):

    def test_get_molecular_mass_at_threshold(self):
        dmap = IMP.em.read_map(self.get_input_file_name("1z5s.mrc"),
                               IMP.em.MRCReaderWriter())
        # 1z5s weights ~ 50 kDa
        weight_In_kD = 50000
        # result/expactation discrepancy in number of pixels
        epsilon = 4

        apix = dmap.get_spacing()
        protein_pix_mass = IMP.atom.get_mass_from_volume(
            apix * apix * apix)
        allowedMassDelta = epsilon * protein_pix_mass

        threshold =\
            IMP.em.get_threshold_for_approximate_mass(dmap, weight_In_kD)
        mass = IMP.em.get_molecular_mass_at_threshold(dmap, threshold)

        self.assertTrue(
            abs(weight_In_kD - mass) < allowedMassDelta,
            """asked threshold t to attein mass M1,
               and mass M2 was computed at threshold t;
                though M1 and M2 are different : """ +
            str(weight_In_kD) + " != " + str(mass))

    def test_get_threshold_for_small_volume(self):
        """Check threshold for volumes smaller than one voxel"""
        dmap = IMP.em.create_density_map(10, 10, 10, 1.0)
        for i in range(dmap.get_number_of_voxels()):
            dmap.set_value(i, float(i))
        # Volume of zero or less than a voxel should give the highest density
        for vol in (0.0, 0.5):
            t = IMP.em.get_threshold_for_approximate_volume(dmap, vol)
            self.assertAlmostEqual(t, 999.0, delta=1e-6)
        t = IMP.em.get_threshold_for_approximate_volume(dmap, 5.0)
        self.assertAlmostEqual(t, 995.0, delta=1e-6)
        # Volume larger than the map should give the lowest density
        t = IMP.em.get_threshold_for_approximate_volume(dmap, 5000.0)
        self.assertAlmostEqual(t, 0.0, delta=1e-6)


if __name__ == '__main__':
    IMP.test.main()
