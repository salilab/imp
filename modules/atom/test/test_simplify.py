import unittest
import IMP
import IMP.test
import IMP.core
import IMP.atom

class SimplifyTests(IMP.test.TestCase):

    def test_simplify_by_residue(self):
        """Test protein simplification by residues"""
        def residual_cond(x):
            if x != 0:
                return 1
            else:
                return 0
        IMP.set_log_level(IMP.SILENT)#VERBOSE)
        m= IMP.Model()
        mh= IMP.atom.read_pdb(self.get_input_file_name('input.pdb'), m)
        chains= IMP.atom.get_by_type(mh, IMP.atom.CHAIN_TYPE)
        num_residues=len(IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE))
        IMP.atom.add_radii(mh)
        for res_segment in [5,10,20,30,num_residues]:
            mh_simp= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(chains[0].get_particle()), res_segment)
            self.assertEqual(num_residues/res_segment+1*residual_cond(num_residues%res_segment), len(IMP.core.get_leaves(mh_simp)))


    def test_simplify_by_segments(self):
        """Test protein simplification by segments"""
        IMP.set_log_level(IMP.SILENT)#VERBOSE)
        m= IMP.Model()
        mh= IMP.atom.read_pdb(self.get_input_file_name('input.pdb'), m)
        chains= IMP.atom.get_by_type(mh, IMP.atom.CHAIN_TYPE)
        IMP.atom.add_radii(mh)
        #define the segments
        segs = IMP.IntRanges()
        num_res= len(IMP.atom.get_by_type(mh,IMP.atom.RESIDUE_TYPE))
        start=0
        step=30
        while start < num_res:
            segs.append(IMP.IntRange(start,min(start+step,num_res-1)))
            start=start+step+1
            #print segs[-1]
        mh_simp= IMP.atom.create_simplified_along_backbone(IMP.atom.Chain(chains[0].get_particle()),segs)
        self.assertEqual(num_res/step,len(IMP.core.get_leaves(mh_simp)))


if __name__ == '__main__':
    unittest.main()
