import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import random,math

class Tests(IMP.test.TestCase):
    '''
    Check if the CC by rigid rotation makes sense
    '''
    def load_density_map(self):
        mrw = IMP.em.MRCReaderWriter()
        self.scene = IMP.em.read_map(self.get_input_file_name("1z5s_10.mrc"), mrw)
        self.resolution=10.
        self.voxel_size=2.
        self.scene.get_header_writable().set_resolution(self.resolution)
        self.scene.update_voxel_size(self.voxel_size)
        self.scene.set_origin(34.0,8.0,-92.0)
    def load_protein(self,pdb_filename):
        self.mp= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.imp_model, IMP.atom.CAlphaPDBSelector())
        self.mp_ref= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                                   self.imp_model, IMP.atom.CAlphaPDBSelector())#IMP.
        IMP.atom.add_radii(self.mp)
        self.radius_key = IMP.core.XYZR.get_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.particles = IMP.core.get_leaves(self.mp)
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        self.imp_model = IMP.Model()
        self.load_density_map()
        self.load_protein("1z5s_A.pdb")
    def test_compute_fitting_scores(self):
        #move the particles to be way outside of the density initially
        #IMP.base.set_log_level(IMP.base.VERBOSE)
        mp_xyz=IMP.core.XYZs(IMP.core.get_leaves(self.mp))
        displacement = IMP.algebra.Vector3D(100,100,100)
        displacement_t = IMP.algebra.Transformation3D(
            IMP.algebra.get_identity_rotation_3d(),
            IMP.algebra.Vector3D(100,100,100))
        for xyz in mp_xyz:
            xyz.set_coordinates(
                displacement_t.get_transformed(xyz.get_coordinates()))
        #generate a set of random transformations
        ts=[]
        ts.append(displacement_t.get_inverse())
        for i in range(20):
            translation = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
            axis = IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
            rand_angle = random.uniform(-15./180*math.pi,15./180*math.pi)
            r= IMP.algebra.get_rotation_about_axis(axis, rand_angle)
            t=IMP.algebra.Transformation3D(r,translation)
            ts.append(t*displacement_t.get_inverse())
        scores_fast = IMP.em.compute_fitting_scores(
            self.particles,
            self.scene,
            ts,True)
        scores_slow = IMP.em.compute_fitting_scores(
            self.particles,
            self.scene,
            ts,False)
        #check that the functions calculated 20 scores
        self.assertEqual(scores_fast.get_number_of_solutions(), len(ts))
        self.assertEqual(scores_slow.get_number_of_solutions(), len(ts))
        #check that the scores are similar
        for i in range(scores_fast.get_number_of_solutions()):
            print i, " fast:",scores_fast.get_score(i)
            print i, " slow:",scores_slow.get_score(i)
            self.assertAlmostEqual(scores_fast.get_score(i),
                                   scores_slow.get_score(i), delta=0.1)
        #check that scores make sense, we use the slow scores are
        #they are more accurate
        mp_ref_xyz=IMP.core.XYZs(IMP.core.get_leaves(self.mp_ref))
        for xyz in mp_xyz:
            xyz.set_coordinates(ts[0].get_transformed(xyz.get_coordinates()))
        best_score=[0,scores_slow.get_score(0),
                    IMP.atom.get_rmsd(mp_ref_xyz,mp_xyz)]#,ts[0])]
        for xyz in mp_xyz:
            xyz.set_coordinates(ts[0].get_inverse().get_transformed(xyz.get_coordinates()))
        for i in range(1,scores_slow.get_number_of_solutions()):
            for xyz in mp_xyz:
                xyz.set_coordinates(ts[i].get_transformed(xyz.get_coordinates()))
            rmsd=IMP.atom.get_rmsd(mp_xyz,mp_ref_xyz)#,ts[i])
            for xyz in mp_xyz:
                xyz.set_coordinates(ts[i].get_inverse().get_transformed(xyz.get_coordinates()))
            score = scores_slow.get_score(i)
            print "rmsd:",rmsd," score:",score
            if score<best_score[1]:
                best_score=[i,score,rmsd]
        print "best score:",best_score
        self.assertLess(best_score[2],5)

if __name__ == '__main__':
    IMP.test.main()
