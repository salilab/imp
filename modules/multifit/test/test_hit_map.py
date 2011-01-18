import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
import IMP.restrainer


class HitMapTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_data(self):
        self.mdl=IMP.Model()
        self.radius_key = IMP.core.XYZR.get_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.mh=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),self.mdl)
        self.dmap=IMP.em.read_map(self.get_input_file_name("1z5s_20.mrc"),IMP.em.MRCReaderWriter())
        self.dmap.get_header().set_resolution(20)
        self.rb=IMP.atom.setup_as_rigid_body(self.mh)
        self.ref=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.load_data()

    def test_hit_map(self):
        """test hit map creation"""
        self.assertLess(0,1)
        #create all fits
        rots=[IMP.algebra.get_identity_rotation_3d()]
        fits=IMP.multifit.fft_based_rigid_fitting(self.rb,self.ref,self.dmap,0.5,rots,100,False,True)
        print fits.get_solutions().get_number_of_solutions(),":",len(IMP.multifit.convert_em_to_multifit_format(fits.get_solutions()))
        #create hit map
        hits = IMP.multifit.create_hit_map(self.rb,self.ref,
                                           IMP.multifit.convert_em_to_multifit_format(fits.get_solutions()),
                                           self.dmap)
        hits.calcRMS()
        max_val=hits.get_max_value()
        print "max_val",max_val
        self.assertLess(0,max_val)
        IMP.em.write_map(hits,"hits.mrc",IMP.em.MRCReaderWriter())
if __name__ == '__main__':
    IMP.test.main()
