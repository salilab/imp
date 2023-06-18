import utils
import os
import unittest
try:
    import urllib.request as urllib2
except ImportError:
    import urllib2

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm
import ihm.location
import ihm.representation
import ihm.model
import ihm.source
import ihm.flr


class Tests(unittest.TestCase):
    def test_system(self):
        """Test System class"""
        s = ihm.System(title='test system')
        self.assertEqual(s.title, 'test system')
        self.assertEqual(s.id, 'model')

    def test_chem_comp(self):
        """Test ChemComp class"""
        cc1 = ihm.ChemComp(id='GLY', code='G', code_canonical='G')
        self.assertEqual(cc1.id, 'GLY')
        self.assertEqual(cc1.code, 'G')
        self.assertEqual(cc1.code_canonical, 'G')
        self.assertEqual(cc1.type, 'other')
        self.assertEqual(str(cc1), '<ihm.ChemComp(GLY)>')
        self.assertIsNone(cc1.formula)
        self.assertIsNone(cc1.formula_weight)
        cc2 = ihm.ChemComp(id='GLY', code='G', code_canonical='G')
        cc3 = ihm.ChemComp(id='G', code='G', code_canonical='G')
        self.assertEqual(cc1, cc2)
        self.assertEqual(hash(cc1), hash(cc2))
        self.assertNotEqual(cc1, cc3)

    def test_chem_comp_id_5(self):
        """Test new-style 5-character CCD IDs in ChemComp"""
        cc1 = ihm.ChemComp(id='MYGLY', code='G', code_canonical='G')
        self.assertEqual(cc1.id, 'MYGLY')
        self.assertEqual(cc1.code, 'G')
        self.assertEqual(cc1.code_canonical, 'G')
        self.assertEqual(cc1.type, 'other')

    def test_chem_comp_weight(self):
        """Test ChemComp.formula_weight"""
        # No formula
        cc = ihm.ChemComp('X', 'X', 'X', formula=None)
        self.assertIsNone(cc.formula_weight)
        # Bad formula
        cc = ihm.ChemComp('X', 'X', 'X', formula='C90H')
        self.assertRaises(ValueError, lambda x: x.formula_weight, cc)
        # Formula with unknown element
        cc = ihm.ChemComp('X', 'X', 'X', formula='C5 Es')
        self.assertIsNone(cc.formula_weight)
        # Formula with known elements and no charge
        cc = ihm.ChemComp('X', 'X', 'X', formula='C6 H12 P')
        self.assertAlmostEqual(cc.formula_weight, 115.136, delta=0.01)
        # Formula with element 'X' (e.g. GLX, ASX)
        cc = ihm.ChemComp('X', 'X', 'X', formula='C6 H12 P X2')
        self.assertAlmostEqual(cc.formula_weight, 115.136, delta=0.01)
        # Formula with known elements and formal charge
        cc = ihm.ChemComp('X', 'X', 'X', formula='C6 H12 P 1')
        self.assertAlmostEqual(cc.formula_weight, 115.136, delta=0.01)

    def test_peptide_chem_comp(self):
        """Test PeptideChemComp class"""
        cc1 = ihm.PeptideChemComp(id='GLY', code='G', code_canonical='G')
        self.assertEqual(cc1.type, 'peptide linking')

    def test_l_peptide_chem_comp(self):
        """Test LPeptideChemComp class"""
        cc1 = ihm.LPeptideChemComp(id='MET', code='M', code_canonical='M')
        self.assertEqual(cc1.type, 'L-peptide linking')

    def test_rna_chem_comp(self):
        """Test RNAChemComp class"""
        cc1 = ihm.RNAChemComp(id='G', code='G', code_canonical='G')
        self.assertEqual(cc1.type, 'RNA linking')

    def test_dna_chem_comp(self):
        """Test DNAChemComp class"""
        cc1 = ihm.DNAChemComp(id='DG', code='DG', code_canonical='G')
        self.assertEqual(cc1.type, 'DNA linking')

    def test_non_polymer_chem_comp(self):
        """Test NonPolymerChemComp class"""
        cc1 = ihm.NonPolymerChemComp('HEM')
        self.assertEqual(cc1.type, 'non-polymer')
        self.assertEqual(cc1.code_canonical, 'X')
        cc2 = ihm.NonPolymerChemComp('HEM', code_canonical='G')
        self.assertEqual(cc2.code_canonical, 'G')

    def test_water_chem_comp(self):
        """Test WaterChemComp class"""
        cc1 = ihm.WaterChemComp()
        self.assertEqual(cc1.type, 'non-polymer')

    def test_l_peptide_alphabet(self):
        """Test LPeptideAlphabet class"""
        a = ihm.LPeptideAlphabet
        self.assertEqual(a._comps['G'].type, 'peptide linking')

        self.assertEqual(a._comps['M'].id, 'MET')
        self.assertEqual(a._comps['M'].code, 'M')
        self.assertEqual(a._comps['M'].code_canonical, 'M')
        self.assertEqual(a._comps['M'].type, 'L-peptide linking')
        self.assertEqual(a._comps['M'].name, "METHIONINE")
        self.assertEqual(a._comps['M'].formula, 'C5 H11 N O2 S')
        self.assertAlmostEqual(a._comps['M'].formula_weight, 149.211,
                               delta=0.01)

        self.assertEqual(a._comps['Z'].id, 'GLX')
        self.assertEqual(a._comps['Z'].name, "GLU/GLN AMBIGUOUS")
        self.assertEqual(a._comps['Z'].formula, 'C5 H8 N O2 X2')
        self.assertAlmostEqual(a._comps['Z'].formula_weight, 114.124,
                               delta=0.01)

        a = ihm.LPeptideAlphabet()
        self.assertIn('MSE', a)
        self.assertNotIn('DG', a)
        self.assertEqual(len(a.keys), 25)
        self.assertEqual(len(a.values), 25)
        self.assertEqual(sorted(a.keys)[0], 'A')
        self.assertEqual(len(a.items), 25)
        item0 = sorted(a.items)[0]
        self.assertEqual(item0[0], 'A')
        self.assertEqual(item0[1].id, 'ALA')
        self.assertEqual(a['MSE'].id, 'MSE')
        self.assertEqual(a['MSE'].code, 'MSE')
        self.assertEqual(a['MSE'].code_canonical, 'M')
        self.assertEqual(a['MSE'].type, 'L-peptide linking')
        self.assertEqual(a['UNK'].id, 'UNK')
        self.assertEqual(a['UNK'].code, 'UNK')
        self.assertEqual(a['UNK'].code_canonical, 'X')
        self.assertEqual(a['UNK'].type, 'L-peptide linking')

    def test_d_peptide_alphabet(self):
        """Test DPeptideAlphabet class"""
        dcode_from_canon = {'A': 'DAL', 'C': 'DCY', 'D': 'DAS', 'E': 'DGL',
                            'F': 'DPN', 'H': 'DHI', 'I': 'DIL', 'K': 'DLY',
                            'L': 'DLE', 'M': 'MED', 'N': 'DSG', 'P': 'DPR',
                            'Q': 'DGN', 'R': 'DAR', 'S': 'DSN', 'T': 'DTH',
                            'V': 'DVA', 'W': 'DTR', 'Y': 'DTY', 'G': 'G'}
        da = ihm.DPeptideAlphabet
        la = ihm.LPeptideAlphabet
        # Weights and formulae of all standard amino acids should be identical
        # between L- and D- forms (except for lysine, where the formal charge
        # differs between the two forms)
        for canon in 'ACDEFGHILMNPQRSTVWY':
            lcode = canon
            dcode = dcode_from_canon[canon]
            self.assertEqual(da._comps[dcode].formula,
                             la._comps[lcode].formula)
            self.assertAlmostEqual(da._comps[dcode].formula_weight,
                                   la._comps[lcode].formula_weight, delta=0.01)

    def test_rna_alphabet(self):
        """Test RNAAlphabet class"""
        a = ihm.RNAAlphabet
        self.assertEqual(a._comps['A'].id, 'A')
        self.assertEqual(a._comps['A'].code, 'A')
        self.assertEqual(a._comps['A'].code_canonical, 'A')

    def test_dna_alphabet(self):
        """Test DNAAlphabet class"""
        a = ihm.DNAAlphabet
        self.assertEqual(a._comps['DA'].id, 'DA')
        self.assertEqual(a._comps['DA'].code, 'DA')
        self.assertEqual(a._comps['DA'].code_canonical, 'A')

    def test_chem_descriptor(self):
        """Test ChemDescriptor class"""
        d1 = ihm.ChemDescriptor(
            'EDC', chemical_name='test-EDC',
            chem_comp_id='test-chem-comp',
            common_name='test-common-EDC', smiles='CCN=C=NCCCN(C)C')
        self.assertEqual(d1.auth_name, 'EDC')
        self.assertEqual(d1.chem_comp_id, 'test-chem-comp')
        self.assertEqual(d1.chemical_name, 'test-EDC')
        self.assertEqual(d1.common_name, 'test-common-EDC')
        self.assertEqual(d1.smiles, 'CCN=C=NCCCN(C)C')
        self.assertIsNone(d1.inchi)
        self.assertIsNone(d1.inchi_key)

    def test_entity(self):
        """Test Entity class"""
        e1 = ihm.Entity('AHCD', description='foo')
        # Should compare identical if sequences are the same
        e2 = ihm.Entity('AHCD', description='bar')
        e3 = ihm.Entity('AHCDE', description='foo')
        heme = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        self.assertEqual(e1, e2)
        self.assertNotEqual(e1, e3)
        self.assertEqual(e1.seq_id_range, (1, 4))
        self.assertEqual(e3.seq_id_range, (1, 5))
        # seq_id does not exist for nonpolymers
        self.assertEqual(heme.seq_id_range, (None, None))

    def test_entity_weight(self):
        """Test Entity.formula_weight"""
        e1 = ihm.Entity('AHCD')
        self.assertAlmostEqual(e1.formula_weight, 499.516, delta=0.1)
        # Entity containing a component with unknown weight
        heme = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        self.assertIsNone(heme.formula_weight)

    def test_entity_type(self):
        """Test Entity.type"""
        protein = ihm.Entity('AHCD')
        heme = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        water = ihm.Entity([ihm.WaterChemComp()])
        self.assertEqual(protein.type, 'polymer')
        self.assertTrue(protein.is_polymeric())
        self.assertEqual(heme.type, 'non-polymer')
        self.assertFalse(heme.is_polymeric())
        self.assertEqual(water.type, 'water')
        self.assertFalse(water.is_polymeric())

        # A single amino acid should be classified non-polymer
        single_aa = ihm.Entity('A')
        self.assertEqual(single_aa.type, 'non-polymer')
        self.assertFalse(single_aa.is_polymeric())

        # ... unless forced polymer
        single_aa._force_polymer = True
        self.assertEqual(single_aa.type, 'polymer')
        self.assertTrue(single_aa.is_polymeric())

        # An entity with no sequence is a polymer
        empty = ihm.Entity([])
        self.assertEqual(empty.type, 'polymer')
        self.assertTrue(empty.is_polymeric())

    def test_entity_src_method_default(self):
        """Test default values of Entity.src_method"""
        protein = ihm.Entity('AHCD')
        water = ihm.Entity([ihm.WaterChemComp()])
        self.assertEqual(protein.src_method, "man")
        self.assertEqual(water.src_method, "nat")

        # src_method is readonly
        def try_set():
            protein.src_method = 'foo'

        self.assertRaises(TypeError, try_set)

    def test_entity_source(self):
        """Test setting Entity source"""
        man = ihm.Entity('AHCD', source=ihm.source.Manipulated())
        self.assertEqual(man.src_method, "man")

        nat = ihm.Entity('AHCD', source=ihm.source.Natural())
        self.assertEqual(nat.src_method, "nat")

        syn = ihm.Entity('AHCD', source=ihm.source.Synthetic())
        self.assertEqual(syn.src_method, "syn")

    def test_software(self):
        """Test Software class"""
        s1 = ihm.Software(name='foo', version='1.0',
                          classification='1', description='2', location='3')
        s2 = ihm.Software(name='foo', version='2.0',
                          classification='4', description='5', location='6')
        s3 = ihm.Software(name='foo', version='1.0',
                          classification='7', description='8', location='9')
        s4 = ihm.Software(name='bar', version='1.0',
                          classification='a', description='b', location='c')
        s5 = ihm.Software(name='bar', version=ihm.unknown,
                          classification='a', description='b', location='c',
                          citation='foo')
        # Should compare equal iff name and version both match
        self.assertEqual(s1, s3)
        self.assertEqual(hash(s1), hash(s3))
        self.assertNotEqual(s1, s2)
        self.assertNotEqual(s1, s4)
        # Unknown values should not compare equal to known
        self.assertNotEqual(s4, s5)
        self.assertNotEqual(hash(s4), hash(s5))

    def test_citation(self):
        """Test Citation class"""
        s = ihm.Citation(title='Test paper', journal='J Mol Biol',
                         volume=45, page_range=(1, 20), year=2016,
                         authors=['Smith A', 'Jones B'],
                         doi='10.2345/S1384107697000225',
                         pmid='1234')
        self.assertEqual(s.title, 'Test paper')

    def _get_from_pubmed_id(self, json_fname):
        def mock_urlopen(url):
            self.assertTrue(url.endswith('&id=29539637'))
            fname = utils.get_input_file_name(TOPDIR, json_fname)
            return open(fname)
        # Need to mock out urllib2 so we don't hit the network (expensive)
        # every time we test
        try:
            orig_urlopen = urllib2.urlopen
            urllib2.urlopen = mock_urlopen
            return ihm.Citation.from_pubmed_id(29539637)
        finally:
            urllib2.urlopen = orig_urlopen

    def test_citation_from_pubmed_id(self):
        """Test Citation.from_pubmed_id()"""
        c = self._get_from_pubmed_id('pubmed_api.json')
        self.assertEqual(
            c.title,
            'Integrative structure and functional anatomy of a nuclear '
            'pore complex (test of python-ihm lib).')
        self.assertEqual(c.journal, 'Nature')
        self.assertEqual(c.volume, '555')
        self.assertEqual(c.page_range, ['475', '482'])
        self.assertEqual(c.year, '2018')
        self.assertEqual(c.pmid, 29539637)
        self.assertEqual(c.doi, '10.1038/nature26003')
        self.assertEqual(len(c.authors), 32)
        self.assertEqual(c.authors[0], 'Kim, S.J.')

    def test_citation_from_pubmed_id_one_page(self):
        """Test Citation.from_pubmed_id() with page rather than range"""
        c = self._get_from_pubmed_id('pubmed_api_one_page.json')
        self.assertEqual(c.page_range, '475')

    def test_citation_from_pubmed_id_no_volume_page(self):
        """Test Citation.from_pubmed_id() with no volume or page info"""
        c = self._get_from_pubmed_id('pubmed_api_no_pages.json')
        self.assertIsNone(c.page_range)
        self.assertIsNone(c.volume)

    def test_citation_from_pubmed_id_no_doi(self):
        """Test Citation.from_pubmed_id() with no DOI"""
        c = self._get_from_pubmed_id('pubmed_api_no_doi.json')
        self.assertEqual(
            c.title,
            'Integrative structure and functional anatomy of a nuclear '
            'pore complex (test of python-ihm lib).')
        self.assertIsNone(c.doi)
        # Make sure that page range "475-82" is handled as 475,482
        self.assertEqual(c.page_range, ['475', '482'])

    def test_entity_residue(self):
        """Test Residue derived from an Entity"""
        e = ihm.Entity('AHCDAH')
        r = e.residue(3)
        self.assertEqual(r.entity, e)
        self.assertIsNone(r.asym)
        self.assertEqual(r.seq_id, 3)
        self.assertEqual(r.comp.id, 'CYS')

    def test_water_asym(self):
        """Test WaterAsymUnit class"""
        e = ihm.Entity('AHCDAH')
        water = ihm.Entity([ihm.WaterChemComp()])
        a = ihm.AsymUnit(e)
        self.assertEqual(a.seq_id_range, (1, 6))
        self.assertEqual(len(a.sequence), 6)
        self.assertEqual(a.number_of_molecules, 1)

        a = ihm.WaterAsymUnit(water, number=3)
        self.assertEqual(a.seq_id_range, (1, 3))
        self.assertEqual(len(a.sequence), 3)
        self.assertEqual(a.number_of_molecules, 3)

        self.assertRaises(TypeError, ihm.AsymUnit, water)
        self.assertRaises(TypeError, ihm.WaterAsymUnit, e)

    def test_asym_unit_residue(self):
        """Test Residue derived from an AsymUnit"""
        e = ihm.Entity('AHCDAH')
        a = ihm.AsymUnit(e, auth_seq_id_map=5)
        r = a.residue(3)
        self.assertIsNone(r.entity)
        self.assertEqual(r.asym, a)
        self.assertEqual(r.seq_id, 3)
        self.assertEqual(r.auth_seq_id, 8)
        self.assertIsNone(r.ins_code)
        self.assertEqual(r.comp.id, 'CYS')

    def test_atom_entity(self):
        """Test Atom class built from an Entity"""
        e = ihm.Entity('AHCDAH')
        a = e.residue(3).atom('CA')
        self.assertEqual(a.id, 'CA')
        self.assertEqual(a.residue.entity, e)
        self.assertEqual(a.residue.seq_id, 3)
        self.assertEqual(a.entity, e)
        self.assertIsNone(a.asym)
        self.assertEqual(a.seq_id, 3)

    def test_atom_asym(self):
        """Test Atom class built from an AsymUnit"""
        e = ihm.Entity('AHCDAH')
        asym = ihm.AsymUnit(e)
        a = asym.residue(3).atom('CA')
        self.assertEqual(a.id, 'CA')
        self.assertEqual(a.residue.seq_id, 3)
        self.assertIsNone(a.entity)
        self.assertEqual(a.asym, asym)
        self.assertEqual(a.seq_id, 3)

    def test_entity_range(self):
        """Test EntityRange class"""
        e = ihm.Entity('AHCDAH')
        heme = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        e._id = 42
        self.assertEqual(e.seq_id_range, (1, 6))
        r = e(3, 4)
        self.assertEqual(r.seq_id_range, (3, 4))
        self.assertEqual(r._id, 42)
        # Cannot create ranges for nonpolymeric entities
        self.assertRaises(TypeError, heme.__call__, 1, 1)
        samer = e(3, 4)
        otherr = e(2, 4)
        self.assertEqual(r, samer)
        self.assertEqual(hash(r), hash(samer))
        self.assertNotEqual(r, otherr)
        self.assertNotEqual(r, e)  # entity_range != entity

    def test_asym_range(self):
        """Test AsymUnitRange class"""
        e = ihm.Entity('AHCDAH')
        heme = ihm.Entity([ihm.NonPolymerChemComp('HEM')])
        a = ihm.AsymUnit(e)
        aheme = ihm.AsymUnit(heme)
        a._id = 42
        self.assertEqual(a.seq_id_range, (1, 6))
        # seq_id is not defined for nonpolymers
        self.assertEqual(aheme.seq_id_range, (None, None))
        r = a(3, 4)
        self.assertEqual(r.seq_id_range, (3, 4))
        self.assertEqual(r._id, 42)
        self.assertEqual(r.entity, e)
        # Cannot create ranges for nonpolymeric entities
        self.assertRaises(TypeError, aheme.__call__, 1, 1)
        samer = a(3, 4)
        otherr = a(2, 4)
        self.assertEqual(r, samer)
        self.assertEqual(hash(r), hash(samer))
        self.assertNotEqual(r, otherr)
        self.assertNotEqual(r, a)        # asym_range != asym
        self.assertNotEqual(r, e(3, 4))  # asym_range != entity_range
        self.assertNotEqual(r, e)        # asym_range != entity

    def test_asym_segment(self):
        """Test AsymUnitSegment class"""
        e = ihm.Entity('AHCDAH')
        a = ihm.AsymUnit(e)
        seg = a.segment('AH--CD', 1, 4)
        self.assertEqual(seg.gapped_sequence, 'AH--CD')
        self.assertEqual(seg.seq_id_range, (1, 4))

    def test_auth_seq_id_offset(self):
        """Test auth_seq_id offset from seq_id"""
        e = ihm.Entity('AHCDAH')
        a = ihm.AsymUnit(e, auth_seq_id_map=5)
        self.assertEqual(a._get_auth_seq_id_ins_code(1), (6, None))

    def test_auth_seq_id_dict(self):
        """Test auth_seq_id dict map from seq_id"""
        e = ihm.Entity('AHCDAH')
        a = ihm.AsymUnit(e, auth_seq_id_map={1: 0, 2: (4, 'A')})
        self.assertEqual(a._get_auth_seq_id_ins_code(1), (0, None))
        self.assertEqual(a._get_auth_seq_id_ins_code(2), (4, 'A'))
        self.assertEqual(a._get_auth_seq_id_ins_code(3), (3, None))

    def test_auth_seq_id_list(self):
        """Test auth_seq_id list map from seq_id"""
        e = ihm.Entity('AHCDAH')
        a = ihm.AsymUnit(e, auth_seq_id_map=[None, 0, 4])
        self.assertEqual(a._get_auth_seq_id_ins_code(1), (0, None))
        self.assertEqual(a._get_auth_seq_id_ins_code(2), (4, None))
        self.assertEqual(a._get_auth_seq_id_ins_code(3), (3, None))

    def test_assembly(self):
        """Test Assembly class"""
        e1 = ihm.Entity('AHCD')
        e2 = ihm.Entity('AHC')
        a = ihm.Assembly([e1, e2], name='foo', description='bar')
        self.assertEqual(a.name, 'foo')
        self.assertEqual(a.description, 'bar')

    def test_remove_identical(self):
        """Test remove_identical function"""
        x = {}
        y = {}
        all_objs = ihm._remove_identical([x, x, y])
        # Order should be preserved, but only one x should be returned
        self.assertEqual(list(all_objs), [x, y])

    def test_all_model_groups(self):
        """Test _all_model_groups() method"""
        model_group1 = 'mg1'
        model_group2 = 'mg2'
        model_group3 = 'mg3'
        model_group4 = 'mg4'
        state1 = [model_group1, model_group2]
        state2 = [model_group2, model_group2]
        s = ihm.System()
        s.state_groups.append([state1, state2])

        proc = ihm.model.OrderedProcess("time steps")
        edge = ihm.model.ProcessEdge(model_group1, model_group2)
        step = ihm.model.ProcessStep([edge], "Linear reaction")
        proc.steps.append(step)
        s.ordered_processes.append(proc)

        e1 = ihm.model.Ensemble(model_group=model_group3, num_models=10,
                                post_process=None, name='cluster1',
                                clustering_method='Hierarchical',
                                clustering_feature='RMSD',
                                precision=4.2)
        ss1 = ihm.model.Subsample(name='foo', num_models=1)
        ss2 = ihm.model.Subsample(name='foo', num_models=1,
                                  model_group=model_group4)
        e1.subsamples.extend((ss1, ss2))
        s.ensembles.append(e1)
        # Ensemble without a model_group
        e2 = ihm.model.Ensemble(model_group=None, num_models=10,
                                post_process=None, name='cluster1',
                                clustering_method='Hierarchical',
                                clustering_feature='RMSD',
                                precision=4.2)
        s.ensembles.append(e2)

        mg = s._all_model_groups()
        # List may contain duplicates but only includes states
        self.assertEqual(list(mg), [model_group1, model_group2,
                                    model_group2, model_group2])

        mg = s._all_model_groups(only_in_states=False)
        # List contains all model groups
        self.assertEqual(list(mg), [model_group1, model_group2,
                                    model_group2, model_group2,
                                    model_group3, model_group4,
                                    model_group1, model_group2])

    def test_all_models(self):
        """Test _all_models() method"""
        class MockModel(object):
            pass
        model1 = MockModel()
        model2 = MockModel()
        model_group1 = [model1, model2]
        model_group2 = [model1, model1]
        s = ihm.System()
        s.state_groups.append([[model_group1, model_group2]])
        ms = s._all_models()
        models = [model for group, model in ms]
        # duplicates should be filtered within groups, but not between groups
        self.assertEqual(models, [model1, model2, model1])

    def test_all_protocols(self):
        """Test _all_protocols() method"""
        class MockObject(object):
            pass
        model1 = MockObject()
        model2 = MockObject()
        model3 = MockObject()
        model_group1 = [model1, model2, model3]
        s = ihm.System()
        s.state_groups.append([[model_group1]])
        p1 = MockObject()
        p2 = MockObject()
        s.orphan_protocols.append(p1)
        model1.protocol = None
        model2.protocol = p2
        model3.protocol = p1
        # duplicates should be filtered globally
        self.assertEqual(list(s._all_protocols()), [p1, p2])

    def test_all_representations(self):
        """Test _all_representations() method"""
        class MockObject(object):
            pass
        model1 = MockObject()
        model2 = MockObject()
        model3 = MockObject()
        model_group1 = [model1, model2, model3]
        s = ihm.System()
        s.state_groups.append([[model_group1]])
        r1 = MockObject()
        r2 = MockObject()
        s.orphan_representations.append(r1)
        model1.representation = None
        model2.representation = r2
        model3.representation = r1
        # duplicates should be filtered globally
        self.assertEqual(list(s._all_representations()), [r1, r2])

    def test_all_assemblies(self):
        """Test _all_assemblies() method"""
        class MockObject(object):
            pass
        model1 = MockObject()
        model2 = MockObject()
        model_group1 = [model1, model2]
        s = ihm.System()
        s.state_groups.append([[model_group1]])
        asmb1 = MockObject()
        asmb2 = MockObject()
        s.orphan_assemblies.append(asmb1)
        model1.assembly = None
        model1.protocol = None
        model2.assembly = asmb2
        step = MockObject()
        step.assembly = asmb1
        prot = MockObject()
        prot.steps = [step]

        analysis1 = MockObject()
        astep1 = MockObject()
        astep1.assembly = asmb2
        analysis1.steps = [astep1]
        prot.analyses = [analysis1]

        model2.protocol = prot
        rsr1 = MockObject()
        rsr1.assembly = asmb2
        rsr2 = MockObject()
        rsr2.assembly = None
        s.restraints.extend((rsr1, rsr2))
        # duplicates should be present; complete assembly is always first
        self.assertEqual(list(s._all_assemblies()),
                         [s.complete_assembly, asmb1, asmb2, asmb1,
                          asmb2, asmb2])

    def test_all_citations(self):
        """Test _all_citations() method"""
        class MockObject(object):
            pass

        c1 = ihm.Citation(title='Test paper', journal='J Mol Biol',
                          volume=45, page_range=(1, 20), year=2016,
                          authors=['Smith A', 'Jones B'],
                          doi='10.2345/S1384107697000225',
                          pmid='1234')
        c2 = ihm.Citation(title='Test paper', journal='J Mol Biol',
                          volume=45, page_range=(1, 20), year=2016,
                          authors=['Smith A', 'Jones B'],
                          doi='1.2.3.4',
                          pmid='1234')
        c3 = ihm.Citation(title='Test paper2', journal='J Mol Biol2',
                          volume=46, page_range=(1, 20), year=2017,
                          authors=['Smith A', 'Jones B'],
                          doi='5.6.7.8',
                          pmid='5678')
        rsr1 = MockObject()  # Not a 3dem restraint
        rsr2 = MockObject()  # 3dem but with no provided citation
        rsr2.fitting_method_citation_id = None
        rsr3 = MockObject()
        rsr2.fitting_method_citation_id = c1

        s1 = ihm.Software(name='test', classification='test code',
                          description='Some test program',
                          version=1, location='http://test.org')
        s2 = ihm.Software(name='test', classification='test code',
                          description='Some test program',
                          version=1, location='http://test.org',
                          citation=c3)

        s = ihm.System()
        s.restraints.extend((rsr1, rsr2, rsr3))
        s.citations.extend((c2, c2))
        s.software.extend((s1, s2))
        # duplicates should be filtered globally
        self.assertEqual(list(s._all_citations()), [c2, c3, c1])

    def test_all_software(self):
        """Test _all_software() method"""
        class MockObject(object):
            pass

        s1 = ihm.Software(name='test', classification='test code',
                          description='Some test program',
                          version=1, location='http://test.org')
        s2 = ihm.Software(name='foo', classification='test code',
                          description='Other test program',
                          location='http://test2.org')

        sm1 = MockObject()
        sm1.software = None
        sm2 = MockObject()
        sm2.software = s1

        s = ihm.System()
        s.orphan_starting_models.extend((sm1, sm2))
        s.software.extend((s2, s2))

        step1 = MockObject()
        step2 = MockObject()
        step1.software = None
        step2.software = s2
        protocol1 = MockObject()
        protocol1.steps = [step1, step2]
        analysis1 = MockObject()
        astep1 = MockObject()
        astep1.software = s2
        analysis1.steps = [astep1]
        protocol1.analyses = [analysis1]
        s.orphan_protocols.append(protocol1)

        r1 = MockObject()
        r2 = MockObject()
        r3 = MockObject()
        r2.software = None
        r3.software = s1
        s.restraints.extend((r1, r2, r3))

        # duplicates are kept
        self.assertEqual(list(s._all_software()), [s2, s2, s1, s2, s2, s1])

    def test_all_dataset_groups(self):
        """Test _all_dataset_groups() method"""
        class MockObject(object):
            pass
        dg1 = MockObject()
        dg2 = MockObject()
        s = ihm.System()
        s.orphan_dataset_groups.append(dg1)
        step1 = MockObject()
        step2 = MockObject()
        step3 = MockObject()
        step1.dataset_group = None
        step2.dataset_group = dg2
        step3.dataset_group = dg1
        protocol1 = MockObject()
        protocol1.steps = [step1, step2, step3]
        analysis1 = MockObject()
        astep1 = MockObject()
        astep1.dataset_group = dg2
        analysis1.steps = [astep1]
        protocol1.analyses = [analysis1]
        s.orphan_protocols.append(protocol1)
        # duplicates should not be filtered
        self.assertEqual(list(s._all_dataset_groups()), [dg1, dg2, dg1, dg2])

    def test_all_locations(self):
        """Test _all_locations() method"""
        class MockObject(object):
            pass

        class MockDataset(object):
            parents = []
        loc1 = MockObject()
        loc2 = MockObject()
        loc3 = MockObject()
        loc4 = MockObject()

        s = ihm.System()
        dataset1 = MockDataset()
        dataset2 = MockDataset()
        dataset2.location = None
        dataset3 = MockDataset()
        dataset3.location = loc1

        s.locations.append(loc1)
        s.orphan_datasets.extend((dataset1, dataset2, dataset3))

        ensemble = MockObject()
        ensemble.file = loc2
        density = MockObject()
        density.file = loc1
        ss1 = MockObject()
        ss1.file = None
        ss2 = MockObject()
        ss2.file = loc4
        ensemble.densities = [density]
        ensemble.subsamples = [ss1, ss2]

        ensemble2 = MockObject()
        ensemble2.file = None
        ensemble2.densities = []
        ensemble2.subsamples = []
        s.ensembles.extend((ensemble, ensemble2))

        start_model = MockObject()
        start_model.dataset = None
        start_model.script_file = loc2
        template = MockObject()
        template.dataset = None
        template.alignment_file = loc3
        start_model.templates = [template]
        s.orphan_starting_models.append(start_model)

        step1 = MockObject()
        step2 = MockObject()
        step1.dataset_group = None
        step2.dataset_group = None
        step1.script_file = None
        step2.script_file = loc2
        protocol1 = MockObject()
        protocol1.steps = [step1, step2]
        analysis1 = MockObject()
        astep1 = MockObject()
        astep1.dataset_group = None
        astep1.script_file = loc2
        analysis1.steps = [astep1]
        protocol1.analyses = [analysis1]
        s.orphan_protocols.append(protocol1)

        # duplicates should not be filtered
        self.assertEqual(list(s._all_locations()), [loc1, loc1, loc2, loc4,
                                                    loc1, loc2, loc3,
                                                    loc2, loc2])

    def test_all_datasets(self):
        """Test _all_datasets() method"""
        class MockObject(object):
            pass

        class MockDataset(object):
            parents = []

        s = ihm.System()
        d1 = MockDataset()
        d2 = MockDataset()
        d3 = MockDataset()
        d4 = MockDataset()

        s.orphan_datasets.append(d1)

        dg1 = [d2]
        s.orphan_dataset_groups.append(dg1)

        start_model1 = MockObject()
        start_model1.dataset = None

        start_model2 = MockObject()
        start_model2.dataset = d3

        template = MockObject()
        template.dataset = None
        start_model1.templates = [template]
        start_model2.templates = []
        s.orphan_starting_models.extend((start_model1, start_model2))

        rsr1 = MockObject()
        rsr1.dataset = d4
        d4.parents = [d2]
        # Handle parent being a TransformedDataset
        trand = MockObject()
        trand.transform = None
        trand.dataset = d1
        d2.parents = [trand]
        d1.parents = d3.parents = []
        s.restraints.append(rsr1)

        # duplicates should not be filtered
        self.assertEqual(list(s._all_datasets()), [d1, d1, d2, d3, d1, d2, d4])

    def test_all_starting_models(self):
        """Test _all_starting_models() method"""
        class MockObject(object):
            pass

        s = ihm.System()
        sm1 = MockObject()
        sm2 = MockObject()
        s.orphan_starting_models.append(sm1)
        rep = ihm.representation.Representation()
        seg1 = ihm.representation.Segment()
        seg1.starting_model = None
        seg2 = ihm.representation.Segment()
        seg2.starting_model = sm2
        seg3 = ihm.representation.Segment()
        seg3.starting_model = sm2
        rep.extend((seg1, seg2, seg3))
        s.orphan_representations.append(rep)
        # duplicates should be filtered
        self.assertEqual(list(s._all_starting_models()), [sm1, sm2])

    def test_all_geometric_objects(self):
        """Test _all_geometric_objects() method"""
        class MockObject(object):
            pass

        geom1 = MockObject()
        geom2 = MockObject()

        s = ihm.System()
        r1 = MockObject()
        r2 = MockObject()
        r2.geometric_object = None
        r3 = MockObject()
        r3.geometric_object = geom1

        s.orphan_geometric_objects.extend((geom1, geom2))
        s.restraints.extend((r1, r2, r3))

        # duplicates should not be filtered
        self.assertEqual(list(s._all_geometric_objects()),
                         [geom1, geom2, geom1])

    def test_all_features(self):
        """Test _all_features() method"""
        class MockObject(object):
            pass

        f1 = MockObject()
        f2 = MockObject()

        s = ihm.System()
        r1 = MockObject()
        r2 = MockObject()
        r2._all_features = (None,)
        r3 = MockObject()
        r3._all_features = (f1,)

        s.orphan_features.extend((f1, f2))
        s.restraints.extend((r1, r2, r3))

        # duplicates should not be filtered
        self.assertEqual(list(s._all_features()), [f1, f2, f1])

    def test_all_pseudo_sites(self):
        """Test _all_pseudo_sites() method"""
        class MockObject(object):
            pass

        s1 = MockObject()
        s2 = MockObject()

        r1 = MockObject()
        xl = MockObject()
        ps = MockObject()
        ps.site = s2
        xl.pseudo1 = [ps]
        xl.pseudo2 = None
        r1.cross_links = [xl]

        r2 = MockObject()
        xl = MockObject()
        xl.pseudo1 = None
        ps = MockObject()
        ps.site = s1
        xl.pseudo2 = [ps]
        r2.cross_links = [xl]

        s = ihm.System()
        s.orphan_pseudo_sites.extend((s1, s2))
        s.restraints.extend((r1, r2))

        f1 = MockObject()
        f1.site = s2
        s.orphan_features.append(f1)

        # duplicates should not be filtered
        self.assertEqual(list(s._all_pseudo_sites()), [s1, s2, s2, s1, s2])

    def test_all_chem_descriptors(self):
        """Test _all_chem_descriptors() method"""
        class MockObject(object):
            pass

        d1 = ihm.ChemDescriptor("d1")
        d2 = ihm.ChemDescriptor("d2")
        d3 = ihm.ChemDescriptor("d3")
        d4 = ihm.ChemDescriptor("d4")

        s = ihm.System()
        f = ihm.flr.FLRData()
        s.flr_data.append(f)
        r1 = MockObject()
        r2 = MockObject()
        r2.linker = d3
        s.restraints.extend((r1, r2))

        r2.feature = None
        s.orphan_chem_descriptors.extend((d1, d2, d1))

        # FLR chemical descriptors
        conj = ihm.flr.PolyProbeConjugate(
            sample_probe=None, chem_descriptor=d4,
            ambiguous_stoichiometry=False)
        f.poly_probe_conjugates.append(conj)

        # duplicates should not be filtered
        self.assertEqual(list(s._all_chem_descriptors()), [d1, d2, d1, d3, d4])

    def test_all_entity_ranges(self):
        """Test _all_entity_ranges() method"""
        class MockObject(object):
            pass

        s = ihm.System()
        e1 = ihm.Entity('AHCD', description='foo')
        a1 = ihm.AsymUnit(e1)
        s.entities.append(e1)
        s.asym_units.append(a1)
        e1rng = e1(1, 3)
        a1rng = a1(1, 2)

        sm1 = MockObject()
        sm1.asym_unit = e1rng
        s.orphan_starting_models.append(sm1)

        rep = ihm.representation.Representation()
        seg1 = ihm.representation.Segment()
        seg1.starting_model = None
        seg1.asym_unit = a1
        rep.append(seg1)
        s.orphan_representations.append(rep)

        asmb1 = ihm.Assembly([e1, a1])
        s.orphan_assemblies.append(asmb1)

        ensemble = MockObject()
        density = MockObject()
        density.asym_unit = a1rng
        ensemble.densities = [density]
        s.ensembles.append(ensemble)

        # duplicates should not be filtered
        self.assertEqual(list(s._all_entity_ranges()),
                         [e1rng, a1, e1, a1, a1rng])

    def test_update_locations_in_repositories(self):
        """Test update_locations_in_repositories() method"""
        s = ihm.System()
        loc = ihm.location.InputFileLocation(path='foo', repo='bar')
        s.locations.append(loc)
        r = ihm.location.Repository(doi='foo', root='..')
        s.update_locations_in_repositories([r])

    def test_unknown(self):
        """Test the 'unknown' special object"""
        u = ihm.unknown
        self.assertEqual(str(u), '?')
        self.assertEqual(repr(u), '?')
        # Should only be equal to itself
        self.assertEqual(u, u)
        self.assertLessEqual(u, u)
        self.assertGreaterEqual(u, u)
        self.assertNotEqual(u, '?')
        self.assertNotEqual(u, None)
        self.assertFalse(u < u)
        self.assertFalse(u > u)
        # Should act like False
        self.assertFalse(u)


if __name__ == '__main__':
    unittest.main()
