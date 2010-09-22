import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper
import IMP.display

class ProteinRigidFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self,em_filename):
        mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(self.get_input_file_name(em_filename), mrw)
        self.scene.get_header_writable().set_resolution(10.)
        self.scene.update_voxel_size(2.0)

    def load_proteins(self,pdb_filenames):
        self.mhs=IMP.atom.Hierarchies()
        self.ps = IMP.Particles()
        self.rbs=IMP.core.RigidBodies()
        self.leaves_ref = IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
        for pf in pdb_filenames:
            self.mhs.append(IMP.atom.read_pdb(self.open_input_file(pf),
                                       self.imp_model, IMP.atom.CAlphaPDBSelector()))
        for mh in self.mhs:
            IMP.atom.add_radii(mh)
            rb=IMP.atom.setup_as_rigid_body(mh)
            self.ps = self.ps + self.leaves_ref.get_refined(rb.get_particle())
            self.rbs.append(rb)
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        IMP.set_check_level(IMP.NONE)
        self.imp_model = IMP.Model()
        self.load_density_map("1z5s_10.mrc")
        self.load_proteins(["1z5s_A_fitted.pdb","1z5s_B_fitted.pdb",
                            "1z5s_C_fitted.pdb","1z5s_D_fitted.pdb",])
        self.pdb_opt_state=IMP.atom.WritePDBOptimizerState(self.mhs,"temp_%03d.pdb")

    def test_cc_derivaties_using_rigid_body_fast_mode(self):
        """Test that multiple rigid bodies are correctly pulled into the density"""
        fit_r = IMP.em.FitRestraint(IMP.Particles(self.rbs),
                                    self.scene,
                                    self.leaves_ref)
        self.imp_model.add_restraint(fit_r)
        #randomize the position of the protein

        for rb in self.rbs:
            #rand_translation=IMP.algebra.Vector3D(-5.,5.,0.)
            rand_translation = 5.*IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
            rt= IMP.algebra.Transformation3D(IMP.algebra.get_identity_rotation_3d(),rand_translation)
            IMP.core.transform(rb,rt)
        IMP.atom.write_pdb(self.mhs,"aa0.pdb")
        #see that the derivatives pull it back in place
        opt = IMP.core.ConjugateGradients()
        opt.set_model(self.imp_model)
        #opt.add_optimizer_state(self.pdb_opt_state)
        start_score=self.imp_model.evaluate(False)
        print "before optimization"
        end_score=opt.optimize(10)
        print "after optimization"
        #IMP.atom.write_pdb(self.mhs,"aa.pdb")
        self.assertGreater(start_score, end_score)

if __name__ == '__main__':
    IMP.test.main()
