import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit


class Tests(IMP.test.TestCase):
    """Test FFT-based rigid fitting"""

    def load_density_map(self):
        mrw = IMP.em.MRCReaderWriter()
        scene = IMP.em.read_map(self.get_input_file_name("3points.mrc"), mrw)
        scene.get_header_writable().set_resolution(8.)
        scene.update_voxel_size(1.5)
        return scene

    def load_protein(self, model, pdb_filename):
        self.mp= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                                   model, IMP.atom.CAlphaPDBSelector())
        IMP.atom.add_radii(self.mp)
        self.mp_ref= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                                       model, IMP.atom.CAlphaPDBSelector())
        IMP.atom.add_radii(self.mp_ref)

        self.ps =IMP.core.get_leaves(self.mp)
        self.rb=IMP.atom.create_rigid_body(self.mp)

    def test_fft_based_rigid_fitting(self):
        """Test FFT-based rigid fitting"""
        model = IMP.Model()
        scene = self.load_density_map()
        # Use an artificial protein and map containing only 3 atoms, for speed
        self.load_protein(model, "3points.pdb")
        #randomize protein placement
        rand_t = IMP.algebra.Transformation3D(
            IMP.algebra.get_random_rotation_3d(),
            IMP.algebra.get_random_vector_in(
              IMP.algebra.BoundingBox3D(
                IMP.algebra.Vector3D(-10.,-10.,-10.),
                IMP.algebra.Vector3D(10.,10.,10.))))

        xyz=IMP.core.XYZs(self.ps)
        IMP.core.transform(self.rb,rand_t)
        xyz_ref=IMP.core.XYZs(IMP.core.get_leaves(self.mp_ref))
        #fit protein
        fs = IMP.multifit.fft_based_rigid_fitting(self.mp, scene, 0.0, 0.2)
        #check that the rmsd to the reference is low
        best_rmsd=999.
        for f in fs:
            fit_t = f.get_fit_transformation()
            fit_t_inv = fit_t.get_inverse()
            IMP.core.transform(self.rb,fit_t)

            rmsd=IMP.atom.get_rmsd(xyz_ref,xyz)
            #check that the centroid is in the map
            centroid=IMP.core.get_centroid(xyz)
            self.assertTrue(scene.is_part_of_volume(centroid))
            if best_rmsd>rmsd:
                best_rmsd=rmsd
            IMP.core.transform(self.rb,fit_t_inv)
        self.assertLess(best_rmsd, 3.5)

if __name__ == '__main__':
    IMP.test.main()
