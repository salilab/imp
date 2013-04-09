import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    """Test local fitting functions"""

    def sample_density_map(self):
        self.scene = IMP.em.particles2density(IMP.core.get_leaves(self.mh),
                                              10.,2.)

    def load_protein(self,pdb_filename):
        self.m = IMP.Model()
        self.mh= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.m, IMP.atom.CAlphaPDBSelector())
        self.weight_key = IMP.atom.Mass.get_mass_key()
        IMP.atom.add_radii(self.mh)
        IMP.atom.create_rigid_body(self.mh)
        self.rb=IMP.core.RigidMember(IMP.core.get_leaves(self.mh)[0]).get_rigid_body()
        self.particles = IMP.core.get_leaves(self.mh)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.TERSE)
        #IMP.base.set_check_level(IMP.base.NONE)
        self.imp_model = IMP.Model()
        self.load_protein("1z5s_A.pdb")
        self.sample_density_map()
        mhs=IMP.atom.Hierarchies()
        mhs.append(self.mh)
        self.fast_pdb_opt_state=IMP.atom.WritePDBOptimizerState(mhs,"fast_temp_%03d.pdb")
        self.slow_pdb_opt_state=IMP.atom.WritePDBOptimizerState(mhs,"slow_temp_%03d.pdb")

    def test_fast_vs_slow_local_fitting(self):
        """Check that fast and slow local fitting converge to the same score"""
        rand_translation = 15.*IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        rt= IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),rand_translation)
        IMP.core.transform(self.rb,rt)
        refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
        print "calculate fast"
        num_mc=1
        num_cg=10
        fr_fast=IMP.em.local_rigid_fitting(
            self.mh,
            refiner,
            self.weight_key,
            self.scene,[self.fast_pdb_opt_state],1,num_mc,num_cg,2.,.3,True)
        print "calculate slow"
        fr_slow=IMP.em.local_rigid_fitting(
             self.mh,
             refiner,
             self.weight_key,
             self.scene,[self.slow_pdb_opt_state],1,num_mc,num_cg,2.,.3,False)
        print "fast:",fr_fast.get_score(0)," slow:",fr_slow.get_score(0)
        self.assertAlmostEqual(fr_fast.get_score(0), fr_slow.get_score(0),
                               delta=0.1)

if __name__ == '__main__':
    IMP.test.main()
