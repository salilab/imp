import IMP.pmi
from IMP.pmi.io import Subsequence,SubsequenceData
import IMP.pmi.io as io
import IMP.test
import os,sys

class DataStorageTests(IMP.test.TestCase):
    def setUp(self):
        IMP.test.TestCase.setUp(self)
        self.mdl = IMP.Model()

    def test_subsequence(self):
        """Test setup of subsequence"""
        mh = IMP.atom.read_pdb(self.get_input_file_name('chainA.pdb'),self.mdl)
        sd = Subsequence(chain='A',residue_tuple=(100,125))
        sd.add_range(chain='A',residue_tuple=(325,355))
        sel = sd.get_selection(hier=mh,atom_type=IMP.atom.AtomType('CA'))
        idxs = sorted([IMP.atom.get_residue(IMP.atom.Atom(h)).get_index()
                for h in sel.get_selected_particles()])
        self.assertEqual(idxs,list(range(100,126))+list(range(325,356)))
        self.assertEqual(sd.__repr__(),'100-125.A_325-355.A')

    def test_subsequence_data(self):
        """Test the SubsequenceData collection"""
        ssd = SubsequenceData()
        ssd.add_subsequence('helix',Subsequence(molecule='m1',
                                                residue_tuple=(55,58)))
        ssd.add_subsequence('helix',Subsequence(molecule='m2',
                                                residue_tuple=(195,208)))
        beta = Subsequence(molecule='m1',residue_tuple=(5,10))
        beta.add_range(molecule='m2',residue_tuple=(95,100))
        ssd.add_subsequence('beta',beta)

        self.assertEqual(len(ssd['helix']),2)
        self.assertEqual(len(ssd['beta']),1)
        self.assertEqual(ssd['helix'][0].__repr__(),
                         '55-58.m1')
        self.assertEqual(ssd['helix'][1].__repr__(),
                         '195-208.m2')
        self.assertEqual(ssd['beta'][0].__repr__(),
                         '5-10.m1_95-100.m2')

    def test_dssp_parsing(self):
        """Test reading DSSP files"""
        sses = io.parse_dssp(self.get_input_file_name('chainA.dssp'),'A')
        self.assertEqual(sorted(sses.keys()),sorted(['helix','beta','loop']))
        self.assertEqual(len(sses['helix']),20)
        self.assertEqual(len(sses['beta']),3)
        self.assertEqual(len(sses['loop']),32)
    '''
    def test_crosslink_data(self):
        """Test the CrossLinkData storage class"""
        global_sdict = SelectionDict(self.mdl,
                                     atom_type=IMP.atom.AtomType("CA"))
        xld = CrossLinkData(self.mdl,global_sdict)
        xld.add_cross_link(0,
                           SelectionDict(self.mdl,
                                         residue_index=5,
                                         molecule='himom'),
                           SelectionDict(self.mdl,
                                         residue_index=10,
                                         molecule='himom'),
                           score=5.0)
        xld.add_cross_link(0,
                           SelectionDict(self.mdl,
                                         residue_index=25,
                                         molecule='himom'),
                           SelectionDict(self.mdl,
                                         residue_index=35,
                                         molecule='himom'),
                           score=5.0)
        xld.add_cross_link(1,
                           SelectionDict(self.mdl,
                                         residue_index=225,
                                         molecule='hidad'),
                           SelectionDict(self.mdl,
                                         residue_index=335,
                                         molecule='hidad'),
                           score=5.0)

    def test_xl_parsing(self):
        """Test the XL davis parser"""
        data = io.parse_xlinks_davis(self.mdl,
                                     self.get_input_file_name('xls_davis.txt'),
                                     name_map={'His-TEV-Tub4':'ytub'},
                                     named_offsets={'ytub':-33})
        self.assertEqual(len(data),41)
        self.assertEqual(cmp(data[0],
                             [{'r1':SelectionDict(self.mdl,
                                                 residue_index=337,
                                                 molecule='ytub'),
                              'r2':SelectionDict(self.mdl,
                                                 residue_index=831,
                                                 molecule='Spc98'),
                              'score':10.88819}]),0)
    '''
if __name__ == '__main__':
    IMP.test.main()
