import IMP
import IMP.test
import IMP.em

references=[
            IMP.em.HARPAZ,
            IMP.em.ANDERSSON,
            IMP.em.TSAI,
            IMP.em.QUILLIN,
            IMP.em.SQUIRE
            ]

class DensityMapVolumetricsTest(IMP.test.TestCase):
#
#    def test_get_reference_protein_density(self):
#        for ref in references:
#            densityValue = IMP.em.get_reference_protein_density(ref)

    def test_get_used_protein_density(self):
        self.assertTrue(
                IMP.em.get_used_protein_density()
                ==
                IMP.em.get_reference_protein_density(IMP.em.HARPAZ),
                "default used protein density value should be HARPAZ")

    def test_set_and_get_used_protein_density(self):
        # crash on negative densities
        self.assertRaises( IMP.UsageException,
                           lambda d:IMP.em.set_used_protein_density(d),-1.0 )

        # setting density directly
        densitySet =  IMP.em.set_used_protein_density(1.0)
        self.assertTrue(densitySet == IMP.em.get_used_protein_density()==1.0,
                "after setting it to 1.0, protein density value should be 1.0")
        # setting density through reference
        for ref in references :
            density =  IMP.em.set_used_protein_density(ref)
            self.assertTrue(
                density == IMP.em.get_used_protein_density(),
                "protein density value mismatch between what I just set and what I then get (through reference)"
                )
            self.assertTrue(
                density == IMP.em.get_reference_protein_density(ref),
                "protein density value mismatch between what I just set and what I then get (reference value mismatch)"
                )

    def test_get_molecular_mass_at_threshold(self):
        dmap = IMP.em.read_map(self.get_input_file_name("1z5s.mrc"),IMP.em.MRCReaderWriter())
        weight_In_kD              = 50000 # 50 kDa
        epsilon                   = 2     # in number of pixels

        apix                      = dmap.get_spacing()
        allowedMassDelta          = apix*apix*apix * epsilon * IMP.em.get_used_protein_density()

        threshold = IMP.em.get_threshold_for_approximate_mass(dmap,weight_In_kD)
        mass      = IMP.em.get_molecular_mass_at_threshold (dmap, threshold)
        self.assertTrue(abs(weight_In_kD - mass) < allowedMassDelta,
            "asked threshold t to attein mass M1, mass M2 computed at threshold t; though M1 and M2 are different"
            )



# Float compute_threshold_for_approximate_volume(em::DensityMap* m, Float desiredVolume);



if __name__ == '__main__':
    IMP.test.main()
