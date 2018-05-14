import utils
import os
import unittest
import sys
if sys.version_info[0] >= 3:
    from io import StringIO
else:
    from io import BytesIO as StringIO

TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
utils.set_search_paths(TOPDIR)
import ihm.reader

class Tests(unittest.TestCase):
    def test_read(self):
        """Test read() function"""
        fh = StringIO("data_model\n_struct.entry_id testid\n")
        s, = ihm.reader.read(fh)
        self.assertEqual(s.id, 'testid')

    def test_system_reader(self):
        """Test SystemReader class"""
        s = ihm.reader._SystemReader()

    def test_id_mapper(self):
        """Test IDMapper class"""
        class MockObject(object):
            def __init__(self, x, y):
                self.x, self.y = x, y

        testlist = []
        im = ihm.reader._IDMapper(testlist, MockObject, '1', y='2')
        a = im.get_by_id('ID1')
        b = im.get_by_id('ID1')
        self.assertEqual(id(a), id(b))
        self.assertEqual(a.x, '1')
        self.assertEqual(a.y, '2')
        self.assertEqual(testlist, [a])

    def test_handler(self):
        """Test Handler base class"""
        class MockObject(object):
            pass
        o = MockObject()
        o.system = 'foo'
        h = ihm.reader._Handler(o)
        self.assertEqual(h.system, 'foo')

    def test_handler_copy_if_present(self):
        """Test copy_if_present method"""
        class MockObject(object):
            pass
        o = MockObject()
        h = ihm.reader._Handler(None)
        h._copy_if_present(o, {'foo':'bar', 'bar':'baz', 't':'u'},
                           keys=['test', 'foo'],
                           mapkeys={'bar':'baro', 'x':'y'})
        self.assertEqual(o.foo, 'bar')
        self.assertEqual(o.baro, 'baz')
        self.assertFalse(hasattr(o, 't'))
        self.assertFalse(hasattr(o, 'x'))
        self.assertFalse(hasattr(o, 'bar'))

    def test_struct_handler(self):
        """Test StructHandler"""
        fh = StringIO("_struct.entry_id eid\n_struct.title 'Test title'")
        s, = ihm.reader.read(fh)
        self.assertEqual(s.id, 'eid')
        self.assertEqual(s.title, 'Test title')

    def test_multiple_systems(self):
        """Test multiple systems from data blocks"""
        fh = StringIO("""
data_id1
_struct.entry_id id1
data_id2
_struct.entry_id id2
""")
        s1, s2 = ihm.reader.read(fh)
        self.assertEqual(s1.id, 'id1')
        self.assertEqual(s2.id, 'id2')

    def test_software_handler(self):
        """Test SoftwareHandler"""
        fh = StringIO("""
loop_
_software.pdbx_ordinal
_software.name
_software.classification
_software.description
_software.version
_software.type
_software.location
1 'test software' 'test class' 'test desc' program 1.0.1 https://example.org
""")
        s, = ihm.reader.read(fh)
        software, = s.software
        self.assertEqual(software._id, '1')
        self.assertEqual(software.name, 'test software')
        self.assertEqual(software.classification, 'test class')


    def test_citation_handler(self):
        """Test CitationHandler and CitationAuthorHandler"""
        fh = StringIO("""
loop_
_citation.id
_citation.journal_abbrev
_citation.journal_volume
_citation.page_first
_citation.page_last
_citation.year
2 'Mol Cell Proteomics' 9 2943 . 2014
3 'Mol Cell Proteomics' 9 2943 2946 2014
4 'Mol Cell Proteomics' 9 . . 2014
#
#
loop_
_citation_author.citation_id
_citation_author.name
_citation_author.ordinal
3 'Foo A' 1
3 'Bar C' 2
3 . 3
5 'Baz X' 4
""")
        s, = ihm.reader.read(fh)
        citation1, citation2, citation3, citation4 = s.citations
        self.assertEqual(citation1._id, '2')
        self.assertEqual(citation1.page_range, '2943')
        self.assertEqual(citation1.authors, [])

        self.assertEqual(citation2._id, '3')
        self.assertEqual(citation2.page_range, ('2943', '2946'))
        self.assertEqual(citation2.authors, ['Foo A', 'Bar C'])

        self.assertEqual(citation3._id, '4')
        self.assertEqual(citation3.authors, [])
        self.assertEqual(citation3.page_range, None)

        # todo: should probably be an error, no _citation.id == 4
        self.assertEqual(citation4._id, '5')
        self.assertEqual(citation4.authors, ['Baz X'])

    def test_chem_comp_handler(self):
        """Test ChemCompHandler and EntityPolySeqHandler"""
        chem_comp_cat = """
loop_
_chem_comp.id
_chem_comp.type
MET 'L-peptide linking'
CYS 'D-peptide linking'
MYTYPE 'D-PEPTIDE LINKING'
"""
        entity_poly_cat = """
loop_
_entity_poly_seq.entity_id
_entity_poly_seq.num
_entity_poly_seq.mon_id
_entity_poly_seq.hetero
1 1 MET .
1 4 MYTYPE .
1 5 CYS .
1 2 MET .
"""
        fh1 = StringIO(chem_comp_cat + entity_poly_cat)
        fh2 = StringIO(entity_poly_cat + chem_comp_cat)
        # Order of the two categories shouldn't matter
        for fh in fh1, fh2:
            s, = ihm.reader.read(fh)
            e1, = s.entities
            s = e1.sequence
            self.assertEqual(len(s), 5)
            lpeptide = ihm.LPeptideAlphabet()
            self.assertEqual(id(s[0]), id(lpeptide['M']))
            self.assertEqual(id(s[1]), id(lpeptide['M']))
            self.assertEqual(id(s[4]), id(lpeptide['C']))
            self.assertEqual(s[2], None)
            self.assertEqual(s[3].id, 'MYTYPE')
            self.assertEqual(s[3].type, 'D-peptide linking')
            self.assertEqual(s[3].__class__, ihm.DPeptideChemComp)
            # Class of standard type shouldn't be changed
            self.assertEqual(s[4].type, 'L-peptide linking')
            self.assertEqual(s[4].__class__, ihm.LPeptideChemComp)

    def test_entity_handler(self):
        """Test EntityHandler"""
        fh = StringIO("""
loop_
_entity.id
_entity.type
_entity.pdbx_description
_entity.pdbx_number_of_molecules
_entity.details
1 polymer Nup84 2 .
2 polymer Nup85 3 .
""")
        s, = ihm.reader.read(fh)
        e1, e2 = s.entities
        self.assertEqual(e1.description, 'Nup84')
        self.assertEqual(e1.number_of_molecules, '2') # todo: coerce to int

    def test_asym_unit_handler(self):
        """Test AsymUnitHandler"""
        fh = StringIO("""
loop_
_struct_asym.id
_struct_asym.entity_id
_struct_asym.details
A 1 Nup84
B 1 Nup85
""")
        s, = ihm.reader.read(fh)
        a1, a2 = s.asym_units
        self.assertEqual(a1._id, 'A')
        self.assertEqual(a1.entity._id, '1')

        self.assertEqual(a1.details, 'Nup84')
        self.assertEqual(a2.entity._id, '1')
        self.assertEqual(a2._id, 'B')
        self.assertEqual(a2.details, 'Nup85')
        self.assertEqual(id(a1.entity), id(a2.entity))

    def test_assembly_details_handler(self):
        """Test AssemblyDetailsHandler"""
        fh = StringIO("""
loop_
_ihm_struct_assembly_details.assembly_id
_ihm_struct_assembly_details.assembly_name
_ihm_struct_assembly_details.assembly_description
1 'Complete assembly' 'All known components'
""")
        s, = ihm.reader.read(fh)
        a1, = s.orphan_assemblies
        self.assertEqual(a1._id, '1')
        self.assertEqual(a1.name, 'Complete assembly')
        self.assertEqual(a1.description, 'All known components')

    def test_assembly_handler(self):
        """Test AssemblyHandler"""
        fh = StringIO("""
loop_
_ihm_struct_assembly.ordinal_id
_ihm_struct_assembly.assembly_id
_ihm_struct_assembly.parent_assembly_id
_ihm_struct_assembly.entity_description
_ihm_struct_assembly.entity_id
_ihm_struct_assembly.asym_id
_ihm_struct_assembly.seq_id_begin
_ihm_struct_assembly.seq_id_end
1 1 1 Nup84 1 A 1 726
2 1 1 Nup85 2 B 1 744
3 2 1 Nup86 2 . 1 50
""")
        s, = ihm.reader.read(fh)
        a1, a2 = s.orphan_assemblies
        self.assertEqual(a1._id, '1')
        self.assertEqual(a1.parent, None)
        self.assertEqual(len(a1), 2)
        # AsymUnitRange
        self.assertEqual(a1[0]._id, 'A')
        self.assertEqual(a1[0].seq_id_range, (1,726))
        self.assertEqual(a1[1]._id, 'B')
        self.assertEqual(a1[1].seq_id_range, (1,744))

        self.assertEqual(a2._id, '2')
        self.assertEqual(a2.parent, a1)
        # EntityRange
        self.assertEqual(len(a2), 1)
        self.assertEqual(a2[0]._id, '2')
        self.assertEqual(a2[0].seq_id_range, (1,50))

    def test_external_file_handler(self):
        """Test ExtRef and ExtFileHandler"""
        ext_ref_cat = """
loop_
_ihm_external_reference_info.reference_id
_ihm_external_reference_info.reference_provider
_ihm_external_reference_info.reference_type
_ihm_external_reference_info.reference
_ihm_external_reference_info.refers_to
_ihm_external_reference_info.associated_url
1 Zenodo DOI 10.5281/zenodo.1218053 Archive https://example.com/foo.zip
2 . 'Supplementary Files' . Other .
3 Zenodo DOI 10.5281/zenodo.1218058 File https://example.com/foo.dcd
"""
        ext_file_cat = """
loop_
_ihm_external_files.id
_ihm_external_files.reference_id
_ihm_external_files.file_path
_ihm_external_files.content_type
_ihm_external_files.details
1 1 scripts/test.py 'Modeling workflow or script' 'Test script'
2 2 foo/bar.txt 'Input data or restraints' 'Test text'
3 3 . 'Modeling or post-processing output' 'Ensemble structures'
4 3 . . .
"""
        # Order of the categories shouldn't matter
        fh1 = StringIO(ext_ref_cat + ext_file_cat)
        fh2 = StringIO(ext_file_cat + ext_ref_cat)
        for fh in fh1, fh2:
            s, = ihm.reader.read(fh)
            l1, l2, l3, l4 = s.locations
            self.assertEqual(l1.path, 'scripts/test.py')
            self.assertEqual(l1.details, 'Test script')
            self.assertEqual(l1.repo.doi, '10.5281/zenodo.1218053')
            self.assertEqual(l1.__class__, ihm.location.WorkflowFileLocation)

            self.assertEqual(l2.path, 'foo/bar.txt')
            self.assertEqual(l2.details, 'Test text')
            self.assertEqual(l2.repo, None)
            self.assertEqual(l2.__class__, ihm.location.InputFileLocation)

            self.assertEqual(l3.path, '.')
            self.assertEqual(l3.details, 'Ensemble structures')
            self.assertEqual(l3.repo.doi, '10.5281/zenodo.1218058')
            self.assertEqual(l3.__class__, ihm.location.OutputFileLocation)

            self.assertEqual(l4.path, '.')
            self.assertEqual(l4.details, None)
            self.assertEqual(l4.repo.doi, '10.5281/zenodo.1218058')
            # Type is unspecified
            self.assertEqual(l4.__class__, ihm.location.FileLocation)

    def test_dataset_list_handler(self):
        """Test DatasetListHandler"""
        fh = StringIO("""
loop_
_ihm_dataset_list.id
_ihm_dataset_list.data_type
_ihm_dataset_list.database_hosted
1 'Experimental model' YES
2 'COMPARATIVE MODEL' YES
3 'EM raw micrographs' YES
4 . YES
""")
        s, = ihm.reader.read(fh)
        d1, d2, d3, d4 = s.orphan_datasets
        self.assertEqual(d1.__class__, ihm.dataset.PDBDataset)
        self.assertEqual(d2.__class__, ihm.dataset.ComparativeModelDataset)
        self.assertEqual(d3.__class__, ihm.dataset.EMMicrographsDataset)
        # No specified data type - use base class
        self.assertEqual(d4.__class__, ihm.dataset.Dataset)

    def test_dataset_group_handler(self):
        """Test DatasetGroupHandler"""
        fh = StringIO("""
loop_
_ihm_dataset_group.ordinal_id
_ihm_dataset_group.group_id
_ihm_dataset_group.dataset_list_id
1 1 1
2 1 2
""")
        s, = ihm.reader.read(fh)
        d1, d2 = s.orphan_datasets
        g1, = s.orphan_dataset_groups
        self.assertEqual(len(g1), 2)
        self.assertEqual(g1[0], d1)
        self.assertEqual(g1[1], d2)
        # No type specified yet
        self.assertEqual(d1.__class__, ihm.dataset.Dataset)
        self.assertEqual(d2.__class__, ihm.dataset.Dataset)

    def test_dataset_extref_handler(self):
        """Test DatasetExtRefHandler"""
        fh = StringIO("""
loop_
_ihm_dataset_external_reference.id
_ihm_dataset_external_reference.dataset_list_id
_ihm_dataset_external_reference.file_id
1 4 11
2 6 12
""")
        s, = ihm.reader.read(fh)
        d1, d2 = s.orphan_datasets
        self.assertEqual(d1._id, '4')
        self.assertEqual(d1.location._id, '11')
        self.assertEqual(d2._id, '6')
        self.assertEqual(d2.location._id, '12')

    def test_dataset_dbref_handler(self):
        """Test DatasetDBRefHandler"""
        fh = StringIO("""
loop_
_ihm_dataset_related_db_reference.id
_ihm_dataset_related_db_reference.dataset_list_id
_ihm_dataset_related_db_reference.db_name
_ihm_dataset_related_db_reference.accession_code
_ihm_dataset_related_db_reference.version
_ihm_dataset_related_db_reference.details
1 1 PDB 3JRO . .
2 3 PDB 3F3F 30-OCT-08 'CRYSTAL STRUCTURE'
3 5 emdb EMD-123 . .
4 6 . . . .
""")
        s, = ihm.reader.read(fh)
        d1, d2, d3, d4 = s.orphan_datasets
        self.assertEqual(d1.location.db_name, 'PDB')
        self.assertEqual(d1.location.__class__, ihm.location.PDBLocation)
        self.assertEqual(d1.location.access_code, '3JRO')
        self.assertEqual(d2.location.db_name, 'PDB')
        self.assertEqual(d2.location.__class__, ihm.location.PDBLocation)
        self.assertEqual(d2.location.access_code, '3F3F')
        self.assertEqual(d2.location.version, '30-OCT-08')
        self.assertEqual(d2.location.details, 'CRYSTAL STRUCTURE')
        self.assertEqual(d3.location.db_name, 'EMDB')
        self.assertEqual(d3.location.__class__, ihm.location.EMDBLocation)
        self.assertEqual(d3.location.access_code, 'EMD-123')
        self.assertEqual(d3.location.version, None)
        self.assertEqual(d3.location.details, None)
        self.assertEqual(d4.location.db_name, None)
        self.assertEqual(d4.location.__class__, ihm.location.DatabaseLocation)
        self.assertEqual(d4.location.access_code, None)

    def test_related_datasets_handler(self):
        """Test RelatedDatasetsHandler"""
        fh = StringIO("""
loop_
_ihm_related_datasets.ordinal_id
_ihm_related_datasets.dataset_list_id_derived
_ihm_related_datasets.dataset_list_id_primary
1 4 1
""")
        s, = ihm.reader.read(fh)
        d1, d2 = s.orphan_datasets
        self.assertEqual(d1._id, '4')
        self.assertEqual(d2._id, '1')
        self.assertEqual(d1.parents, [d2])
        self.assertEqual(d2.parents, [])

    def test_model_representation_handler(self):
        """Test ModelRepresentationHandler"""
        fh = StringIO("""
loop_
_ihm_model_representation.ordinal_id
_ihm_model_representation.representation_id
_ihm_model_representation.segment_id
_ihm_model_representation.entity_id
_ihm_model_representation.entity_description
_ihm_model_representation.entity_asym_id
_ihm_model_representation.seq_id_begin
_ihm_model_representation.seq_id_end
_ihm_model_representation.model_object_primitive
_ihm_model_representation.starting_model_id
_ihm_model_representation.model_mode
_ihm_model_representation.model_granularity
_ihm_model_representation.model_object_count
1 1 1 1 Nup84 A 1 6 sphere . flexible by-feature 1
2 1 2 1 Nup84 A 7 20 sphere 1 rigid by-residue .
3 2 1 1 Nup84 A . . atomistic . flexible by-atom .
4 3 1 2 Nup85 B . . sphere . . multi-residue .
""")
        s, = ihm.reader.read(fh)
        r1, r2, r3 = s.orphan_representations
        self.assertEqual(len(r1), 2)
        s1, s2 = r1
        self.assertEqual(s1.__class__, ihm.representation.FeatureSegment)
        self.assertEqual(s1.primitive, 'sphere')
        self.assertEqual(s1.count, 1)
        self.assertEqual(s1.rigid, False)
        self.assertEqual(s1.starting_model, None)
        self.assertEqual(s1.asym_unit.seq_id_range, (1,6))

        self.assertEqual(s2.__class__, ihm.representation.ResidueSegment)
        self.assertEqual(s2.primitive, 'sphere')
        self.assertEqual(s2.count, None)
        self.assertEqual(s2.rigid, True)
        self.assertEqual(s2.starting_model._id, '1')
        self.assertEqual(s2.asym_unit.seq_id_range, (7,20))

        self.assertEqual(len(r2), 1)
        s1, = r2
        self.assertEqual(s1.__class__, ihm.representation.AtomicSegment)

        self.assertEqual(len(r3), 1)
        s1, = r3
        self.assertEqual(s1.__class__, ihm.representation.MultiResidueSegment)

    def test_starting_model_details_handler(self):
        """Test StartingModelDetailsHandler"""
        fh = StringIO("""
loop_
_ihm_starting_model_details.starting_model_id
_ihm_starting_model_details.entity_id
_ihm_starting_model_details.entity_description
_ihm_starting_model_details.asym_id
_ihm_starting_model_details.seq_id_begin
_ihm_starting_model_details.seq_id_end
_ihm_starting_model_details.starting_model_source
_ihm_starting_model_details.starting_model_auth_asym_id
_ihm_starting_model_details.starting_model_sequence_offset
_ihm_starting_model_details.dataset_list_id
1 1 Nup84 A 7 483 'comparative model' Q 8 4
2 1 Nup84 A . . 'comparative model' X . 6
""")
        s, = ihm.reader.read(fh)
        m1, m2 = s.orphan_starting_models
        self.assertEqual(m1.asym_unit._id, 'A')
        self.assertEqual(m1.asym_unit.seq_id_range, (7,483))
        self.assertEqual(m1.asym_id, 'Q')
        self.assertEqual(m1.offset, 8)
        self.assertEqual(m1.dataset._id, '4')

        self.assertEqual(m2.asym_unit._id, 'A')
        self.assertEqual(m2.asym_id, 'X')
        self.assertEqual(m2.offset, 0)
        self.assertEqual(m2.dataset._id, '6')

    def test_starting_comparative_models_handler(self):
        """Test StartingComparativeModelsHandler"""
        fh = StringIO("""
loop_
_ihm_starting_comparative_models.ordinal_id
_ihm_starting_comparative_models.starting_model_id
_ihm_starting_comparative_models.starting_model_auth_asym_id
_ihm_starting_comparative_models.starting_model_seq_id_begin
_ihm_starting_comparative_models.starting_model_seq_id_end
_ihm_starting_comparative_models.template_auth_asym_id
_ihm_starting_comparative_models.template_seq_id_begin
_ihm_starting_comparative_models.template_seq_id_end
_ihm_starting_comparative_models.template_sequence_identity
_ihm_starting_comparative_models.template_sequence_identity_denominator
_ihm_starting_comparative_models.template_dataset_list_id
_ihm_starting_comparative_models.alignment_file_id
1 1 A 7 436 C 9 438 90.000 1 3 2
2 1 A 33 424 C 33 424 100.000 1 1 .
""")
        s, = ihm.reader.read(fh)
        m1, = s.orphan_starting_models
        t1, t2 = m1.templates
        self.assertEqual(t1.dataset._id, '3')
        self.assertEqual(t1.asym_id, 'C')
        self.assertEqual(t1.seq_id_range, (7,436))
        self.assertEqual(t1.template_seq_id_range, (9,438))
        self.assertAlmostEqual(t1.sequence_identity, 90.0, places=1)
        self.assertEqual(t1.sequence_identity_denominator, 1)
        self.assertEqual(t1.alignment_file._id, '2')
        self.assertEqual(t2.alignment_file, None)

    def test_protocol_handler(self):
        """Test ProtocolHandler"""
        fh = StringIO("""
loop_
_ihm_modeling_protocol.ordinal_id
_ihm_modeling_protocol.protocol_id
_ihm_modeling_protocol.step_id
_ihm_modeling_protocol.struct_assembly_id
_ihm_modeling_protocol.dataset_group_id
_ihm_modeling_protocol.struct_assembly_description
_ihm_modeling_protocol.protocol_name
_ihm_modeling_protocol.step_name
_ihm_modeling_protocol.step_method
_ihm_modeling_protocol.num_models_begin
_ihm_modeling_protocol.num_models_end
_ihm_modeling_protocol.multi_scale_flag
_ihm_modeling_protocol.multi_state_flag
_ihm_modeling_protocol.ordered_flag
1 1 1 1 1 . Prot1 Sampling 'Monte Carlo' 0 500 YES NO NO
2 1 2 1 2 . Prot1 Sampling 'Monte Carlo' 500 5000 YES . NO
""")
        s, = ihm.reader.read(fh)
        p1, = s.orphan_protocols
        self.assertEqual(p1.name, "Prot1")
        self.assertEqual(len(p1.steps), 2)
        self.assertEqual(p1.steps[0].assembly._id, '1')
        self.assertEqual(p1.steps[0].dataset_group._id, '1')
        self.assertEqual(p1.steps[0].name, 'Sampling')
        self.assertEqual(p1.steps[0].method, 'Monte Carlo')
        self.assertEqual(p1.steps[0].num_models_begin, 0)
        self.assertEqual(p1.steps[0].num_models_end, 500)
        self.assertEqual(p1.steps[0].multi_scale, True)
        self.assertEqual(p1.steps[0].multi_state, False)
        self.assertEqual(p1.steps[0].ordered, False)
        self.assertEqual(p1.steps[1].multi_scale, True)
        self.assertEqual(p1.steps[1].multi_state, None)
        self.assertEqual(p1.steps[1].ordered, False)

    def test_post_process_handler(self):
        """Test PostProcessHandler"""
        fh = StringIO("""
loop_
_ihm_modeling_post_process.id
_ihm_modeling_post_process.protocol_id
_ihm_modeling_post_process.analysis_id
_ihm_modeling_post_process.step_id
_ihm_modeling_post_process.type
_ihm_modeling_post_process.feature
_ihm_modeling_post_process.num_models_begin
_ihm_modeling_post_process.num_models_end
_ihm_modeling_post_process.struct_assembly_id
_ihm_modeling_post_process.dataset_group_id
1  1   1   1   'filter'  'energy/score'  15000   6520 . .
2  1   1   2   'cluster' 'dRMSD'         6520    6520 . .
3  1   2   1   'filter'  'energy/score'  15000   6520 . .
4  1   2   2   'filter'  'composition'   6520    6520 . .
5  1   2   3   'cluster' 'dRMSD'         6520    6520 . .
6  2   3   1   'none' .         .    . . .
""")
        s, = ihm.reader.read(fh)
        p1, p2 = s.orphan_protocols
        self.assertEqual(len(p1.analyses), 2)
        self.assertEqual(len(p2.analyses), 1)
        a1, a2 = p1.analyses
        self.assertEqual(len(a1.steps), 2)
        self.assertEqual(a1.steps[0].__class__, ihm.analysis.FilterStep)
        self.assertEqual(a1.steps[0].feature, 'energy/score')
        self.assertEqual(a1.steps[0].num_models_begin, 15000)
        self.assertEqual(a1.steps[0].num_models_end, 6520)
        self.assertEqual(a1.steps[1].__class__, ihm.analysis.ClusterStep)
        self.assertEqual(len(a2.steps), 3)

        a1, = p2.analyses
        self.assertEqual(len(a1.steps), 1)
        self.assertEqual(a1.steps[0].__class__, ihm.analysis.EmptyStep)
        self.assertEqual(a1.steps[0].feature, 'none')
        self.assertEqual(a1.steps[0].num_models_begin, None)
        self.assertEqual(a1.steps[0].num_models_end, None)

    def test_model_list_handler(self):
        """Test ModelListHandler"""
        fh = StringIO("""
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 1 'Best scoring model' 'Cluster 1' 1 2 3
2 2 2 'Best scoring model' 'Cluster 2' 1 1 1
""")
        s, = ihm.reader.read(fh)
        sg, = s.state_groups
        state, = sg
        self.assertEqual(state.name, None) # auto-created state
        mg1, mg2 = state
        self.assertEqual(mg1.name, 'Cluster 1')
        self.assertEqual(mg1._id, '1')
        m, = mg1
        self.assertEqual(m._id, '1')
        self.assertEqual(m.name, 'Best scoring model')
        self.assertEqual(m.assembly._id, '1')
        self.assertEqual(m.protocol._id, '2')
        self.assertEqual(m.representation._id, '3')
        self.assertEqual(mg2.name, 'Cluster 2')
        self.assertEqual(mg2._id, '2')
        m, = mg2
        self.assertEqual(m._id, '2')

    def test_multi_state_handler(self):
        """Test MultiStateHandler"""
        fh = StringIO("""
loop_
_ihm_model_list.ordinal_id
_ihm_model_list.model_id
_ihm_model_list.model_group_id
_ihm_model_list.model_name
_ihm_model_list.model_group_name
_ihm_model_list.assembly_id
_ihm_model_list.protocol_id
_ihm_model_list.representation_id
1 1 1 'Best scoring model' 'Cluster 1' 1 2 3
#
#
loop_
_ihm_multi_state_modeling.ordinal_id
_ihm_multi_state_modeling.state_id
_ihm_multi_state_modeling.state_group_id
_ihm_multi_state_modeling.population_fraction
_ihm_multi_state_modeling.state_type
_ihm_multi_state_modeling.state_name
_ihm_multi_state_modeling.model_group_id
_ihm_multi_state_modeling.experiment_type
_ihm_multi_state_modeling.details
1 1 1 0.4 'complex formation' 'unbound' 1  'Fraction of bulk'  'unbound molecule 1'
2 2 1 .  'complex formation' 'unbound' 2  'Fraction of bulk'  'unbound molecule 2'
3 3 1 .  'complex formation' 'bound'   3  'Fraction of bulk'  'bound molecules 1 and 2'
4 3 1 .  'complex formation' 'bound'   4  'Fraction of bulk'  'bound molecules 1 and 2'
""")
        s, = ihm.reader.read(fh)
        sg, = s.state_groups
        s1, s2, s3, = sg
        self.assertAlmostEqual(s1.population_fraction, 0.4, places=1)
        self.assertEqual(s1.type, 'complex formation')
        self.assertEqual(s1.name, 'unbound')
        self.assertEqual(len(s1), 1)
        mg1, = s1
        self.assertEqual(mg1.name, 'Cluster 1')
        self.assertEqual(s1.experiment_type, 'Fraction of bulk')
        self.assertEqual(s1.details, 'unbound molecule 1')

        self.assertEqual(s2.population_fraction, None)
        self.assertEqual(len(s2), 1)
        self.assertEqual(len(s3), 2)

    def test_ensemble_handler(self):
        """Test EnsembleHandler"""
        fh = StringIO("""
loop_
_ihm_ensemble_info.ensemble_id
_ihm_ensemble_info.ensemble_name
_ihm_ensemble_info.post_process_id
_ihm_ensemble_info.model_group_id
_ihm_ensemble_info.ensemble_clustering_method
_ihm_ensemble_info.ensemble_clustering_feature
_ihm_ensemble_info.num_ensemble_models
_ihm_ensemble_info.num_ensemble_models_deposited
_ihm_ensemble_info.ensemble_precision_value
_ihm_ensemble_info.ensemble_file_id
1 'Cluster 1' 2 3 . dRMSD 1257 1 15.400 9
""")
        s, = ihm.reader.read(fh)
        e, = s.ensembles
        self.assertEqual(e.model_group._id, '3')
        self.assertEqual(e.num_models, 1257)
        self.assertEqual(e.post_process._id, '2')
        self.assertEqual(e.clustering_method, None)
        self.assertEqual(e.clustering_feature, 'dRMSD')
        self.assertEqual(e.name, 'Cluster 1')
        self.assertAlmostEqual(e.precision, 15.4, places=1)
        self.assertEqual(e.file._id, '9')

    def test_density_handler(self):
        """Test DensityHandler"""
        fh = StringIO("""
loop_
_ihm_localization_density_files.id
_ihm_localization_density_files.file_id
_ihm_localization_density_files.ensemble_id
_ihm_localization_density_files.entity_id
_ihm_localization_density_files.asym_id
_ihm_localization_density_files.seq_id_begin
_ihm_localization_density_files.seq_id_end
1 22 9 1 A 1 726
2 23 9 2 B . .
""")
        s, = ihm.reader.read(fh)
        e, = s.ensembles
        self.assertEqual(e._id, '9')
        d1, d2 = e.densities
        self.assertEqual(d1._id, '1')
        self.assertEqual(d1.file._id, '22')
        self.assertEqual(d1.asym_unit.__class__, ihm.AsymUnitRange)
        self.assertEqual(d1.asym_unit.seq_id_range, (1,726))
        self.assertEqual(d2._id, '2')
        self.assertEqual(d2.asym_unit.__class__, ihm.AsymUnit)


if __name__ == '__main__':
    unittest.main()
