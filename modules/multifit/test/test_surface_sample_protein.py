import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import os

class Tests(IMP.test.TestCase):
    """Tests for sampled density maps"""

    def setUp(self):
        """initialize IMP environment create particles"""
        IMP.test.TestCase.setUp(self)
        #init IMP model ( the environment)
        self.mdl = IMP.Model()
        self.mh=IMP.atom.read_pdb(self.get_input_file_name("1z5s_A.pdb"),
                             self.mdl,IMP.atom.CAlphaPDBSelector())
        IMP.atom.add_radii(self.mh)
        self.particles = IMP.core.get_leaves(self.mh)

    def test_add_surface_attribute(self):
        """Check that reading a map back in preserves the stdevs"""
        IMP.base.set_log_level(IMP.base.VERBOSE)
        voxel_size=1.
        IMP.multifit.add_surface_index(self.mh,voxel_size)
        shell_key= IMP.FloatKey("surf_ind")
        for p in self.particles:
            #print p.get_particle().get_value(shell_key)
            self.assertGreater(p.get_particle().get_value(shell_key), 3.,
                               "map was not sampled correctly")


if __name__ == '__main__':
    IMP.test.main()
