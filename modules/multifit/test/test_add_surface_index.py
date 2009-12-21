import unittest
import sys,os
import IMP
import IMP.em
import IMP.multifit
import IMP.test
import IMP.core
import IMP.atom
import IMP.helper

class ProteinRigidFittingTest(IMP.test.TestCase):
    """Class to test EM correlation restraint"""

    def load_protein(self,pdb_filename):
        self.mp= IMP.atom.read_pdb(self.open_input_file(pdb_filename),
                              self.imp_model, IMP.atom.CAlphaSelector())#IMP.atom.NonWaterSelector())
        IMP.atom.add_radii(self.mp)
        self.radius_key = IMP.core.XYZR.get_default_radius_key()
        self.weight_key = IMP.atom.Mass.get_mass_key()

    def setUp(self):
        """Build test model and optimizer"""
        IMP.test.TestCase.setUp(self)
        IMP.set_log_level(IMP.SILENT)
        self.imp_model = IMP.Model()
        self.resolution=3.0
        self.apix=1.0
        self.surf_key=IMP.FloatKey("surf_ind")
        self.load_protein("1z5s_A.pdb")

    def test_add_surface_index(self):
        """Check that the surface indexes are added correctly"""
        IMP.multifit.add_surface_index(self.mp,self.resolution,self.apix)
        ps=IMP.core.get_leaves(self.mp)
        ps_3=IMP.Particles()
        for ph in ps:
            p=ph.get_particle()
            if p.get_value(self.surf_key)>3.:
                ps_3.append(p)
        self.assert_(len(ps_3)>130, "wrong surface index assignment to particles")
if __name__ == '__main__':
    unittest.main()
