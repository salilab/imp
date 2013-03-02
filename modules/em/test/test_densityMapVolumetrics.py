import IMP
import IMP.test
import IMP.atom
import IMP.em

class Tests(IMP.test.TestCase):

    def test_get_molecular_mass_at_threshold(self):
        dmap = IMP.em.read_map( self.get_input_file_name("1z5s.mrc"),
                                IMP.em.MRCReaderWriter())
        # 1z5s weights ~ 50 kDa
        weight_In_kD          = 50000
        # result/expactation discrepency in number of pixels
        epsilon               = 4

        apix                  = dmap.get_spacing()
        protein_pix_mass      = IMP.atom.get_mass_from_volume(apix*apix*apix)
        allowedMassDelta      =  epsilon * protein_pix_mass

        threshold =\
            IMP.em.get_threshold_for_approximate_mass(dmap,weight_In_kD)
        mass      = IMP.em.get_molecular_mass_at_threshold (dmap, threshold)

        self.assertTrue(abs(weight_In_kD - mass) < allowedMassDelta,
            """asked threshold t to attein mass M1,
               and mass M2 was computed at threshold t;
                though M1 and M2 are different : """+
                str(weight_In_kD)+" != "+str(mass)
            )



if __name__ == '__main__':
    IMP.test.main()
