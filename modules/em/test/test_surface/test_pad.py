import unittest
import IMP.test
import IMP.em

class PadTests(IMP.test.TestCase):

    def setUp(self):
        # Initial values and names of files
        IMP.test.TestCase.setUp(self)
        self.mdl=IMP.Model()
        sel=IMP.atom.NonAlternativePDBSelector()
        mh1 = IMP.atom.read_pdb(self.get_input_file_name('1z5s_A.pdb'),self.mdl,sel)
        IMP.atom.add_radii(mh1)
        mh2 = IMP.atom.read_pdb(self.get_input_file_name('1tdx.pdb'),self.mdl,sel)
        IMP.atom.add_radii(mh2)
        self.prot1_atoms = IMP.core.get_leaves(mh1)
        self.prot2_atoms = IMP.core.get_leaves(mh2)
        self.d1 = IMP.em.SampledDensityMap(self.prot1_atoms,3.0,1.0)
        self.d2 = IMP.em.SampledDensityMap(self.prot2_atoms,3.0,1.0)
    def test_map_padding(self):
        """Test map padding"""
        h1=self.d1.get_header()
        h2=self.d2.get_header()
        min1=self.d1.get_min_value()
        min2=self.d2.get_min_value()
        max1=self.d1.get_max_value()
        max2=self.d2.get_max_value()
        #fix map dimensions

        nx=max(h1.nx,h2.nx)
        ny=max(h1.ny,h2.ny)
        nz=max(h1.nz,h2.nz)

        self.d1.pad(nx,ny,nz)
        self.d2.pad(nx,ny,nz)

        self.assert_(min1 == self.d1.get_min_value(), "wrong map values after padding")
        self.assert_(min2 == self.d2.get_min_value(), "wrong map values after padding")
        self.assert_(max1 == self.d1.get_max_value(), "wrong map values after padding")
        self.assert_(max2 == self.d2.get_max_value(), "wrong map values after padding")


if __name__=='__main__':
    unittest.main()
