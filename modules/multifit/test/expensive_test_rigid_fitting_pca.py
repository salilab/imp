import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
import IMP.restrainer


class ProteinRigidFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_density_map(self):
        mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(self.get_input_file_name("1f7dA00_8.mrc"), mrw)
        self.scene.get_header_writable().set_resolution(8.)
        self.scene.update_voxel_size(1.5)
    def load_protein(self,pdb_filename):
        self.mp= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.imp_model, IMP.atom.NonWaterPDBSelector())
        IMP.atom.add_radii(self.mp)
        self.mp_ref= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.imp_model, IMP.atom.NonWaterPDBSelector())
        IMP.atom.add_radii(self.mp_ref)

        self.radius_key = IMP.core.XYZR.get_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.ps = IMP.core.get_leaves(self.mp)
        self.rb=IMP.atom.setup_as_rigid_body(self.mp)
        self.refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.VERBOSE)#SILENT)
        self.imp_model = IMP.Model()
        self.load_density_map()
        self.load_protein("1f7dA00.pdb")


    def test_pca_based_rigid_fitting(self):
        """test pca based fitting"""
        #randomize protein placement
        rand_t = IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.get_random_vector_in(
              IMP.algebra.BoundingBox3D(
                IMP.algebra.Vector3D(-10.,-10.,-10.),
                IMP.algebra.Vector3D(10.,10.,10.))))

        xyz=IMP.core.XYZs(self.ps)
        IMP.core.transform(self.rb,rand_t)
        #IMP.atom.write_pdb(self.mp,"translated.pdb")
        xyz_ref=IMP.core.XYZs(IMP.core.get_leaves(self.mp_ref))
        #fit protein
        fs = IMP.multifit.pca_based_rigid_fitting(
               self.rb,self.refiner,self.scene,0.15)
        #check that the rmsd to the reference is low
        self.assertEqual(fs.get_number_of_solutions(), 24)
        print "number of solutions:",fs.get_number_of_solutions()
        best_rmsd=999.
        for i in range(fs.get_number_of_solutions()):
            fit_t = fs.get_transformation(i)
            fit_t_inv = fit_t.get_inverse()
            IMP.core.transform(self.rb,fit_t)
            rmsd=IMP.atom.get_rmsd(xyz_ref,xyz)
            #IMP.atom.write_pdb(self.mp,"fitted_"+str(i)+".pdb")
            print "====rmsd: ",rmsd, " score: ",fs.get_score(i)
            if best_rmsd>rmsd:
                best_rmsd=rmsd
            IMP.core.transform(self.rb,fit_t_inv)
        self.assertAlmostEqual(best_rmsd,0., delta=1.)


if __name__ == '__main__':
    IMP.test.main()
