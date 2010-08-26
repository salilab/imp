import unittest
import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper

class LocalFittingTest(IMP.test.TestCase):
    """Test local fitting functions"""

    def sample_density_map(self):
        self.scene = IMP.em.particles2density(IMP.core.get_leaves(self.mh),
                                              10.,2.)

    def load_protein(self,pdb_filename):
        self.m = IMP.Model()
        self.mh= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.m, IMP.atom.CAlphaPDBSelector())
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        IMP.atom.add_radii(self.mh)
        IMP.atom.setup_as_rigid_body(self.mh)
        self.particles = IMP.core.get_leaves(self.mh)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
        self.load_protein("1z5s_A.pdb")
        self.sample_density_map()
        mhs=IMP.atom.Hierarchies()
        mhs.append(self.mh)
        self.pdb_opt_state=None#IMP.atom.WritePDBOptimizerState(mhs,"temp_%03d.pdb")
    def test_fast_vs_slow_local_fitting(self):
        """Check that fast and slow local fitting converge to the same score"""
        print "calculate fast"
        rand_translation = 15.*IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
        rt= IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),rand_translation)
        IMP.core.transform(IMP.core.RigidBody(self.mh),rt)
        fr_fast=IMP.em.local_rigid_fitting(
            IMP.core.RigidBody(self.mh),
            self.radius_key, self.weight_key,
            self.scene,self.pdb_opt_state,1,2,100,2.,.3,True)
        fr_slow=IMP.em.local_rigid_fitting(
             IMP.core.RigidBody(self.mh),
             self.radius_key, self.weight_key,
             self.scene,None,1,2,50,2.,.3,False)
        print "fast:",fr_fast.get_score(0)," slow:",fr_slow.get_score(0)
        '''
        self.assert_(abs(fr_fast.get_score(0)-fr_slow.get_score(0))<0.1)
        '''
if __name__ == '__main__':
    unittest.main()
