import sys,os
import IMP
import IMP.em
import IMP.test
import IMP.core
import IMP.atom

class Tests(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def sample_density_map(self):
        self.scene = IMP.em.particles2density(IMP.core.get_leaves(self.mh),
                                              10.,2.)

    def load_protein(self,pdb_filename):
        self.m = IMP.Model()
        self.mh= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.m, IMP.atom.CAlphaPDBSelector())
        self.radius_key = IMP.core.XYZR.get_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()
        IMP.atom.add_radii(self.mh)
        IMP.atom.create_rigid_body(self.mh)
        self.particles = IMP.core.get_leaves(self.mh)

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.base.set_log_level(IMP.base.SILENT)
        IMP.base.set_check_level(IMP.base.NONE)
        self.mdl = IMP.Model()
        self.load_protein("1z5s_A.pdb")
        self.sample_density_map()

    def test_correct_spacing(self):
        """Check that the correct spacing gets the highest CC"""
        spacings=[1.7,1.8,1.9,2.0,2.1,2.2,2.3]
        scores=[]
        for ii,spacing in enumerate(spacings):
            self.scene.update_voxel_size(spacing)
            scores.append(IMP.em.compute_fitting_score(IMP.core.get_leaves(self.mh),self.scene))
        for i,score in enumerate(scores):
            print "score for spacing:",spacings[i]," is:",score
        for ind in [0,1,2,4,5,6]:
            self.assertGreater(scores[i], scores[0], "wrong spacing:"+str(spacings[ind])+" has better value than spacing=1.7")
        for i in range(6):
            self.assertAlmostEqual(scores[i], scores[i+1], delta=0.3,
                                   msg="scores should be similar")

if __name__ == '__main__':
    IMP.test.main()
