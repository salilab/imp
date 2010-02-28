import unittest
import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper
import random,math

class RigidBodyCorrelationByGridRotation(IMP.test.TestCase):
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
                              self.imp_model, IMP.atom.CAlphaPDBSelector())#IMP.atom.NonWaterSelector())
        self.mp_ref= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                                   self.imp_model, IMP.atom.CAlphaPDBSelector())#IMP.
        IMP.atom.add_radii(self.mp)
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.particles = IMP.Particles(IMP.core.get_leaves(self.mp))
    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
        self.load_density_map()
        self.load_protein("1z5s_A.pdb")
    def test_compute_fitting_scores(self):
        #generate a set of random transformations
        ts=[]
        ts.append(IMP.algebra.get_identity_transformation_3d())
        for i in range(20):
            translation = IMP.algebra.get_random_vector_in(IMP.algebra.get_unit_bounding_box_3d())
            axis = IMP.algebra.get_random_vector_on(IMP.algebra.get_unit_sphere_3d())
            rand_angle = random.uniform(-15./180*math.pi,15./180*math.pi)
            r= IMP.algebra.get_rotation_in_radians_about_axis(axis, rand_angle);
            t=IMP.algebra.Transformation3D(r,translation)
            ts.append(t)
        fr=IMP.em.FittingSolutions()
        IMP.em.compute_fitting_scores(self.particles,
                                      self.scene,
                                      self.radius_key,self.weight_key,
                                      ts,fr)
        mp_xyz=IMP.core.XYZs(IMP.core.get_leaves(self.mp))
        mp_ref_xyz=IMP.core.XYZs(IMP.core.get_leaves(self.mp_ref))
        best_score=[0,fr.get_score(0),
                   IMP.atom.get_rmsd(mp_xyz,mp_ref_xyz,ts[0])]
        for i in range(1,fr.get_number_of_solutions()):
            rmsd=IMP.atom.get_rmsd(mp_xyz,mp_ref_xyz,ts[i])
            score = fr.get_score(i)
            #print "rmsd:",rmsd," score:",score
            if score<best_score[1]:
                best_score=[i,score,rmsd]
        print "best score:",best_score
        self.assert_(best_score[2]<5)

if __name__ == '__main__':
    unittest.main()
