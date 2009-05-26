import unittest
import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.utils
import IMP.core
import IMP.atom
import IMP.helper

class ProteinRigidFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self):
        self.scene = IMP.em.DensityMap()
        mrw = IMP.em.MRCReaderWriter()
        self.scene.Read(self.get_input_file_name("1z5s_10.mrc"), mrw)
        self.scene.get_header_writable().set_resolution(10.)
        self.scene.get_header_writable().set_spacing(2.0)
        self.scene.get_header_writable().set_xorigin(34.0)
        self.scene.get_header_writable().set_yorigin(8.0)
        self.scene.get_header_writable().set_zorigin(-92.0)
    def load_protein(self,pdb_filename):
        self.m = IMP.Model()
        self.mp= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.m, IMP.atom.CAlphaSelector())#IMP.atom.NonWaterSelector())
        self.mps = IMP.atom.Hierarchys()
        self.mps.append(self.mp)
        self.radius_key = IMP.FloatKey("radius")
        self.weight_key = IMP.FloatKey("weight")
        #add radius and weight attributes
        self.particles = IMP.Particles()
        for p in IMP.core.get_leaves(self.mp):
            self.particles.append(p)
        for p in self.particles:
            p.add_attribute(self.radius_key, 1.5)
            p.add_attribute(self.weight_key, 1.0)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
        self.load_density_map()
        self.load_protein("1z5s_A.pdb")

    def test_em_fit(self):
        """Check that correlation of particles with their own density is 1"""
        #create a rigid body
        rb_p = IMP.Particle(self.imp_model)
        rb_state = IMP.helper.create_rigid_body(rb_p,self.particles)
        rb_d = IMP.core.RigidBody.cast(rb_p);
        ref_trans = rb_d.get_transformation()

        fr = IMP.em.FittingSolutions()
        IMP.em.local_rigid_fitting_around_point(
            rb_d,rb_state,self.radius_key, self.weight_key,
            self.scene,IMP.algebra.Vector3D(87.0856,71.7701,-56.3955),
            fr,None,
            3,5,50)
        for i in xrange(fr.get_number_of_solutions()):
            print "transformation " +str(i)
            tt = fr.get_transformation(i)/ref_trans
            #tt.show()
            #print "|" + str(fr.get_score(i))

        #test that if you apply the transformation on the original configuration you get the same result
        # (in rmsd and score)

        #second, test that the optimization gets you close.
        score = self.imp_model.evaluate(False)
        self.assert_(fr.get_number_of_solutions() == 3, "not enough solutions returned")
        self.assert_(fr.get_score(0) < fr.get_score(1), "solutions are not sorted")
        for i in xrange(3):
            print fr.get_score(i)
            self.assert_(fr.get_score(i) < 1.0, "wrong CC values")

if __name__ == '__main__':
    unittest.main()
