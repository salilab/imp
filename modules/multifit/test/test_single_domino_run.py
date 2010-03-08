import IMP
import IMP.test
import sys
import IMP.em
import IMP.multifit
import IMP.helper
import unittest
import os
import time

class SingleDominoRunTests(IMP.test.TestCase):
    """Tests for a domino run on a single mapping"""
    def setup_filenames(self):
        self.protein_fns=[self.get_input_file_name("1z5s_A.pdb"),
                            self.get_input_file_name("1z5s_B.pdb"),
                            self.get_input_file_name("1z5s_C.pdb"),
                            self.get_input_file_name("1z5s_D.pdb")]
        self.density_fn=self.get_input_file_name("1z5s_20.mrc")
        self.jt_fn=self.get_input_file_name("1z5s.jt")
        self.anchor_points_fn=self.get_input_file_name("1z5s_anchor_points.pdb")
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.setup_filenames()
        self.mdl=IMP.Model()
        self.mhs=IMP.atom.Hierarchies()
        self.ca_sel=IMP.atom.CAlphaPDBSelector()
        #load the components
        for prot_fn in self.protein_fns:
            mh=IMP.atom.read_pdb(prot_fn,self.mdl,self.ca_sel)
            self.mhs.append(mh)
            IMP.atom.add_radii(mh)
            IMP.atom.setup_as_rigid_body(mh)
        #load the density map
        self.dmap=IMP.helper.load_em_density_map(self.density_fn,3.,20.)
        #load the anchor points
        self.ap_mh=IMP.atom.read_pdb(self.anchor_points_fn,self.mdl,self.ca_sel)
        self.anchor_ps = IMP.Particles(IMP.core.get_leaves(self.ap_mh))
        name_key=IMP.StringKey("name")
        for i in range(len(self.anchor_ps)):
            self.anchor_ps[i].add_attribute(name_key,str(i))
        #load junction tree
        self.jt=IMP.domino.JunctionTree()
        IMP.domino.read_junction_tree(self.jt_fn,self.jt)
    def test_run(self):
        """Check that the correct answer is returned"""
        anchor2comp = IMP.ParticlePairs()
        for i in range(4):
            anchor2comp.append(IMP.ParticlePair(self.anchor_ps[i],self.mhs[i].get_particle()))
        single_run=IMP.multifit.SingleDominoRun(anchor2comp,self.mhs,self.jt,self.dmap)

if __name__ == '__main__':
    unittest.main()
