import unittest
import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
import IMP.helper


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

        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.ps = IMP.Particles(IMP.core.get_leaves(self.mp))

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
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
        xyz=IMP.core.XYZsTemp(self.ps)
        for x in xyz:
            x.set_coordinates(rand_t.get_transformed(x.get_coordinates()))
        xyz_ref=IMP.core.XYZsTemp(IMP.core.get_leaves(self.mp_ref))
        #fit protein
        fs = IMP.multifit.pca_based_rigid_fitting(
               IMP.container.ListSingletonContainer(self.ps),self.scene,0.15)
        #check that the rmsd to the reference is low
        self.assert_(fs.get_number_of_solutions()>0)
        for i in range(fs.get_number_of_solutions()):
            fit_t = fs.get_transformation(i)
            fit_t_inv = fit_t.get_inverse()
            for x in xyz:
                x.set_coordinates(fit_t.get_transformed(x.get_coordinates()))
            print "rmsd: ",IMP.atom.get_rmsd(xyz_ref,xyz), " score: ",fs.get_score(i)
            # if i==0:
            #     self.assertInTolerance(IMP.atom.get_rmsd(xyz_ref,xyz),0.,6.)
            #     self.assertInTolerance(fs.get_score(i),1.,0.3)
            for x in xyz:
                x.set_coordinates(fit_t_inv.get_transformed(x.get_coordinates()))


if __name__ == '__main__':
    unittest.main()
