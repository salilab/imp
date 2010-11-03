import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom
import IMP.multifit
import IMP.restrainer


class FFTFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""


    def load_data(self,pdb_filename):
        self.m= IMP.Model()
        self.h=IMP.atom.read_pdb(self.get_input_file_name(pdb_filename),self.m)
        IMP.atom.add_radii(self.h)
        self.h_ref=IMP.atom.read_pdb(self.get_input_file_name(pdb_filename),self.m)
        self.mhs=[]
        self.mhs_ref=[]
        self.rbs=[]
        self.ps=[]
        for i,c in enumerate(IMP.atom.get_by_type(self.h, IMP.atom.CHAIN_TYPE)):
            self.mhs.append(c)
            self.ps+=IMP.core.get_leaves(self.mhs[-1])
            self.rbs.append(IMP.atom.setup_as_rigid_body(c))

        for c in IMP.atom.get_by_type(self.h_ref, IMP.atom.CHAIN_TYPE):
            self.mhs_ref.append(c)
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        self.refiner=IMP.core.LeavesRefiner(IMP.atom.Hierarchy.get_traits())
        self.scene = IMP.em.particles2density(self.ps,6,1.5)
        self.scene.get_header_writable().set_resolution(6.)
        self.scene.update_voxel_size(1.5)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
    def test_fft_based_rigid_fitting_translation_for_protein1(self):
        """test FFT based fitting for a components of 1z5s"""
        print "1z5s"
        self.fft_based_rigid_fitting_translation_for_protein("1z5s.pdb")
    def test_fft_based_rigid_fitting_translation_for_protein2(self):
        """test FFT based fitting for a components of 3hfm"""
        print "3hfm"
        self.fft_based_rigid_fitting_translation_for_protein("3hfm.pdb")

    def fft_based_rigid_fitting_translation_for_protein(self,filename):
        self.load_data(filename)
        for i,mh in enumerate(self.mhs[3:]):
            i=3
            print i
            #randomize protein placement
            rand_t = IMP.algebra.Transformation3D(
                IMP.algebra.get_identity_rotation_3d(),
                IMP.algebra.get_random_vector_in(
                IMP.algebra.BoundingBox3D(
                IMP.algebra.Vector3D(-10.,-10.,-10.),
                IMP.algebra.Vector3D(10.,10.,10.))))
            rb=IMP.core.RigidBody(mh)
            xyz=IMP.core.XYZsTemp(IMP.core.get_leaves(mh))
            IMP.core.transform(rb,rand_t)
            xyz_ref=IMP.core.XYZsTemp(IMP.core.get_leaves(self.mhs_ref[i]))
            #fit protein
            rots=IMP.algebra.Rotation3Ds()
            rots.append(IMP.algebra.get_identity_rotation_3d())
            fs = IMP.multifit.fft_based_rigid_fitting(
                rb,self.refiner,self.scene,0,rots,20,True)
            #check that the score makes sense
            sols=fs.get_solutions()
            best_rmsd=9999
            best_score=0
            best_trans=""
            for j in range(sols.get_number_of_solutions()):
                IMP.core.transform(rb,sols.get_transformation(j));
                rmsd=IMP.atom.get_rmsd(xyz_ref,xyz)
                IMP.core.transform(rb,sols.get_transformation(j).get_inverse());
                if best_rmsd>rmsd:
                    best_rmsd=rmsd
                    best_score=sols.get_score(j)
                    best_trans=sols.get_transformation(j)
            cc_map=fs.get_max_cc_map()
            IMP.em.write_map(cc_map,"comp"+str(i)+"_cc_max.mrc",IMP.em.MRCReaderWriter())
            print "PDB best RMSD for",str(i),"is:",best_rmsd
            print "SCORE:",best_score
            self.assertLess(best_rmsd,8.)
            print "end"
            #self.assertAlmostEqual(best_score,1.,delta=1.)


if __name__ == '__main__':
    IMP.test.main()
